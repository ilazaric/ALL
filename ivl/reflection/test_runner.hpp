#include <ivl/linux/raw_syscalls>
#include <cassert>
#include <cstring>
#include <meta>
#include <print>
#include <vector>

// This file is auto-included into tests, at end.

consteval std::vector<std::meta::info> all_test_functions() {
  std::vector<std::meta::info> namespaces{^^::};
  std::vector<std::meta::info> test_functions;
  for (size_t ns_idx = 0; ns_idx < namespaces.size(); ++ns_idx) {
    auto ns = namespaces[ns_idx];
    for (auto member : members_of(ns, std::meta::access_context::unchecked())) {
      if (is_namespace(member)) {
        namespaces.push_back(member);
        continue;
      }
      if (!is_function(member)) continue;
      if (strcmp(IVL_FILE, source_location_of(member).file_name()) != 0) continue;
      if (!annotations_of_with_type(member, ^^ivl::test_t).empty()) {
        test_functions.push_back(member);
      }
    }
  }
  return test_functions;
}

consteval std::vector<std::meta::info> wrap(std::vector<std::meta::info> v) {
  std::vector<std::meta::info> ret;
  for (auto i : v) ret.push_back(reflect_constant(i));
  return ret;
}

// `true` means that the test passed.
template <std::meta::info I>
bool invoke_function() {
  std::println("RUNNING TEST {}", display_string_of(I));
  auto pid = ivl::linux::raw_syscalls::fork();
  assert(pid >= 0);
  if (pid == 0) {
    [:I:]();
    ivl::linux::raw_syscalls::exit_group(0);
  }
  int wstatus;
  ivl::linux::raw_syscalls::wait4(pid, &wstatus, 0, nullptr);
  if (wstatus != 0) {
    std::println("TEST FAILED {}", display_string_of(I));
    std::println(" - with exit status {}", wstatus);
    return false;
  } else {
    std::println("TEST PASSED {}", display_string_of(I));
    return true;
  }
}

template <std::meta::info... Is>
bool invoke_functions() {
  bool ret = true;
  ((ret &= invoke_function<Is>()), ...);
  return ret;
}

// TODO: re-exec under run_test
// ....: or should every test be re-exec'd as safe?
// ....: this re-exec could be not forkish, not vforkish
// TODO: cmdline args to execute individual tests, mayhaps verbose
int main(int argc, char* argv[]) { return ![:substitute(^^invoke_functions, wrap(all_test_functions())):](); }
