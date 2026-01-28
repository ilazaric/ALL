#include <ivl/build_system/parse_codebase>
#include <ivl/fs/format_path>
#include <ivl/linux/utils>
#include <ivl/process>
#include <ivl/reflection/json>
#include <ivl/util>
#include <filesystem>
#include <fstream>
#include <set>
#include <vector>

// TODO: refactor into header, generate_build_sources has it too
std::vector<std::filesystem::path> find_sources(const std::filesystem::path& dir) {
  exists(dir) || ivl::panic("Path `{}` does not exist", dir);
  // if (is_regular_file(dir)) {
  //   dir.extension() == ".cpp" || dir.extension() == ".hpp" ||
  //     ivl::panic("File `{}` has bad extension, expected `.cpp` or `.hpp`", dir.native());
  //   return {dir};
  // }

  is_directory(dir) || ivl::panic("Path `{}` is neither a regular file nor a directory", dir);

  std::vector<std::filesystem::path> ret;
  for (auto&& entry : std::filesystem::recursive_directory_iterator(dir)) {
    if (!entry.is_regular_file()) continue;
    auto&& p = entry.path();
    if (p.extension() == ".cpp" || p.extension() == ".hpp") ret.emplace_back(p);
  }
  return ret;
}

struct target_t {
  std::filesystem::path file;
  enum class kind_t {
    REGULAR,
    TEST,
    // ALL,
  } kind;
  auto operator<=>(const target_t&) const = default;
};

struct pp_info_t {
  std::vector<std::filesystem::path> files;
  std::vector<std::filesystem::file_time_type> mtimes;
  std::vector<std::string> add_compiler_flags;
  std::vector<std::string> add_compiler_flags_tail;
  std::vector<std::string> dependencies;
  std::vector<std::string> test_dependencies;
  bool has_reg_variant;
  bool has_test_variant;

  bool is_stale() const {
    for (size_t idx = 0; idx < files.size(); ++idx) {
      auto& file = files[idx];
      auto& mtime = mtimes[idx];
      if (!exists(file)) return true;
      if (last_write_time(file) != mtime) return true;
    }
    return false;
  }
};

struct manifest_t {
  std::map<std::filesystem::path, pp_info_t> pp_targets;
  // std::map<std::filesystem::path, std::filesystem::file_time_type> built_targets;

