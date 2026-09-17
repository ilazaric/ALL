#warning "TODO"

#include <ivl/build_system/task_executor>
#include <ivl/linux/file_descriptor>
#include <ivl/linux/terminate_syscalls>
#include <ivl/linux/utility>
#include <ivl/reflection/test_attribute>
#include <sys/mman.h>

// IVL test_only()
#pragma IVL test_only

namespace ivl::build_system {
task_config bash(std::string id, std::string arg) {}

[[= ivl::test]] void test_1() {
  namespace sys = linux::terminate_syscalls;
  linux::owned_file_descriptor in(sys::memfd_create("input", MFD_CLOEXEC));
  linux::owned_file_descriptor out(sys::memfd_create("output", MFD_CLOEXEC));
  pid_t pid = sys::getpid();
  linux::write_file_slow(ivl::fmt::format("/proc/{}/fd/{}", pid, in.get()), "hello ");
}
} // namespace ivl::build_system
