#include <ivl/parsing/ninja>
#include <ivl/reflection/json>
#include <filesystem>
#include <print>
#include <string>
#include <ivl/command_line_argument_parsing/raw_arguments>

struct args {
  std::filesystem::path C;
  std::filesystem::path f = "build.ninja";
  std::string t;
};

int ivl_main(const args& args, ivl::cmdline_parsing::raw_arguments arg_targets) {
  if (!args.C.empty()) current_path(args.C);
  auto state = ivl::parsing::ninja::parse(args.f);
  auto targets = arg_targets.empty() ? std::vector(std::from_range, state.defaults)
                                     : std::vector<std::string>(std::from_range, arg_targets.rest);
  for (auto&& target : targets) contract_assert(state.targets.contains(target));
  if (args.t == "commands") {
    ivl::todo();
  } else if (args.t == "targets") {
    ivl::todo();
  } else {
    std::println("unknown command: {:?}", args.t);
    return 1;
  }
  return 0;
}
