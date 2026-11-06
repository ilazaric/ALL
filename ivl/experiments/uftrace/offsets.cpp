#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/exception/print_context>
#include <ivl/logger>
#include <ivl/process>
#include <ivl/string/split>
#include <ivl/string/trim>
#include <ivl/utility/hex>
#include <ivl/utility/pair_access>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <vector>

struct offset_reference {
  std::string from;
  std::string to;
  int64_t offset;
};

int ivl_main(ivl::cmdline_parsing::implicit, const std::filesystem::path& obj) {
  ivl::process_function objdump{
    .pathname = "/usr/bin/objdump",
    .argv = {"-j", ".text", "-d"},
  };
  ivl::fmt::println("disassembling ...");
  auto disasm = objdump(obj);
  LOG(disasm.size());

  std::string current;
  std::vector<std::string> known_names;
  std::vector<offset_reference> references;
  for (auto line : ivl::string::split(disasm, "\n")) {
    auto line_orig = line;
    EXCEPTION_PRINT_CONTEXT("line:{}", line_orig);
    line = ivl::string::trim(line);
    if (line.empty()) continue;
    if (line.ends_with(">:")) {
      line.pop_back();
      line.pop_back();
      auto start = line.find('<');
      contract_assert(start != std::string::npos);
      current = line.substr(start + 1);
      if (!implicit_flag("suppress_log_current")) LOG(current);
      known_names.push_back(current);
      continue;
    }

    auto start = line.find('<');
    auto end = line.find('>');
    contract_assert((start == std::string::npos) == (end == std::string::npos));
    if (start == std::string::npos) continue;
    contract_assert(start < end);
    contract_assert(line.find('<', start + 1) == std::string::npos);
    contract_assert(line.find('>', end + 1) == std::string::npos);

    auto ref = line.substr(start + 1, end - start - 1);
    bool neg = ref.contains('-');
    if (neg) ref[ref.find('-')] = '+';
    contract_assert(ref.find('-') == std::string::npos);
    int64_t offset = 0;
    if (auto pl = ref.find('+'); pl != std::string::npos) {
      contract_assert(ref.find('+', pl + 1) == std::string::npos);
      auto off = ref.substr(pl + 1);
      ref = ref.substr(0, pl);
      contract_assert(off.starts_with("0x"));
      off = off.substr(2);
      offset = (int64_t)ivl::util::unhex_num(off);
      contract_assert(offset >= 0);
    }
    references.emplace_back(current, ref, neg ? -offset : offset);
  }
  ivl::fmt::println("parsing disassembly done");
  LOG(known_names.size());
  LOG(references.size());
  ivl::fmt::println();

  std::map<std::string, size_t> min_offsets;
  for (auto&& [from, to, offset] : references) {
    if (offset <= 0) continue;
    if (implicit_flag("only_self_references") && from != to) continue;
    if (!min_offsets.contains(to)) min_offsets[to] = (size_t)-1;
    min_offsets[to] = std::min(min_offsets[to], (size_t)offset);
  }

  std::vector<std::pair<std::string, size_t>> sorted(std::from_range, min_offsets);
  std::ranges::sort(sorted, {}, ivl::second);
  std::set known(std::from_range, known_names);
  size_t printed = 0;
  for (auto&& [name, offset] : sorted) {
    if (offset > implicit_value("limit_offset", (size_t)-1)) continue;
    if (implicit_flag("suppress_unknown") && !known.contains(name)) continue;
    LOG(name, offset);
    ++printed;
  }
  LOG(printed);
  return 0;
}

/*
$ ./offsets --only_self_references --suppress_log_current --suppress_unknown --limit_offset 8
/usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus
[...]
[LOG] ivl/experiments/uftrace/offsets.cpp:ivl_main(96): printed=128
 */
