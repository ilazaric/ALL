#include <ivl/build_system/parse_codebase>
#include <ivl/linux/utils>
#include <ivl/process>
#include <ivl/reflection/json>
#include <ivl/util>
#include <filesystem>
#include <fstream>
#include <set>
#include <thread>
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

struct cxx_cfg_part_t {
  std::filesystem::path pathname;
  std::vector<std::string> argv;
  std::map<std::string, std::string> envp;
  auto operator<=>(const cxx_cfg_part_t&) const = default;
};

struct pp_info_t {
  std::filesystem::path file;

  std::vector<std::filesystem::path> files;
  std::vector<std::filesystem::file_time_type> mtimes;

  std::vector<std::string> add_compiler_flags;
  std::vector<std::string> add_compiler_flags_tail;

  // std::vector<std::string> dependencies;
  // std::vector<std::string> test_dependencies;

  bool built_regular = false;
  bool built_test = false;

  bool is_stale() const {
    for (size_t idx = 0; idx < files.size(); ++idx) {
      auto& file = files[idx];
      auto& mtime = mtimes[idx];
      if (!exists(file)) return true;
      if (last_write_time(file) != mtime) return true;
    }
    return false;
  }

  bool build_regular(const cxx_cfg_part_t& cfg, const std::filesystem::path& out) {
    if (built_regular) return true;

    create_directories(out.parent_path());

    ivl::process_config cxx_cfg;
    cxx_cfg.pathname = cfg.pathname;
    cxx_cfg.argv = cfg.argv;
    cxx_cfg.envp = cfg.envp;

    cxx_cfg.argv.insert_range(cxx_cfg.argv.end(), add_compiler_flags);
    cxx_cfg.argv.push_back(file);
    cxx_cfg.argv.push_back("-o");
    cxx_cfg.argv.push_back(out);
    cxx_cfg.argv.insert_range(cxx_cfg.argv.end(), add_compiler_flags_tail);

    auto wstatus = cxx_cfg.clone_and_exec().unwrap_or_terminate().wait().unwrap_or_terminate();
    if (wstatus != 0) {
      std::println(stderr, "ERROR: compilation of file `{}` failed with status {}", out, wstatus);
      return false;
    }

    built_regular = true;
    return true;
  }

  bool build_test(const cxx_cfg_part_t& cfg, const std::filesystem::path& out) {
    if (built_test) return true;

    create_directories(out.parent_path());

    ivl::process_config cxx_cfg;
    cxx_cfg.pathname = cfg.pathname;
    cxx_cfg.argv = cfg.argv;
    cxx_cfg.envp = cfg.envp;

    auto src = file;
    while (src.filename() != "source_copy") src = src.parent_path();

    cxx_cfg.argv.insert_range(cxx_cfg.argv.end(), add_compiler_flags);
    cxx_cfg.argv.push_back("-include");
    cxx_cfg.argv.push_back(file);
    cxx_cfg.argv.push_back(std::format("-DIVL_FILE=\"{}\"", file.lexically_relative(src)));
    cxx_cfg.argv.push_back("-include");
    cxx_cfg.argv.push_back("ivl/reflection/test_runner");
    cxx_cfg.argv.push_back("/dev/null");
    cxx_cfg.argv.push_back("-o");
    cxx_cfg.argv.push_back(out);
    cxx_cfg.argv.insert_range(cxx_cfg.argv.end(), add_compiler_flags_tail);

    auto wstatus = cxx_cfg.clone_and_exec().unwrap_or_terminate().wait().unwrap_or_terminate();
    if (wstatus != 0) {
      std::println(stderr, "ERROR: compilation of file `{}` failed with status {}", out, wstatus);
      return false;
    }

    built_test = true;
    return true;
  }
};

// struct build_info_t {
//   std::filesystem::path source_file;
//   std::filesystem::path binary_file;
//   std::filesystem::file_time_type mtime;
// };

