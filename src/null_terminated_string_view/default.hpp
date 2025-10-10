#pragma once

#include <string_view>
#include <string>

namespace ivl {

  // just like `std::string_view`, but with a guarantee
  // that one-past-the-end ptr points to a null character
  // useful for c-esque utilities
  //
  // TODO?: `std::string_view` is an alias to
  // - `std::basic_string_view<char, std::char_traits<char>>`
  // - should this also be like that?
  struct null_terminated_string_view {
    std::string_view sv;

    // for default constructor
    static constexpr char empty_string[] = "";

    // * member types
    using value_type             = std::string_view::value_type;
    using pointer                = std::string_view::pointer;
    using const_pointer          = std::string_view::const_pointer;
    using reference              = std::string_view::reference;
    using const_reference        = std::string_view::const_reference;
    using const_iterator         = std::string_view::const_iterator;
    using iterator               = std::string_view::iterator;
    using const_reverse_iterator = std::string_view::const_reverse_iterator;
    using reverse_iterator       = std::string_view::reverse_iterator;
    using size_type              = std::string_view::size_type;
    using difference_type        = std::string_view::difference_type;

    // * constructors & assignment
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0980r1.pdf
#if __cpp_lib_constexpr_string >= 201907L
    constexpr
#endif
      null_terminated_string_view(const std::string& s) noexcept
        : sv(s) {
    }

    constexpr null_terminated_string_view(const char* s) : sv(s ? s : empty_string) {}

    constexpr null_terminated_string_view() noexcept : sv(empty_string) {}

    // TODO: add constructor from std::filesystem::path if needed

    constexpr null_terminated_string_view(const null_terminated_string_view&) noexcept = default;
    constexpr null_terminated_string_view(null_terminated_string_view&&) noexcept      = default;

    constexpr null_terminated_string_view& operator=(const null_terminated_string_view&) noexcept = default;
    constexpr null_terminated_string_view& operator=(null_terminated_string_view&&) noexcept      = default;

    // * destructor
    constexpr ~null_terminated_string_view() noexcept = default;

    // * conversions
    constexpr operator std::string_view() const noexcept { return sv; }

    // * iterators
    constexpr const_iterator begin() const noexcept { return sv.begin(); }
    constexpr const_iterator cbegin() const noexcept { return sv.cbegin(); }

    constexpr const_iterator end() const noexcept { return sv.end(); }
    constexpr const_iterator cend() const noexcept { return sv.cend(); }

    constexpr const_reverse_iterator rbegin() const noexcept { return sv.rbegin(); }
    constexpr const_reverse_iterator crbegin() const noexcept { return sv.crbegin(); }

    constexpr const_reverse_iterator rend() const noexcept { return sv.rend(); }
    constexpr const_reverse_iterator crend() const noexcept { return sv.crend(); }

    // * element access
    constexpr const_reference operator[](std::size_t pos) const { return sv[pos]; }
    constexpr const_reference at(std::size_t pos) const { return sv.at(pos); }

    // even if empty, there is a null character at one-past-the-end
    constexpr const_reference front() const noexcept { return sv.front(); }
    constexpr const_reference back() const noexcept { return sv.back(); }
    constexpr const_pointer   data() const noexcept { return sv.data(); }

    // * capacity
    constexpr size_type size() const noexcept { return sv.size(); }
    constexpr size_type length() const noexcept { return sv.length(); }

    constexpr size_type max_size() const noexcept { return sv.max_size(); }

    [[nodiscard]] constexpr bool empty() const noexcept { return sv.empty(); }

    // * modifiers
    constexpr void remove_prefix(size_type n) { sv.remove_prefix(n); }
    // not implementing `remove_suffix`, can't keep null-terminated invariant
    constexpr void swap(NullStringView& v) noexcept { sv.swap(v.sv); }

    // * operations
    // TODO

    // * constants
    static constexpr size_type npos = std::string_view::npos;
  };

} // namespace ivl
