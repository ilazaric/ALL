#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/logger>
#include "compress"
#include "default"
#include <filesystem>

int ivl_main(ivl::cmdline_parsing::implicit, const std::filesystem::path& p) {
  auto root = ivl::uftrace::parse_replay_file(p, implicit_flag("parse_describe"), implicit_flag("parse_verify"));
  ivl::uftrace::compressor c;
  auto cr = c.compress(root);
  LOG(c.nodes.size());
  return 0;
}
