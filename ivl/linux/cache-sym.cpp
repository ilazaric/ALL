#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

uint64_t from_hex(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  std::cerr << "bad hex: [" << c << "] " << (int)c << std::endl;
  exit(1);
}

uint64_t from_hex(std::string_view s) {
  if (s.size() > 16) {
    std::cerr << "too long hex: " << s << std::endl;
    exit(1);
  }
  uint64_t out = 0;
  for (auto c : s)
    out = out * 16 + from_hex(c);
  return out;
}

void do_not_optimize(const auto& value) { asm volatile("" : : "g"(value) : "memory"); }

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "expecting single arg, path to json\n";
    return 1;
  }

  std::filesystem::path       json_file(argv[1]);
  const auto                  j            = json::parse(std::ifstream(json_file));
  const std::filesystem::path device       = j["device"];
  const uint64_t              device_start = from_hex(j["device_start"]);
  const uint64_t              device_end   = from_hex(j["device_end"]);
  const uint64_t              purge_start  = from_hex(j["purge_start"]);
  const uint64_t              purge_end    = from_hex(j["purge_end"]);

  const auto fd = ivl::fs::OwnedFD::open(device, O_RDWR);
  auto       virt_start =
    mmap(nullptr, device_end - device_start, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE, fd.get(), 0);
  if (virt_start == nullptr) {
    perror("mmap");
    return 1;
  }

  const auto purge = [&] {
    uint8_t bla = 0;
    for (uint64_t i = purge_start; i < purge_end; ++i)
      bla += ((uint8_t*)virt_start)[i - device_start];
    do_not_optimize(bla);
  };

  return 0;
}
