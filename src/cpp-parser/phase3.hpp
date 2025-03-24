#pragma once

#include <unicode/utf8.h>
#include <unicode/uchar.h>
#include <vector>
#include <optional>
#include <tuple>

namespace ivl::cppp {

  // API:
  // try_parse(std::span<const UChar32>) -> std::optional<T>
  // missing how much was consumed
  // try_parse(span) -> Opt<T, span> // out span represents left over suffix
  // should the type be able to represent nothing?
  // i feel like i want try_parse_into() actually
  // sequence of tokens parsed into a grammar + semantics, spits out a sequence of tokens?
  // lets go with try_parse() for now
  // should empty be representable?

  // how does one represent diffs?
  // might be application specific
  
  using Stream = std::span<const UChar32>;
  template<typename T> using ParseRet = std::optional<std::pair<T, Stream>>;

  template<typename, typename> struct Bla {};

  struct NotImplemented {
    static ParseRet<NotImplemented> try_parse(Stream){
      std::cerr << "BAD" << std::endl;
      return std::nullopt;
    }
  };
  
  template<typename...> struct Or {};
  
  template<typename... Ts> struct And {
    std::tuple<Ts...> data;
    static ParseRet<And> try_parse(Stream s){
      if constexpr (sizeof...(Ts) == 1){
        auto bla = try_parse<Ts...>(s);
        if (!bla) return std::nullopt;
        return {{And{bla->first}, bla->second}};
      } else {
        
      }
    }
  };
  
  template<typename T> struct Opt {
    std::optional<T> data;
    static ParseRet<Opt> try_parse(Stream s){
      T ret = T::try_parse(s);
      if (!ret) return {{Opt{std::nullopt}, s}};
      return {{Opt{std::move(ret->first)}, ret->second}};
    }
  };

  template<UChar32 C> struct Char {
    static ParseRet<Char> try_parse(Stream s){
      if (s.empty()) return std::nullopt;
      if (s[0] != C) return std::nullopt;
      return {{{}, s.subspan(1)}};
    }
  };

  struct HChar : Bla<HChar, NotImplemented> {};
  struct HCharSequence : Bla<HCharSequence, And<HChar, Opt<HCharSequence>>> {};

  struct QChar : Bla<QChar, NotImplemented> {};
  struct QCharSequence : Bla<QCharSequence, And<QChar, Opt<QCharSequence>>> {};

  struct HeaderName :
    Bla<HeaderName,
        Or<
          And<Char<'<'>, HCharSequence, Char<'>'>>,
          And<Char<'"'>, QCharSequence, Char<'"'>>
          >
        > {};

  struct PreprocessingTokenOrWhitespace :
    Bla<PreprocessingTokenOrWhitespace,
        Or<
          HeaderName,
          ImportKeyword,
          ModuleKeyword,
          ExportKeyword,
          Identifier,
          PpNumber,
          CharacterLiteral,
          UserDefinedCharacterLiteral,
          StringLiteral,
          UserDefinedStringLiteral,
          PreprocessingOpOrPunc,
          NonWhitespaceCharacter,
          // ^ above is PreprocessingToken
          WhitespaceCharacter
          >> {};

  std::vector<PreprocessingTokenOrWhitespace> phase3(std::span<const UChar32> code){
    
  }
  
} // namespace ivl::cppp
