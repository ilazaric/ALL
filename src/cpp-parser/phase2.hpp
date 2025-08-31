#pragma once

#include <unicode/uchar.h>
#include <unicode/utf8.h>
#include <vector>

namespace ivl::cppp {

  constexpr UChar32 UNICODE_NEWLINE   = '\n';
  constexpr UChar32 UNICODE_BACKSLASH = '\\';

  std::vector<UChar32> phase2(std::span<const UChar32> code) {
    bool source_is_empty = code.empty();
    if (!code.empty() && code[0] == 0xFEFF) code = code.subspan(1);

    std::vector<UChar32> ret;
    bool                 backslash = false;
    std::vector<UChar32> whitespaces;

    auto parse_pt = [&](UChar32 pt) {
      if (pt == UNICODE_NEWLINE) {
        if (!backslash) {
          ret.push_back(pt);
          return;
        }
        // \ ...ws... \n
        backslash = false;
        whitespaces.clear();
        return;
      }

      if (u_isWhitespace(pt)) {
        (backslash ? whitespaces : ret).push_back(pt);
        return;
      }

      if (backslash) {
        ret.push_back(UNICODE_BACKSLASH);
        backslash = false;
        ret.insert(ret.end(), whitespaces.begin(), whitespaces.end());
        whitespaces.clear();
      }

      if (pt == UNICODE_BACKSLASH) {
        backslash = true;
        return;
      }

      ret.push_back(pt);
    };

    for (auto pt : code)
      parse_pt(pt);

    if (!source_is_empty && (ret.empty() || ret.back() == UNICODE_NEWLINE)) parse_pt(UNICODE_NEWLINE);

    return ret;
  }

} // namespace ivl::cppp
