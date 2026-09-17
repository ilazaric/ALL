#include <ivl/crypto/blake3>
#include <ivl/linux/raw_syscalls>
#include <cassert>
#include <iostream>
#include <ivl/format>

int main() {
  std::string contents;
  {
    char buf[1 << 16];
    while (true) {
      auto ret = ivl::linux::raw_syscalls::read(0, buf, sizeof(buf));
      assert(ret >= 0);
      if (ret == 0) break;
      contents += std::string_view(buf, ret);
    }
  }
  auto hash = ivl::crypto::blake3::hash(contents);
  std::string_view bla((char*)&hash, (char*)(&hash + 1));
  for (auto c : bla) ivl::fmt::print("{:02x}", c);
  ivl::fmt::println("  -");
}
