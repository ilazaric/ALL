#pragma once

#include <ivl/format>
#include <ivl/linux/raw_syscalls>
#include <ivl/reflection/test_attribute>
#include <ivl/reflection/test_matrix>
#include <ivl/reflection/utility>
#include <cassert>
#include <cstring>
#include <meta>
#include <vector>

// This file is auto-included into tests, at end.

struct test_thing {
  const char* name_start;
  size_t name_length;
  void (*call)();
  bool should_fail = false;
  std::string_view name() const { return std::string_view(name_start, name_length); }
};

template<typename T, auto Mem>
void invoke_class_function() {
  T t;
  t.[:Mem:]();
}

consteval std::string qualified_name(std::meta::info i) {
  std::string ret(identifier_of(i));
  while (true) {
    i = parent_of(i);
    if (i == ^^::) break;
    ret = identifier_of(i) + ("::" + ret);
  }
  return ret;
}

consteval std::vector<test_thing> handle_test_broadcast(ivl::test_broadcast b) {
  std::vector<test_thing> test_functions;
  auto temp_name = qualified_name(b.temp);
  for (auto args : b.matrix.data) {
    auto cl = substitute(b.temp, args);
    for (auto member : members_of(cl, std::meta::access_context::unchecked())) {
      if (!is_function(member)) continue;
      if (!annotations_of_with_type(member, ^^ivl::test_t).empty()) {
        auto name = ivl::fmt::format("{}{::?}::{}", temp_name, args, identifier_of(member));
        test_functions.emplace_back(
          std::define_static_string(name), name.size(),
          extract<void (*)()>(substitute((^^invoke_class_function), {cl, reflect_constant(member)}))
        );
      }
      if (!annotations_of_with_type(member, ^^ivl::test_fail_t).empty()) {
        auto name = ivl::fmt::format("{}{::?}::{}", temp_name, args, identifier_of(member));
        test_functions.emplace_back(
          std::define_static_string(name), name.size(),
          extract<void (*)()>(substitute((^^invoke_class_function), {cl, reflect_constant(member)})), true
        );
      }
    }
  }
  return test_functions;
}

consteval std::vector<test_thing> all_test_functions() {
  std::vector<std::meta::info> namespaces{^^::};
  std::vector<test_thing> test_functions;
  for (size_t ns_idx = 0; ns_idx < namespaces.size(); ++ns_idx) {
    auto ns = namespaces[ns_idx];
    for (auto member : members_of(ns, std::meta::access_context::unchecked())) {
      if (is_namespace(member)) {
        namespaces.push_back(member);
        continue;
      }
      if (strcmp(IVL_FILE, source_location_of(member).file_name()) != 0) continue;
      if (is_variable(member) && decay(type_of(member)) == ^^ivl::test_broadcast) {
        test_functions.insert_range(test_functions.end(), handle_test_broadcast(extract<ivl::test_broadcast>(member)));
        continue;
      }
      if (!is_function(member)) continue;
      if (!annotations_of_with_type(member, ^^ivl::test_t).empty()) {
        auto name = display_string_of(member);
        test_functions.emplace_back(std::define_static_string(name), name.size(), extract<void (*)()>(member));
        continue;
      }
      if (!annotations_of_with_type(member, ^^ivl::test_fail_t).empty()) {
        auto name = display_string_of(member);
        test_functions.emplace_back(std::define_static_string(name), name.size(), extract<void (*)()>(member), true);
        continue;
      }
    }
  }
  return test_functions;
}

// `true` means that the test passed.
bool invoke_function(test_thing t) noexcept {
  ivl::fmt::println("... RUNNING TEST {}", t.name());
  fflush(stdout);
  auto pid = ivl::linux::raw_syscalls::fork();
  assert(pid >= 0);
  if (pid == 0) {
    try {
      t.call();
    } catch (const std::exception& e) {
      ivl::fmt::println("!!! EXCEPTION:\n{}", e.what());
      fflush(stdout);
      ivl::linux::raw_syscalls::exit_group(1);
    }
    fflush(stdout);
    ivl::linux::raw_syscalls::exit_group(0);
  }
  int wstatus;
  ivl::linux::raw_syscalls::wait4(pid, &wstatus, 0, nullptr);
  if ((wstatus == 0) == t.should_fail) {
    ivl::fmt::println("!!! TEST FAILED  {}", t.name());
    ivl::fmt::println("    - with exit status {}", wstatus);
    return false;
  } else {
    ivl::fmt::println("... TEST PASSED  {}", t.name());
    return true;
  }
}

// TODO: re-exec under run_test
// ....: or should every test be re-exec'd as safe?
// ....: this re-exec could be not forkish, not vforkish
// TODO: cmdline args to execute individual tests, mayhaps verbose
int main(int argc, char* argv[]) {
  bool ret = true;
  constexpr auto foo = define_static_array(all_test_functions());
  for (auto t : foo) ret &= invoke_function(t);
  return !ret;
}
