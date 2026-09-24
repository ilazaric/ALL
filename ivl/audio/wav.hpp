#pragma once

#include <ivl/linux/utility>
#include <ivl/logger>
#include <ivl/meta>
#include <ivl/utility>
#include <algorithm>
#include <cstring>
#include <ranges>

// https://docs.fileformat.com/audio/wav/
// https://en.wikipedia.org/wiki/WAV
// https://web.archive.org/web/19991115123323/https://www.borg.com/~jglatt/tech/wave.htm

namespace ivl::wav {
struct payload {
  // fmt
  uint16_t format_type;
  uint16_t channel_count;
  uint32_t sample_rate;
  uint32_t bytes_per_sec;
  uint16_t bytes_per_block;
  uint16_t bits_per_sample;
  // data
  std::string data;

  bool operator==(const payload&) const = default;
};

payload load(const std::filesystem::path& file, bool silent = false) {
  auto raw = linux::read_file(file);
  if (!silent) LOG(file, raw.size());
  std::string_view rem = raw;

  auto consume = [&](size_t n) {
    contract_assert(n <= rem.size());
    auto ret = rem.substr(0, n);
    rem.remove_prefix(n);
    return ret;
  };

  auto check_eq = [&](std::string_view expected) {
    auto actual = consume(expected.size());
    if (!silent) LOG(actual, expected);
    contract_assert(actual == expected);
  };

  auto consume_as = [&](auto ret) {
    auto data = consume(sizeof(ret));
    memcpy(&ret, data.data(), sizeof(ret));
    return ret;
  };

  bool fmt_seen = false;
  bool data_seen = false;
  payload ret;

  check_eq("RIFF");
  auto file_size = 8 + consume_as(uint32_t{});
  if (!silent) LOG(file_size);
  contract_assert(file_size == raw.size());
  check_eq("WAVE");

  while (!rem.empty()) {
    contract_assert(rem.size() >= 8);
    auto id = rem.substr(0, 4);
    uint32_t len;
    memcpy(&len, rem.data() + 4, 4);
    if (!silent) LOG(ivl::fmt::format("{:?}", id), len);
    rem.remove_prefix(8);
    contract_assert(rem.size() >= len);
    auto curr = rem.substr(0, len);
    rem.remove_prefix(len);
    std::swap(curr, rem);
    ivl::util::scope_exit _{[&] {
      contract_assert(rem.empty());
      std::swap(curr, rem);
    }};
    contract_assert(rem.size() == len);

    if (id == "LIST") {
      consume(len);
      continue;
    }
    if (id == "fmt ") {
      contract_assert(!fmt_seen);
      fmt_seen = true;
      ret.format_type = consume_as(uint16_t{});
      if (!silent) LOG(ret.format_type);
      ret.channel_count = consume_as(uint16_t{});
      if (!silent) LOG(ret.channel_count);
      ret.sample_rate = consume_as(uint32_t{});
      if (!silent) LOG(ret.sample_rate);
      ret.bytes_per_sec = consume_as(uint32_t{});
      if (!silent) LOG(ret.bytes_per_sec);
      ret.bytes_per_block = consume_as(uint16_t{});
      if (!silent) LOG(ret.bytes_per_block);
      ret.bits_per_sample = consume_as(uint16_t{});
      if (!silent) LOG(ret.bits_per_sample);
      continue;
    }
    if (id == "data") {
      contract_assert(!data_seen);
      data_seen = true;
      ret.data = std::string(consume(len));
      continue;
    }
    panic("unknown chunk id: {:?}", id);
  }

  contract_assert(fmt_seen);
  contract_assert(data_seen);

  return ret;
}

void save(const payload& p, const std::filesystem::path& file) {
  std::string raw;

  auto write = [&](auto x) {
    std::string_view y;
    if constexpr (ivl::meta::same_as_one_of<decltype(x), const char*, std::string, std::string_view>) {
      y = std::string_view(x);
    } else {
      y = std::string_view((const char*)&x, (const char*)(&x + 1));
    }
    raw += y;
  };

  uint32_t len = 0;
  uint32_t pos = 0;

  write("RIFF");
  write(len);
  write("WAVE");

  write("fmt ");
  pos = raw.size();
  write(len);
  write(p.format_type);
  write(p.channel_count);
  write(p.sample_rate);
  write(p.bytes_per_sec);
  write(p.bytes_per_block);
  write(p.bits_per_sample);
  len = raw.size() - pos - sizeof(len);
  memcpy(raw.data() + pos, &len, sizeof(len));

  write("data");
  pos = raw.size();
  write(len);
  write(p.data);
  len = raw.size() - pos - sizeof(len);
  memcpy(raw.data() + pos, &len, sizeof(len));

  len = raw.size() - 8;
  memcpy(raw.data() + 4, &len, sizeof(len));

  LOG(file, raw.size());
  linux::write_file_slow(file, raw);

  contract_assert(load(file, true) == p);
}

std::vector<payload> channel_split(const payload& p) {
  contract_assert(p.bytes_per_sec % p.channel_count == 0);
  contract_assert(p.bytes_per_block % p.channel_count == 0);
  contract_assert(p.data.size() % p.bytes_per_block == 0);
  std::vector<payload> v(p.channel_count);
  for (size_t i = 0; i < p.channel_count; ++i) {
    auto& c = v[i];
    c.format_type = p.format_type;
    c.channel_count = 1;
    c.sample_rate = p.sample_rate;
    c.bytes_per_sec = p.bytes_per_sec / p.channel_count;
    c.bytes_per_block = p.bytes_per_block / p.channel_count;
    c.bits_per_sample = p.bits_per_sample;
  }
  for (size_t i = 0; i < p.data.size(); i += p.bytes_per_block) {
    for (size_t j = 0; j < p.channel_count; ++j) {
      auto& c = v[j];
      c.data += std::string_view(p.data).substr(i + j * c.bytes_per_block, c.bytes_per_block);
    }
  }
  return v;
}

payload channel_merge(const std::vector<payload>& v) {
  contract_assert(!v.empty());
  contract_assert(v.size() < 65536);
  uint16_t channel_count = v.size();
  payload p;
  p.format_type = v[0].format_type;
  p.channel_count = channel_count;
  p.sample_rate = v[0].sample_rate;
  p.bytes_per_sec = v[0].bytes_per_sec * channel_count;
  p.bytes_per_block = v[0].bytes_per_block * channel_count;
  p.bits_per_sample = v[0].bits_per_sample;
  for (auto&& c : v) {
    contract_assert(p.format_type == c.format_type);
    contract_assert(1 == c.channel_count);
    contract_assert(p.sample_rate == c.sample_rate);
    contract_assert(p.bytes_per_sec == c.bytes_per_sec * channel_count);
    contract_assert(p.bytes_per_block == c.bytes_per_block * channel_count);
    contract_assert(p.bits_per_sample == c.bits_per_sample);
    contract_assert(c.data.size() == v[0].data.size());
  }
  p.data.resize(v[0].data.size() * channel_count);
  for (size_t i = 0; i < p.data.size(); i += p.bytes_per_block) {
    for (size_t j = 0; j < p.channel_count; ++j) {
      auto& c = v[j];
      memcpy(p.data.data() + i + j * c.bytes_per_block, c.data.data() + i / channel_count, c.bytes_per_block);
    }
  }
  return p;
}

std::vector<double> extract(const payload& p, size_t channel) {
  contract_assert(p.channel_count > channel);
  contract_assert(p.format_type == 1);
  contract_assert(p.bytes_per_block % p.channel_count == 0);
  contract_assert(p.bytes_per_block / p.channel_count == 2);
  std::vector<double> ret;
  for (size_t i = p.bytes_per_block / p.channel_count * channel; i < p.data.size(); i += p.bytes_per_block) {
    int16_t x;
    memcpy(&x, p.data.data() + i, sizeof(x));
    double y = x;
    y /= (1ull << 15);
    ret.push_back(y);
  }
  return ret;
}

payload synthesize(const std::vector<double>& data, const payload& like) {
  payload p;
  p.format_type = like.format_type;
  p.channel_count = 1;
  p.sample_rate = like.sample_rate;
  p.bytes_per_sec = like.bytes_per_sec / like.channel_count;
  p.bytes_per_block = like.bytes_per_block / like.channel_count;
  p.bits_per_sample = like.bits_per_sample;
  contract_assert(p.format_type == 1);
  contract_assert(p.bytes_per_block == 2);
  size_t clipped_max = 0;
  size_t clipped_min = 0;
  for (auto y : data) {
    y *= (1ull << 15);
    int64_t x = y;
    if (x >= (1ll << 15)) {
      ++clipped_max;
      x = (1ll << 15) - 1;
    }
    if (x < -(1ll << 15)) {
      ++clipped_min;
      x = -(1ll << 15);
    }
    int16_t z = (int16_t)x;
    p.data += std::string_view((const char*)&z, sizeof(z));
  }
  if (clipped_max || clipped_min)
    ivl::panic(
      "synthesis clipped\n"
      "  max: {}\n"
      "  min: {}\n"
      "  max count: {}\n"
      "  min count: {}\n",
      std::ranges::max(data), std::ranges::min(data), clipped_max, clipped_min
    );
  return p;
}
} // namespace ivl::wav
