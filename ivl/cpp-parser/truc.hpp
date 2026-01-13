#include <ivl/linux/utils>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

struct cxx_file {
  // We need the original non-spliced file contents for raw string literals and diagnostics.
  std::string original_contents;
  std::string post_splicing_contents;

  // TODO: these might just be unnecessary complexity
  struct origin_ptr {
    const char* ptr;
    const char& operator*() const { return *ptr; }
    const char& operator[](size_t idx) const { return ptr[idx]; }
    std::ptrdiff_t operator-(origin_ptr o) const { return ptr - o.ptr; }
    origin_ptr operator+(std::ptrdiff_t d) const { return {ptr + d}; }
    auto operator<=>(const origin_ptr&) const = default;
  };

  struct splice_ptr {
    const char* ptr;
    const char& operator*() const { return *ptr; }
    const char& operator[](size_t idx) const { return ptr[idx]; }
    std::ptrdiff_t operator-(splice_ptr o) const { return ptr - o.ptr; }
    splice_ptr operator+(std::ptrdiff_t d) const { return {ptr + d}; }
    auto operator<=>(const splice_ptr&) const = default;
  };

  origin_ptr origin_begin() const { return origin_ptr{original_contents.data()}; }
  splice_ptr splice_begin() const { return splice_ptr{post_splicing_contents.data()}; }

  origin_ptr origin_end() const { return origin_ptr{original_contents.data() + original_contents.size()}; }
  splice_ptr splice_end() const { return splice_ptr{post_splicing_contents.data() + post_splicing_contents.size()}; }

  std::map<std::pair<splice_ptr, splice_ptr>, origin_ptr> splicing_reverts;
  std::map<std::pair<origin_ptr, origin_ptr>, splice_ptr> splicing_appliers;

  origin_ptr convert(splice_ptr sp) const {
    assert(splice_begin() <= sp && sp < splice_end());

    auto contains = [&](auto it) {
      return it != splicing_reverts.end() && it->first.first <= sp && sp < it->first.second;
    };
    auto it = splicing_reverts.lower_bound({sp, sp});
    if (contains(it)) return it->second + (it->first.first - sp);
    if (it != splicing_reverts.begin()) {
      --it;
      if (contains(it)) return it->second + (it->first.first - sp);
    }

    if (sp + 1 == splice_end() && !original_contents.ends_with('\n'))
      throw std::runtime_error("ICE: tried to revert synthesized newline at end of file into original file contents");

    throw std::runtime_error(
      std::format("ICE: failed to revert splice_ptr to origin_ptr, index={}", sp - splice_begin())
    );
  }

  splice_ptr convert(origin_ptr op) const {
    assert(origin_begin() <= op && op < origin_end());

    auto contains = [&](auto it) {
      return it != splicing_appliers.end() && it->first.first <= op && op < it->first.second;
    };
    auto it = splicing_appliers.lower_bound({op, op});
    if (contains(it)) return it->second + (it->first.first - op);
    if (it != splicing_appliers.begin()) {
      --it;
      if (contains(it)) return it->second + (it->first.first - op);
    }

    throw std::runtime_error(
      std::format("ICE: failed to revert splice_ptr to origin_ptr, index={}", op - origin_begin())
    );
  }

  std::string_view debug_context_line(origin_ptr op) const {
    // TODO: think if origin_end() is legal
    assert(origin_begin() <= op && op < origin_end());

    auto idx = op - origin_begin();
    auto newline = original_contents.find('\n', idx);
    if (newline == 0) return std::string_view(original_contents).substr(0, 1);

    auto start = original_contents.rfind('\n', newline - (newline != std::string_view::npos)) + 1;
    return std::string_view(original_contents).substr(0, newline).substr(start);
  }

  // TODO: generalize (range, number of lines)
  // TODO: if line is too long, print only snippet of it
  std::string debug_context(origin_ptr op) const {
    // TODO: think if origin_end() is legal
    // ....: here IMO it could be, just say EOF in output
    assert(origin_begin() <= op && op < origin_end());

    auto containing_line = debug_context_line(op);
    auto row = std::ranges::count(std::string_view(origin_begin().ptr, op.ptr), '\n');
    auto col = op.ptr - containing_line.begin();

    std::string ret;
    ret = std::format(
      "{}\n{: <{}}^ here{} (row:{}, col:{})\n", containing_line, "", col, *op == '\n' ? " (newline)" : "", row + 1,
      col + 1
    );
    if (!containing_line.ends_with('\n')) ret += "note: line does not end with newline\n";
    return ret;
  }

  explicit cxx_file(std::string contents) : original_contents(std::move(contents)) {
    std::vector<std::tuple<size_t, size_t, size_t>> reverts;
    auto append = [&](std::string_view sv) {
      reverts.emplace_back(sv.data() - original_contents.data(), post_splicing_contents.size(), sv.size());
      post_splicing_contents += sv;
    };
    std::string_view sv(original_contents);
    while (!sv.empty()) {
      auto newline = sv.find('\n');
      auto backslash = sv.rfind('\\', newline);
      // backspash + non-newline-whitespace + newline gets spliced
      // https://eel.is/c++draft/lex#phases-1.2
      bool has_backslash_whitespace =
        backslash != std::string_view::npos && std::ranges::all_of(sv.substr(0, newline).substr(backslash + 1), &isspace);

      if (newline == std::string_view::npos) {
        if (has_backslash_whitespace)
          throw std::runtime_error(
            "File ends with backslash and whitespace, missing newline\n" + debug_context(origin_ptr{&sv[backslash]})
          );

        append(sv);
        sv = {};
        break;
      }

      append(sv.substr(0, has_backslash_whitespace ? backslash : newline + 1));
      sv.remove_prefix(newline + 1);
    }

    assert(sv.empty());

    for (auto [origin, splice, length] : reverts) {
      if (length == 0) continue;
      splicing_reverts[{splice_begin() + splice, splice_begin() + splice + length}] = origin_begin() + origin;
      splicing_appliers[{origin_begin() + origin, origin_begin() + origin + length}] = splice_begin() + splice;
    }

    // https://eel.is/c++draft/lex#phases-1.2
    // > A source file that is not empty and that (after splicing)
    // > does not end in a new-line character is processed as if
    // > an additional new-line character were appended to the file.
    if (!original_contents.empty() && !post_splicing_contents.ends_with('\n')) post_splicing_contents.push_back('\n');
  }

  struct parsing_state {
    std::string_view splice_remaining;
  };
};
