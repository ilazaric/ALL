#pragma once

// https://github.com/BLAKE3-team/BLAKE3-specs/blob/master/blake3.pdf

#include <array>
#include <bit>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <vector>

namespace ivl::crypto::blake3 {

// TODO: when I add multiple TU support to build system move most of this out

inline constexpr size_t chunk_size = 1024;
inline constexpr size_t block_size = 64;
static_assert(chunk_size % block_size == 0);

struct input_chain_value : std::array<uint32_t, 8> {
  std::string_view as_view() const { return std::string_view((const char*)this, (const char*)(this + 1)); }
};
static_assert(sizeof(input_chain_value) * CHAR_BIT == 256);

struct message_block : std::array<uint32_t, 16> {
  input_chain_value truncate() const {
    input_chain_value r;
    for (int i = 0; i < r.size(); ++i) r[i] = (*this)[i];
    return r;
  }
};
static_assert(sizeof(message_block) * CHAR_BIT == 512);
static_assert(sizeof(message_block) == block_size);

enum domain_separation : uint32_t {
  CHUNK_START = 1u << 0,
  CHUNK_END = 1u << 1,
  PARENT = 1u << 2,
  ROOT = 1u << 3,
  KEYED_HASH = 1u << 4,
  DERIVE_KEY_CONTEXT = 1u << 5,
  DERIVE_KEY_MATERIAL = 1u << 6,
};

inline constexpr input_chain_value IV{
  0x6a09e667, //
  0xbb67ae85, //
  0x3c6ef372, //
  0xa54ff53a, //
  0x510e527f, //
  0x9b05688c, //
  0x1f83d9ab, //
  0x5be0cd19, //
};

inline constexpr std::array<uint32_t, 16> permutation{
  2, 6,  3,  10, //
  7, 0,  4,  13, //
  1, 11, 12, 5,  //
  9, 14, 15, 8,  //
};

message_block compress_block(const input_chain_value& h, message_block m, uint64_t t, uint32_t b, domain_separation d) {
  message_block v;
  for (int i = 0; i < 8; ++i) v[i] = h[i];
  for (int i = 0; i < 4; ++i) v[i + 8] = IV[i];
  v[12] = t;
  v[13] = (t >> 32);
  v[14] = b;
  v[15] = d;

  auto G = [&](uint32_t i, uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    a = a + b + m[2 * i];
    d = std::rotr(d ^ a, 16);
    c = c + d;
    b = std::rotr(b ^ c, 12);
    a = a + b + m[2 * i + 1];
    d = std::rotr(d ^ a, 8);
    c = c + d;
    b = std::rotr(b ^ c, 7);
  };

  auto round = [&] {
    G(0, v[0], v[4], v[8], v[12]);
    G(1, v[1], v[5], v[9], v[13]);
    G(2, v[2], v[6], v[10], v[14]);
    G(3, v[3], v[7], v[11], v[15]);
    G(4, v[0], v[5], v[10], v[15]);
    G(5, v[1], v[6], v[11], v[12]);
    G(6, v[2], v[7], v[8], v[13]);
    G(7, v[3], v[4], v[9], v[14]);
  };

  auto permute = [&] {
    message_block m2;
    // for (int i = 0; i < 16; ++i) m2[permutation[i]] = m[i];
    for (int i = 0; i < 16; ++i) m2[i] = m[permutation[i]];
    m = m2;
  };

  round();
  for (int i = 0; i < 6; ++i) {
    permute();
    round();
  }

  message_block hp;
  for (int i = 0; i < 8; ++i) hp[i] = v[i] ^ v[i + 8];
  for (int i = 0; i < 8; ++i) hp[i + 8] = v[i + 8] ^ h[i];
  return hp;
}

input_chain_value compress_chain(std::string_view data, input_chain_value k, uint64_t t, bool is_root) {
  auto compress = [&](std::string_view data, domain_separation d) {
    message_block m{};
    memcpy(m.data(), data.data(), data.size());
    k = compress_block(k, m, t, data.size(), d).truncate();
  };

  auto end = domain_separation(CHUNK_END | (is_root ? ROOT : 0u));

  if (data.size() <= block_size) [[unlikely]] {
    compress(data, domain_separation(CHUNK_START | end));
    return k;
  }

  compress(data.substr(0, block_size), CHUNK_START);
  data.remove_prefix(block_size);

  while (data.size() > block_size) {
    compress(data.substr(0, block_size), domain_separation(0));
    data.remove_prefix(block_size);
  }

  compress(data, end);
  return k;
}

input_chain_value compress_parent(
  const input_chain_value& k, const input_chain_value& left, const input_chain_value& right, bool is_root
) {
  auto d = domain_separation(PARENT | (is_root ? ROOT : 0));
  message_block m;
  for (int i = 0; i < 8; ++i) m[i] = left[i];
  for (int i = 0; i < 8; ++i) m[i + 8] = right[i];
  return compress_block(k, m, 0, block_size, d).truncate();
}

input_chain_value hash(std::string_view data) {
  if (data.size() <= chunk_size) [[unlikely]]
    return compress_chain(data, IV, 0, true);

  size_t chunk_count = (data.size() + chunk_size - 1) / chunk_size;
  std::vector<input_chain_value> chunks_compressed(chunk_count);
  for (size_t i = 0; i < chunk_count - 1; ++i) {
    chunks_compressed[i] = compress_chain(data.substr(0, chunk_size), IV, i, false);
    data.remove_prefix(chunk_size);
  }
  chunks_compressed[chunk_count - 1] = compress_chain(data, IV, chunk_count - 1, false);

  auto max_delta = std::bit_floor(chunk_count - 1);
  for (size_t delta = 1; delta < max_delta; delta *= 2)
    for (size_t /*right child*/ idx = delta; idx < chunk_count; idx += 2 * delta)
      chunks_compressed[idx ^ delta] =
        compress_parent(IV, chunks_compressed[idx ^ delta], chunks_compressed[idx], false);

  return compress_parent(IV, chunks_compressed[0], chunks_compressed[max_delta], true);
}

// TODO: keyed_hash(key, input)
// TODO: derive_key(context, key_material)

} // namespace ivl::crypto::blake3
