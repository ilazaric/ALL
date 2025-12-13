#pragma once

#include <ivl/fs/fileview>
#include <limits>
#include <unicode/utf8.h>
#include <vector>

namespace ivl::cppp {

  bool unicode_is_surrogate(UChar32 codepoint) {
    if (codepoint < 0xD800) return false;
    if (codepoint > 0xDFFF) return false;
    return true;
  }

  bool unicode_is_scalar_value(UChar32 codepoint) {
    if (codepoint < 0) return false;
    if (codepoint > 0x10FFFF) return false;
    return !unicode_is_surrogate(codepoint);
  }

  bool is_translation_character(UChar32 codepoint) { return unicode_is_scalar_value(codepoint); }

  std::vector<UChar32> phase1(ivl::str::NullStringView path) {
    ivl::fs::FileView fv{path};
    auto              u8 = fv.as_span<uint8_t>();
    assert(u8.size() <= std::numeric_limits<int32_t>::max());

    const uint8_t*       s      = u8.data();
    int32_t              i      = 0;
    int32_t              length = u8.size();
    UChar32              codepoint;
    std::vector<UChar32> ret;

    while (i < length) {
      U8_NEXT(s, i, length, codepoint);
      if (codepoint < 0) throw std::runtime_error("bad utf8 file");
      if (!is_translation_character(codepoint)) throw std::runtime_error("not translation character");
      ret.push_back(codepoint);
    }

    return ret;
  }

} // namespace ivl::cppp
