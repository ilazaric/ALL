#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/uftrace>
#include <filesystem>

int ivl_main(ivl::cmdline_parsing::implicit, const std::filesystem::path& p) {
  ivl::uftrace::parse_replay_file(p, implicit_flag("parse_describe"), implicit_flag("parse_verify"));
  return 0;
}
