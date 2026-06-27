#pragma once

namespace reg {
constexpr uint16_t CRC16 = 0x8005;

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
} // namespace reg
