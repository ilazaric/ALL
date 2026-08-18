#pragma once

#include <cstdint>

uint32_t new_hash(const char* s) {
  uint32_t h = 5381;
  for (unsigned char c = *s; c != '\0'; c = *++s) h = h * 33 + c;
  return h;
}

uint32_t elf_hash(const char* name_arg) {
  unsigned long int hash = 0;
  for (unsigned char c = *name_arg; c != '\0'; c = *(++name_arg)) {
    unsigned long int hi;
    hash = (hash << 4) + c;
    hi = hash & 0xf0000000;
    hash ^= hi >> 24;
    hash &= 0x0fffffff;
  }
  return hash;
}
