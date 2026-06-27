#pragma once

namespace ivl {
template<typename T>
constexpr std::array<T, 256> crc_table_no_leading(T poly) {
  auto step = [&](T value) {
    bool top_bit = value >> (sizeof(T) * 8 - 1);
    value <<= 1;
    if (top_bit) value ^= poly;
    return value;
  };

  std::array<T, 256> ret;
  ret[0] = 0;

  for (uint8_t i = 1; i; ++i) {
    T tmp = i;
    tmp <<= (sizeof(T) - 1) * 8;
    for (uint8_t j = 0; j < 8; ++j) tmp = step(tmp);
    ret[i] = tmp;
  }

  return ret;
}

constexpr uint16_t CRC16 = 0x8005;

  constexpr auto table = crc_table_no_leading<uint16_t>(CRC16);

constexpr uint16_t gen_crc16(const uint8_t* data, uint16_t size) {
  uint16_t out = 0;

  auto step = [&](bool bit) {
    auto bit_flag = out >> 15;
    out <<= 1;
    out |= bit;
    if (bit_flag) out ^= CRC16;
  };

  for (uint16_t i = 0; i < size; ++i)
    for (uint8_t j = 0; j < 8; ++j) step(data[i] >> j & 1);

  for (int i = 0; i < 16; ++i) step(false);

  return out;
}
} // namespace ivl