struct manifest_part_t {
  std::map<std::filesystem::path, pp_info_t> pp_targets;
  // std::map<cxx_cfg_part_t, std::set<std::filesystem::path>> built_targets;

  std::optional<pp_info_t&>
  get_pp(const std::filesystem::path& file, const cxx_cfg_part_t& cfg, const std::filesystem::path& build_dir) {
    if (auto it = pp_targets.find(file); it != pp_targets.end()) {
      if (!it->second.is_stale()) return {it->second};
      std::println("pp info for `{}` stale, purging and regenerating", file);
      pp_targets.erase(it);
    }

    ivl::process_config cxx_cfg;
    cxx_cfg.pathname = cfg.pathname;
    cxx_cfg.argv = cfg.argv;
    cxx_cfg.envp = cfg.envp;
    auto opp_str = ivl::build_system::preprocess(file, std::move(cxx_cfg));
    if (!opp_str) return std::nullopt;
    auto&& pp_str = *opp_str;
    pp_str.empty() || pp_str.ends_with('\n') || ivl::panic("Preprocessed file `{}` doesn't end with newline", file);

    pp_info_t pp_info;
    pp_info.file = file;
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
            current_file.native().starts_with(build_dir.native()))
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
          // } else if (command == "add_dependencies") {
          //   pp_info.dependencies.insert_range(pp_info.dependencies.end(), pieces);
          // } else if (command == "add_test_dependencies") {
          //   pp_info.test_dependencies.insert_range(pp_info.test_dependencies.end(), pieces);
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

struct manifest_t {
  [[= ivl::json_serialize_as_array]] std::map<cxx_cfg_part_t, manifest_part_t> parts;
};

// IVL add_compiler_flags("-g")
// IVL add_compiler_flags_tail("-lstdc++exp")

#if 1

int main(int argc, char* argv[], char* envp[]) {
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

  cxx_cfg_part_t cxx_cfg;
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
    {"PATH", std::format("{}:{}", cxx_cfg.pathname.parent_path(), "/usr/bin")},
  };

  {
    ivl::process_config gen;
    gen.pathname = root / "build/bootstrap_dir/generate_build_sources";
    gen.argv = {gen.pathname};
    assert(gen.clone_and_exec().unwrap_or_terminate().wait().unwrap_or_terminate() == 0);
  }

  {
    auto artifacts_dir =
      build_dir / "artifacts" / ivl::util::hex(ivl::crypto::blake3::hash(ivl::to_json(cxx_cfg).dump()).as_view());
    LOG(artifacts_dir);
    create_directories(artifacts_dir);
    auto& curr_manifest = manifest.parts[cxx_cfg];
    for (std::filesystem::path target :
         {"ivl/build_system/generate_build_sources.cpp", "ivl/build_system/builder.cpp"}) {
      auto opp = curr_manifest.get_pp(build_dir / "source_copy" / target, cxx_cfg, build_dir);
      opp || ivl::panic("Missing file `{}`", build_dir / "source_copy" / target);
      auto&& pp = *opp;
      if (pp.built_regular) continue;
      auto out = artifacts_dir / target.parent_path() / target.stem();
      pp.build_regular(cxx_cfg, out) || ivl::panic("Failed to build `{}`", target);
      auto out2 = build_dir / "bootstrap_dir" / target.stem();
      if (exists(out2)) remove(out2);
      copy_file(out, out2);
      std::println("Re-execing ...");
      _.fn();
      auto exe = build_dir / "bootstrap_dir/builder";
      ivl::linux::throwing_syscalls::execve(exe.c_str(), argv, envp);
    }
  }

  auto artifacts_dir =
    build_dir / "artifacts" / ivl::util::hex(ivl::crypto::blake3::hash(ivl::to_json(cxx_cfg).dump()).as_view());
  LOG(artifacts_dir);
  create_directories(artifacts_dir);

  auto& curr_manifest = manifest.parts[cxx_cfg];

