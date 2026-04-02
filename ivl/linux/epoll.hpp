#pragma once

#include <ivl/linux/file_descriptor>
#include <ivl/linux/throwing_syscalls>
#include <ivl/utility/enum>
#include <sys/epoll.h>
#include <chrono>
#include <limits>

/* relevant syscalls:
 * * epoll_create1
 * * epoll_create
 * * epoll_ctl
 * * epoll_wait
 * * epoll_pwait
 * * epoll_pwait2 */

// https://darkcoding.net/software/linux-what-can-you-epoll/
// https://www.corsix.org/content/what-is-a-pidfd

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=124765
#define pre(...)

namespace ivl::linux {
using epoll_create_enum = checked_flag_enum<decltype(EPOLL_CLOEXEC)>;
using epoll_events_enum = checked_flag_enum<EPOLL_EVENTS>;

enum class epoll_ctl_raw_enum {
  ADD = EPOLL_CTL_ADD,
  DEL = EPOLL_CTL_DEL,
  MOD = EPOLL_CTL_MOD,
};

using epoll_ctl_enum = checked_enum<epoll_ctl_raw_enum>;

struct epoll_file_descriptor : owned_file_descriptor {
  // TODO: assumed throwing semantics is bad!
  // ....: but some semantics are incompatible with a constructor
  // ....: (result-ish ones)
  // ....: so i just cant use constructors?
  // ....: well, cant use them with some semantics, not all
  // ....: a static member function create() wouldnt have this issue
  explicit epoll_file_descriptor(epoll_create_enum flags = epoll_create_enum::EPOLL_CLOEXEC)
      : owned_file_descriptor(throwing_syscalls::epoll_create1(flags.get())) {}

  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) ctl(epoll_ctl_enum op, file_descriptor fd, struct epoll_event* event) pre(!empty() && !fd.empty()) {
    return [:semantic:] ::epoll_ctl(get(), std::to_underlying(op.get()), fd.get(), event);
  }

  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) ctl_add(file_descriptor fd, struct epoll_event& event) pre(!empty() && !fd.empty()) {
    return ctl<semantic>(epoll_ctl_enum::ADD, fd, &event);
  }

  // embed the listened fd into the epoll_event.data
  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) ctl_add_fd(file_descriptor fd, epoll_events_enum events) pre(!empty() && !fd.empty()) {
    struct epoll_event event;
    event.events = events.get();
    event.data.fd = fd.get();
    return ctl_add<semantic>(fd, event);
  }

  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) ctl_mod(file_descriptor fd, struct epoll_event& event) pre(!empty() && !fd.empty()) {
    return ctl<semantic>(epoll_ctl_enum::MOD, fd, &event);
  }

  // embed the listened fd into the epoll_event.data
  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) ctl_mod_fd(file_descriptor fd, epoll_events_enum events) pre(!empty() && !fd.empty()) {
    struct epoll_event event;
    event.events = events.get();
    event.data.fd = fd.get();
    return ctl_mod<semantic>(fd, event);
  }

  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) ctl_del(file_descriptor fd) pre(!empty() && !fd.empty()) {
    return ctl<semantic>(epoll_ctl_enum::DEL, fd, nullptr);
  }

  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) wait_noblock(std::span<struct epoll_event> events) pre(!empty() && !events.empty()) {
    int maxevents = std::cmp_greater_equal(events.size(), std::numeric_limits<int>::max())
                    ? static_cast<int>(events.size())
                    : std::numeric_limits<int>::max();
    return [:semantic:] ::epoll_wait(get(), events.data(), maxevents, 0);
  }

  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) wait_block_forever(std::span<struct epoll_event> events) pre(!empty() && !events.empty()) {
    int maxevents = std::cmp_greater_equal(events.size(), std::numeric_limits<int>::max())
                    ? static_cast<int>(events.size())
                    : std::numeric_limits<int>::max();
    return [:semantic:] ::epoll_wait(get(), events.data(), maxevents, -1);
  }

  // this can do wait_nonblocking(), via timeout == 0, don't think it's worth blocking it in pre()
  template<std::meta::info semantic /* = ^^::ivl::linux::throwing_syscalls*/>
  decltype(auto) wait_block_for(std::span<struct epoll_event> events, std::chrono::nanoseconds timeout) pre(
    !empty() && !events.empty() && timeout >= std::chrono::nanoseconds(0)
  ) {
    __kernel_timespec ts;
    ts.tv_sec = timeout.count() / 1'000'000'000;
    ts.tv_nsec = timeout.count() % 1'000'000'000;
    int maxevents = std::cmp_greater_equal(events.size(), std::numeric_limits<int>::max())
                    ? static_cast<int>(events.size())
                    : std::numeric_limits<int>::max();
    return [:semantic:] ::epoll_pwait2(get(), events.data(), maxevents, &timeout, nullptr, sizeof(sigset_t));
  }
};
} // namespace ivl::linux
