#pragma once

#include "meta.hpp"
#include <algorithm>
#include <cctype>
#include <ivl/util>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace ivl::langs::tiny {
  // keywords
  struct kw_let {};
  struct kw_print {};
  struct kw_if {};
  struct kw_else {};
  struct kw_while {};
  struct kw_true {};
  struct kw_false {};

  // operators
  struct op_or {};
  struct op_and {};
  struct op_eq {};
  struct op_ne {};
  struct op_le {};
  struct op_lt {};
  struct op_ge {};
  struct op_gt {};
  struct op_plus {};
  struct op_minus {};
  struct op_mul {};
  struct op_div {};
  struct op_not {};
  struct op_paren_open {};
  struct op_paren_close {};
  struct op_curly_open {};
  struct op_curly_close {};
  struct op_semicolon {};
  struct op_comma {};
  struct op_mod {};
  struct op_assign {};

  struct number_start {};
  template <int Digit>
  struct number_digit {};
  struct number_end {};

  struct identifier_start {};
  template <char Char>
  struct identifier_char {};
  struct identifier_end {};

  template <auto, auto, template <auto> typename>
  struct tl_range;
  template <auto L, template <auto> typename TT>
  struct tl_range<L, L, TT> {
    using type = meta::tl<TT<L>>;
  };
  template <auto L, auto R, template <auto> typename TT>
  struct tl_range {
    using type =
      meta::tl_concat<meta::tl<TT<L>>, typename tl_range<decltype(L)(L + 1), R, TT>::type>::type;
  };

  using kw_tokens = meta::tl<kw_let, kw_print, kw_if, kw_else, kw_while, kw_true, kw_false>;

  using op_tokens =
    meta::tl<op_or, op_and, op_eq, op_ne, op_le, op_lt, op_ge, op_gt, op_plus, op_minus, op_mul,
             op_div, op_not, op_paren_open, op_paren_close, op_curly_open, op_curly_close,
             op_semicolon, op_comma, op_mod, op_assign>;

  // TODO: num 0-9 and lit 0-9 can be same
  using number_tokens =
    meta::tl_concat<meta::tl<number_start, number_end>, tl_range<0, 9, number_digit>::type>::type;

  using identifier_tokens = meta::tl_concat<
    meta::tl<identifier_start, identifier_end>, tl_range<'a', 'z', identifier_char>::type,
    tl_range<'A', 'Z', identifier_char>::type, tl_range<'0', '9', identifier_char>::type>::type;

  struct not_a_token {};

  using all_tokens = meta::tl_concat<meta::tl<not_a_token>, kw_tokens, op_tokens, number_tokens,
                                     identifier_tokens>::type;

  static_assert(meta::tl_length<all_tokens>::value < 256);
  static_assert(meta::tl_is_unique<all_tokens>::value);
  static_assert(meta::tl_find<not_a_token, all_tokens>::value == 0);

  // template<typename, typename...>
  // struct invoke_results;
  // template<typename T>
  // struct invoke_results<T> { using type = meta::tl<>; };
  // template<typename T, typename H, typename... Ts>
  // requires(requires {std::declval<T>()(std::declval<H>());})
  // struct invoke_results<T> {
  //   using type = meta::tl_concat<
  //     meta::tl<decltype(std::declval<T>()(std::declval<H>()))>,
  //     invoke_results<T, Ts...>::type
  //     >;
  // };
  // template<typename T, typename H, typename... Ts>
  // requires(!requires {std::declval<T>()(std::declval<H>());})
  // struct invoke_results<T> {
  //   using type = invoke_results<T, Ts...>::type>;
  // };

  struct Token {
    unsigned char index = 0;

    explicit Token(const auto& arg)
        : index(meta::tl_find<std::remove_cvref_t<decltype(arg)>, all_tokens>::value) {}

    template <typename T>
      requires(meta::tl_contains<T, all_tokens>::value)
    bool has() const {
      return index == meta::tl_find<T, all_tokens>::value;
    }

    bool empty() const { return has<not_a_token>(); }

    decltype(auto) with(auto&& callable) const {
      return []<typename... Ts>(meta::tl<Ts...>, auto&& callable, unsigned char index) {
        template for (constexpr auto tag : {meta::tag<Ts> {}...}) {
          if (index == meta::tl_find<typename decltype(tag)::type, all_tokens>::value) {
            return FWD(callable)(typename decltype(tag)::type {});
          }
        }
        throw std::runtime_error(util::str("bad index: ", index));
        std::unreachable();
      }(all_tokens {}, FWD(callable), index);
      std::unreachable();
    }
  };

  // decltype(auto) with_throw(auto&& callable) const {
  //   return with(overload{
  // 	FWD(callable),
  // 	  [](auto arg) requires(!requires {FWD(callable)(arg);}){
  // 	    throw std::runtime_error(util::str("unhandled type: ", typestr<decltype(arg)>()));
  // 	  }
  //     });
  // }

  // assumes comments are stripped out
  std::vector<Token> tokenize(std::string_view file) {
    std::vector<Token> out;

    while (!file.empty()) {
      if (isspace(file[0])) {
        file.remove_prefix(1);
        continue;
      }

      // all ops
#define X(str, type)                                                                               \
  if (file.starts_with(str)) {                                                                     \
    out.emplace_back(type {});                                                                     \
    file.remove_prefix(std::string_view(str).size());                                              \
    continue;                                                                                      \
  }
      X("||", op_or);
      X("&&", op_and);
      X("==", op_eq);
      X("!=", op_ne);
      X("<=", op_le);
      X("<", op_lt);
      X(">=", op_ge);
      X(">", op_gt);
      X("+", op_plus);
      X("-", op_minus);
      X("*", op_mul);
      X("/", op_div);
      X("!", op_not);
      X("(", op_paren_open);
      X(")", op_paren_close);
      X("{", op_curly_open);
      X("}", op_curly_close);
      X(";", op_semicolon);
      X(",", op_comma);
      X("%", op_mod);
      X("=", op_assign);
#undef X

      // number
      if (isdigit(file[0])) {
        out.emplace_back(number_start {});
        while (!file.empty() && isdigit(file[0])) {
          static constexpr auto rg = std::views::iota(0, 9 + 1);
          static_assert(std::ranges::size(rg) == 10);
          template for (constexpr auto c : rg) {
            if (file[0] - '0' == c) {
              out.emplace_back(number_digit<c> {});
              file.remove_prefix(1);
              break;
            }
          }
        }
        if (!file.empty() && isalpha(file[0]))
          throw std::runtime_error("number + alpha");
        out.emplace_back(number_end {});
        continue;
      }

      // remaining is keyword or identifier
      assert(std::isalpha(file[0]));

      // keywords
#define X(kw, type)                                                                                \
  if (file.starts_with(kw) && (file.size() == std::string_view(kw).size() ||                       \
                               !std::isalnum(file[std::string_view(kw).size()]))) {                \
    out.emplace_back(type {});                                                                     \
    file.remove_prefix(std::string_view(kw).size());                                               \
    continue;                                                                                      \
  }
      X("let", kw_let);
      X("print", kw_print);
      X("if", kw_if);
      X("else", kw_else);
      X("while", kw_while);
      X("true", kw_true);
      X("false", kw_false);

      // only identifier makes sense now
      out.emplace_back(identifier_start {});
      while (!file.empty() && std::isalnum(file[0])) {
        static constexpr auto rg = std::views::concat(std::views::iota('a', (char)('z' + 1)),
                                                      std::views::iota('A', (char)('Z' + 1)),
                                                      std::views::iota('0', (char)('9' + 1)));
        static_assert(std::ranges::size(rg) == 62);
        template for (constexpr auto c : rg) {
          if (c == file[0]) {
            out.emplace_back(identifier_char<c> {});
            file.remove_prefix(1);
            break;
          }
        }
      }
      out.emplace_back(identifier_end {});
    }

    return out;
  }

} // namespace ivl::langs::tiny