  std::optional<const pp_info_t&> get_pp(const std::filesystem::path& file, const ivl::process_config& cxx_cfg) {
    if (auto it = pp_targets.find(file); it != pp_targets.end()) {
      if (!it->second.is_stale()) return {it->second};
      std::println("pp info for `{}` stale, purging and regenerating", file);
      pp_targets.erase(it);
    }

    auto opp_str = ivl::build_system::preprocess(file, cxx_cfg);
    if (!opp_str) return std::nullopt;
    auto&& pp_str = *opp_str;
    pp_str.empty() || pp_str.ends_with('\n') || ivl::panic("Preprocessed file `{}` doesn't end with newline", file);

    pp_info_t pp_info;
    std::string_view pp_sv(pp_str);
    std::filesystem::path current_file;
    std::set<std::filesystem::path> files;
    while (!pp_sv.empty()) {
      auto newline = pp_sv.find('\n');
      newline != std::string_view::npos || ivl::panic("ICE `{}`", file);
      auto line = pp_sv.substr(0, newline);
      pp_sv.remove_prefix(newline + 1);
      if (!line.starts_with('#')) continue;
      if (line.starts_with("# ")) {
        auto orig_line = line;
        line.remove_prefix(2);
        auto first_space = line.find(' ');
        first_space == std::string_view::npos&& ivl::panic("Missing space in linemarker\n```\n{}\n```", line);
        auto second_space = line.find(' ', first_space + 1);
        auto file_sv = line.substr(0, second_space).substr(first_space);
        file_sv.starts_with(" \"") ||
          ivl::panic("ICE\nfile=`{}`\ncurrent_file=`{:?}`\nlinemarker=`{:?}`", file, current_file, orig_line);
        file_sv.remove_prefix(2);
        file_sv.ends_with("\"") ||
          ivl::panic("ICE\nfile=`{}`\ncurrent_file=`{:?}`\nlinemarker=`{:?}`", file, current_file, orig_line);
        file_sv.remove_suffix(1);
        // Linemarkers where file ends with "//" denote working directory.
        // Notably `-g` emits it.
        if (file_sv.ends_with("//")) continue;
        // These are built-in "files", not really files, compiler gibberish.
        // `<built-in>` , `<command-line>` , `<stdin>` , ...
        if (file_sv.starts_with("<")) continue;
        current_file = file_sv;
        exists(current_file) ||
          ivl::panic("ICE\nfile=`{}`\ncurrent_file=`{:?}`\nlinemarker=`{:?}`", file, current_file, orig_line);
        if (!current_file.native().starts_with("/usr/include/") && !current_file.native().starts_with("/opt/GCC/") &&
            current_file.native().starts_with("/home/ilazaric/repos/ALL/build/"))
          files.insert(current_file);
        continue;
      }
      auto expected_prefix = std::string_view("#pragma IVL");
      // TODO: panics should probably be log to stderr and continue
      if (line.starts_with(expected_prefix)) {
        if (line.size() == expected_prefix.size() || line[expected_prefix.size()] != ' ')
          ivl::panic("Malformed pragma directive\n```\n{:?}\n```", line);
        auto ivl_pragma_payload = line.substr(expected_prefix.size() + 1);
        auto pieces = ivl::build_system::parse_pragma_arg(ivl_pragma_payload);
        pieces.empty() &&
          ivl::panic("Empty IVL pragma line in preprocessed `{}`, emitted from `{}`", file, current_file);

        auto command = pieces[0];
        pieces.erase(pieces.begin());

        if (command == "add_compiler_flags") {
          pp_info.add_compiler_flags.insert_range(pp_info.add_compiler_flags.end(), pieces);
        } else if (command == "add_compiler_flags_tail") {
          pp_info.add_compiler_flags_tail.insert_range(pp_info.add_compiler_flags_tail.end(), pieces);
        } else if (command == "add_dependencies") {
          pp_info.dependencies.insert_range(pp_info.dependencies.end(), pieces);
        } else if (command == "add_test_dependencies") {
          pp_info.test_dependencies.insert_range(pp_info.test_dependencies.end(), pieces);
        } else if (command == "has_test_variant") {
          if (file != current_file) continue;
          if (!pieces.empty())
            ivl::panic("In file `{}`, bad IVL pragma\n`has_test_variant` takes no arguments\n```\n{}\n```", file, line);
          pp_info.has_test_variant = true;
        } else if (command == "test_only") {
          if (file != current_file) continue;
          if (!pieces.empty())
            ivl::panic("In file `{}`, bad IVL pragma\n`test_only` takes no arguments\n```\n{}\n```", file, line);
          pp_info.has_test_variant = true;
          pp_info.has_reg_variant = false;
        } else {
          std::println(stderr, "ERROR: file `{}` has unrecognized IVL directive:", file);
          std::println(stderr, "ERROR: directive: {}", line);
          std::println(stderr, "ERROR: from file: {}", current_file);
          return std::nullopt;
        }
      }
    }

    files.contains(file) || ivl::panic("ICE `{}`", file);
    pp_info.files = std::vector(std::from_range, std::move(files));
    for (auto&& f : pp_info.files) pp_info.mtimes.push_back(last_write_time(f));

    auto& saved = (pp_targets[file] = std::move(pp_info));
    return {saved};
  }
};

// IVL add_compiler_flags("-g")
// IVL add_compiler_flags_tail("-lstdc++exp")

// int main() {
//   // manifest_t bla;
//   // pp_info_t bla;
//   std::map<std::filesystem::path, int> bla;
//   // std::vector<std::filesystem::file_time_type> bla;
//   // std::cout << ivl::to_json(bla).dump(2) << std::endl;
//   bla = ivl::from_json<decltype(bla)>({});
// }

