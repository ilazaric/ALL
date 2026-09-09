#pragma once

#include <ivl/linux/utility>
#include <cstring>

// https://docs.fileformat.com/audio/wav/
// https://en.wikipedia.org/wiki/WAV
// https://web.archive.org/web/19991115123323/https://www.borg.com/~jglatt/tech/wave.htm

namespace ivl::wav {
struct S {};

// constexpr size_t header_size = 44;

void load(const std::filesystem::path& file) {
  auto raw = linux::read_file(file);

  contract_assert(raw.size() >= header_size);
  std::string_view rem(raw.data(), header_size);

  auto consume = [&](size_t n) {
    contract_assert(n <= rem.size());
    auto ret = rem.substr(0, n);
    rem.remove_prefix(n);
    return ret;
  };

  auto check_eq = [&](std::string_view expected) {
    auto actual = consume(expected.size());
    LOG(actual, expected);
    contract_assert(actual == expected);
  };

  auto consume_as = [&](auto ret) {
    auto data = consume(sizeof(ret));
    memcpy(&ret, data.data(), sizeof(ret));
    return ret;
  };

  LOG(file, raw.size());

  check_eq("RIFF");
  auto file_size = 8 + consume_as(uint32_t{});
  LOG(file_size);
  contract_assert(file_size == raw.size());
  check_eq("WAVE");

  check_eq("fmt ");
  auto data_length = 8 + consume_as(uint32_t{});
  LOG(data_length);
  auto format_type = consume_as(uint16_t{});
  LOG(format_type);
  auto channel_count = consume_as(uint16_t{});
  LOG(channel_count);
  auto sample_rate = consume_as(uint32_t{});
  LOG(sample_rate);
  auto bytes_per_sec = consume_as(uint32_t{});
  LOG(bytes_per_sec);
  auto bytes_per_block = consume_as(uint16_t{});
  LOG(bytes_per_block);
  auto bits_per_sample = consume_as(uint16_t{});
  LOG(bits_per_sample);

  //

  {
    std::string_view rem(header.data(), raw.data() + raw.size());
    while (!rem.empty()) {
      contract_assert(rem.size() >= 8);
      auto id = rem.substr(0, 4);
      uint32_t len;
      memcpy(&len, rem.data() + 4, 4);
      len += 8;
      LOG(std::format("{:?}", id), len);
      contract_assert(rem.size() >= len);
      rem.remove_prefix(len);
    }
  }
  
  // auto chunk_header = consume(4);
  // LOG(std::format("{:?}", chunk_header));
  // auto data_size = consume_as(uint32_t{});
  // LOG(data_size);
  contract_assert(header.empty());
}

void save();
} // namespace ivl::wav
