#include <ivl/linux/raw_syscalls>
#include <ivl/util>
#include <cassert>
#include <iostream>
#include <meta>
#include <vector>

// IVL add_compiler_flags("-freflection -static")

// META LIB
namespace ivl {
struct test_t {
} test;
} // namespace ivl

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
      if (!annotations_of_with_type(member, ^^ivl::test_t).empty()) {
        test_functions.push_back(member);
      }
    }
  }
  return test_functions;
}

template <std::meta::info I>
struct invoke_function_reflection {
  invoke_function_reflection() {
    auto pid = ivl::linux::raw_syscalls::fork();
    assert(pid >= 0);
    if (pid == 0) {
      [:I:]();
      ivl::linux::raw_syscalls::exit_group(0);
    }
    int wstatus;
    ivl::linux::raw_syscalls::wait4(pid, &wstatus, 0, nullptr);
    if (wstatus != 0) {
      std::cout << "TEST FAILED " << display_string_of(I) << std::endl;
    } else {
      std::cout << "TEST PASSED " << display_string_of(I) << std::endl;
    }
  }
};

template <std::meta::info... Is>
struct invoke_function_reflections : invoke_function_reflection<Is>... {};

consteval std::vector<std::meta::info> wrap(std::vector<std::meta::info> v) {
  std::vector<std::meta::info> ret;
  for (auto i : v) ret.push_back(reflect_constant(i));
  return ret;
}
// ~META LIB

// USER CODE
[[= ivl::test]] void test1() { std::cout << "test1\n"; }
[[= ivl::test]] void test2() { std::cout << "test2\n"; }
[[= ivl::test]] void test3() { std::cout << "test3\n"; }

using full_test_runner = [:substitute(^^invoke_function_reflections, wrap(all_test_functions())):];

int main() {
  full_test_runner{};
}