#if 1

int main(int argc, char* argv[]) {
  auto working_dir = std::filesystem::current_path();
  auto root = ivl::util::repo_root();
  auto relative_wd = working_dir.lexically_relative(root);
  auto build_dir = root / "build";
  auto manifest_file = build_dir / "manifest.json";
  std::vector<std::filesystem::path> unresolved_targets(argv + 1, argv + argc);

  manifest_t manifest;
  {
    if (!exists(manifest_file)) goto manifest_load_end;
    is_regular_file(manifest_file) || ivl::panic("Manifest file `{}` is not a regular file", manifest_file);
    // not catching anything bc a malformed manifest is a bug, would prefer to first look at it
    manifest = ivl::from_json<manifest_t>(nlohmann::json::parse(ivl::linux::read_file(manifest_file)));
  manifest_load_end:;
  }

  ivl::util::scope_exit _{[&] {
    std::ofstream of(manifest_file);
    of << ivl::to_json(manifest).dump(2) << std::endl;
  }};

  {
    ivl::process_config gen;
    gen.pathname = root / "ivl/build_system/generate_build_sources";
    gen.argv = {gen.pathname};
    assert(gen.clone_and_exec().unwrap_or_terminate().wait().unwrap_or_terminate() == 0);
  }

  std::map<std::filesystem::path, std::vector<std::filesystem::path>> nested_targets;
  for (auto&& file : find_sources(build_dir / "source_copy")) {
    auto target = "/" / file.lexically_relative(build_dir / "source_copy");
    target.replace_extension();
    const auto& self_target = nested_targets[target];
    while (true) {
      nested_targets[target].push_back(file);
      if (target == "/") break;
      target = target.parent_path();
    }
    // self_target.size() == 1 || ivl::panic("Multiple files with same stem: {}", self_target);
  }

  ivl::process_config cxx_cfg;
  cxx_cfg.pathname = "/opt/GCC/bin/g++";
  cxx_cfg.argv = {
    cxx_cfg.pathname,
    "-xc++",
    "-Wl,-rpath=/opt/GCC/lib64",
    "-DIVL_LOCAL",
    "-O3",
    "-std=c++26",
    "-freflection",
    std::format("-ffile-prefix-map={}/=", root),
    std::format("@{}/include_dirs/args.rsp", build_dir),
  };
  cxx_cfg.envp = {
    {"LC_ALL", "C"},
    {"PATH", cxx_cfg.pathname.parent_path()},
  };

  std::vector<target_t> targets;
  for (auto&& orig : unresolved_targets) {
    auto ut = orig;
    if (ut.is_relative()) ut = "/" / relative_wd / ut;
    target_t::kind_t kind = target_t::kind_t::REGULAR;
    auto selector = [&](std::string_view sv) {
      if (!ut.native().ends_with(sv)) return false;
      std::string_view v(ut.native());
      v.remove_suffix(sv.size());
      ut = v;
      return true;
    };

    if (selector(":test")) kind = target_t::kind_t::TEST;
    // else if (selector(":regular")) kind = target_t::kind_t::REGULAR;
    ut.filename().native().contains(':') && ivl::panic("Broken selector on target `{}`", orig);
    nested_targets.contains(ut) || ivl::panic("No files correspond to `{}`", orig);
    for (auto&& file : nested_targets[ut]) {
      auto opp = manifest.get_pp(file, cxx_cfg);
      if (!opp) continue;
      auto&& pp = *opp;
      continue;
    }
  }

  // {
  //   std::set<std::filesystem::path> bla;
  //   for (auto&& target : targets) bla.insert(target.file);
  //   for (auto&& target : bla) {
  //     auto it = manifest.pp_targets.find(target);
  //     if (it != manifest.pp_targets.end()) {
  //       if (!it->second.is_stale()) continue;
  //     }
  //     auto opp = ivl::build_system::preprocess(target, cxx_cfg);
  //     if (!opp) continue;
  //   }
  // }
}

#endif
