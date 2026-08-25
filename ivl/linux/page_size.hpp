#pragma once

namespace ivl::linux {
// TODO: this should be validated on startup via sysconf(_SC_PAGE_SIZE)
inline constexpr size_t page_size = 4096;
} // namespace ivl::linux
