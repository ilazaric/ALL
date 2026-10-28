#include <ivl/command_line_argument_parsing/implicit_exposed>
#include <ivl/contracts>
#include <ivl/format>
#include <ivl/linux/utility>
#include <ivl/logger>
#include <ivl/stl/string>
#include <ivl/uftrace>
#include <ivl/utility/forward>
#include <ivl/utility/pair_access>
#include <algorithm>
#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <vector>

int ivl_main(ivl::cmdline_parsing::implicit, const std::filesystem::path& p) {
  ivl::uftrace::parse_replay_file(p, implicit_flag("parse_describe"), implicit_flag("parse_verify"));
  return 0;
}