  std::map<std::filesystem::path, std::vector<std::filesystem::path>> nested_targets;
  for (auto&& file : find_sources(build_dir / "source_copy")) {
    auto target = "/" / file.lexically_relative(build_dir / "source_copy");
    if (target == "/ivl/reflection/test_runner.hpp") continue;
    target.replace_extension();
    const auto& self_target = nested_targets[target];
    while (true) {
      nested_targets[target].push_back(file);
      if (target == "/") break;
      target = target.parent_path();
    }
    self_target.size() == 1 || ivl::panic("Multiple files with same stem: {}", self_target);
  }

  std::set<target_t> targets;
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
      auto opp = curr_manifest.get_pp(file, cxx_cfg, build_dir);
      if (!opp) continue;
      auto&& pp = *opp;
      if (kind == target_t::kind_t::REGULAR && file.extension() != ".cpp") continue;
      // TODO
      if (kind == target_t::kind_t::TEST && file.extension() != ".hpp") continue;
      if (kind == target_t::kind_t::TEST &&
          std::ranges::find(pp.files, build_dir / "include_dirs/regular/ivl/reflection/test_attribute") ==
            pp.files.end())
        continue;
      targets.emplace(file, kind);
    }
  }

  std::println("Targets:");
  for (auto&& [file, kind] : targets) std::println("{}{}", file, kind == target_t::kind_t::REGULAR ? "" : ":test");

  {
    std::vector<target_t> vec_targets(std::from_range, targets);
    size_t thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) thread_count = 1;
    if (thread_count != 1) --thread_count;
    LOG(thread_count);

    std::atomic<size_t> target_tail = 0;
    std::vector<std::vector<target_t>> failures(thread_count);
    auto worker = [&](size_t idx) {
      while (true) {
        auto tail = target_tail.fetch_add(1);
        if (tail >= vec_targets.size()) break;
        auto&& target = vec_targets[tail];
        auto&& pp = *curr_manifest.get_pp(target.file, cxx_cfg, build_dir);
        if (target.kind == target_t::kind_t::REGULAR) {
          auto out = artifacts_dir / target.file.lexically_relative(build_dir / "source_copy").parent_path() /
                     target.file.stem();
          if (!pp.build_regular(cxx_cfg, out)) failures[idx].push_back(target);
        } else {
          auto out = artifacts_dir / target.file.lexically_relative(build_dir / "source_copy").parent_path() /
                     std::format("{}:test", target.file.stem());
          if (!pp.build_test(cxx_cfg, out)) failures[idx].push_back(target);
        }
      }
    };

    {
      std::vector<std::jthread> workers;
      for (size_t i = 0; i < thread_count; ++i) workers.emplace_back(worker, i);
    }

    std::vector<target_t> all_failures;
    for (auto&& f : failures) all_failures.insert_range(all_failures.end(), f);
    if (!all_failures.empty()) {
      std::ranges::sort(all_failures);
      std::println("Failed targets:");
      for (auto&& [file, kind] : all_failures)
        std::println("{}{}", file, kind == target_t::kind_t::REGULAR ? "" : ":test");
      return 1;
    }
  }

  // for (auto&& target : targets) {
  //   auto&& pp = *curr_manifest.get_pp(target.file, cxx_cfg);
  //   if (target.kind == target_t::kind_t::REGULAR) {
  //     auto out =
  //       artifacts_dir / target.file.lexically_relative(build_dir / "source_copy").parent_path() / target.file.stem();
  //     pp.build_regular(cxx_cfg, out) || ivl::panic("Failed to compile regular `{}`", target.file);
  //   } else {
  //     auto out = artifacts_dir / target.file.lexically_relative(build_dir / "source_copy").parent_path() /
  //                std::format("{}:test", target.file.stem());
  //     pp.build_test(cxx_cfg, out) || ivl::panic("Failed to compile test `{}`", target.file);
  //   }
  // }
}

#endif
