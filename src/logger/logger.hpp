#pragma once

/*
  requires c++20
  
  this implements a supercool macro fn `LOG(...)`
  through that macro fn the strings of expressions we want to log
  become accessible to us during compile time, in a pretty nice way

  if you are lazy just include this header and do
  `using namespace ivl::logger::default_logger;`

  if you are ambitious please refer to the implementation of
  `default_logger::logger` class template at the bottom, that should
  clear up how you can implement your own logger,
  you can dump to a specific file instead of stderr,
  add timestamps, whatever you can think of

  the jist is that a class template called `logger` should be visible from
  the point of invocation of `LOG(..)` macro fn
  template args of `logger` should be two types:
  - first type representing the names of all expressions we want to log
  - second type representing the location of `LOG(...)` invocation
  furthermore, it should provide a static member function template
  called `print` that is capable of accepting all arguments passed
  to `LOG(...)`
  most likely you want it to be variadic

  `logger` is a class template bc it seemed to be easiest
  to pass both compile time args (names, location) and runtime args

  you might want to implement an additional class that
  holds the state you need for logging

  this is a pretty simple header, if you don't like something
  just change it in your repository copy :)
 */

#include <array>
#include <iostream> // only used for `default_logger`, remove if unnecessary
#include <string_view>

namespace ivl::logger {

// inline not that important tbh but wtv
inline static constexpr std::array<char, 3> opens = {'(', '\'', '"'};
inline static constexpr std::array<char, 3> closes = {')', '\'', '"'};
static_assert(opens.size() == closes.size());

constexpr std::size_t find_comma(std::string_view names) {
  std::size_t start = 0;
  std::size_t commaloc;
  while (true) {
    commaloc = names.find(',', start);
    if (commaloc == std::string_view::npos)
      break;
    std::pair<std::size_t, std::size_t> openloc = {std::string_view::npos, 0};
    for (std::size_t i = 0; i < opens.size(); ++i)
      openloc = std::min(
          openloc, {names.substr(start, commaloc - start).find(opens[i]), i});
    if (openloc.first == std::string_view::npos)
      break;
    std::size_t closeloc =
        names.substr(start + openloc.first + 1).find(closes[openloc.second]);
    start += openloc.first + closeloc + 1 + 1;
  }
  return commaloc;
}

static_assert(find_comma("(,),x") == 3);
static_assert(find_comma("',',x") == 3);
static_assert(find_comma("\",\",x") == 3);

constexpr void callback_names(std::string_view allnames, auto &callback) {
  while (true) {
    std::size_t commaloc = find_comma(allnames);
    std::string_view name = allnames.substr(0, commaloc);
    if (name.starts_with(' '))
      name = name.substr(1);
    callback(name);
    if (commaloc == std::string_view::npos)
      return;
    allnames = allnames.substr(commaloc + 1);
  }
}

constexpr std::size_t count_names(std::string_view allnames) {
  struct {
    std::size_t count = 0;
    constexpr void operator()(std::string_view) { ++count; }
  } callback;
  callback_names(allnames, callback);
  return callback.count;
}

// `constexpr` has become pretty powerful, love to see it
template <std::size_t N>
constexpr auto generate_names(std::string_view allnames) {
  struct {
    std::size_t index = 0;
    std::array<std::string_view, N> names;
    constexpr void operator()(std::string_view name) { names[index++] = name; }
  } callback;
  callback_names(allnames, callback);
  return callback.names;
}

// a string that can be passed via template args
template <unsigned N> struct fixed_string {
  char buf[N + 1]{};
  constexpr fixed_string(char const *s) {
    for (unsigned i = 0; i != N; ++i)
      buf[i] = s[i];
  }
  constexpr operator char const *() const { return buf; }
};
template <unsigned N> fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

template <fixed_string T> struct name_storage {
  static constexpr std::string_view allnames{(const char *)T};
  static constexpr std::size_t namecount = count_names(allnames);
  static constexpr auto names = generate_names<namecount>(allnames);
};

// a std::source_location that can be passed via template args
  template <std::uint_least32_t linet, // std::uint_least32_t columnt,
          fixed_string file_namet, fixed_string function_namet>
struct fixed_source_location {
  constexpr static inline auto line = linet;
  // constexpr static inline auto column = columnt;
  constexpr static inline auto file_name = file_namet;
  constexpr static inline auto function_name = function_namet;
};

constexpr std::size_t length(const char *ptr) {
  std::size_t len = 0;
  while (ptr[len])
    ++len;
  return len;
}

namespace default_logger {

template <typename NS, typename CSL> struct logger {
  template <typename... Args> static void print(const Args &...args) {
    static_assert(NS::namecount == sizeof...(Args));
    std::cerr << "[LOG] " << CSL::file_name << "(" << CSL::line << "):";
    std::size_t index = 0;
    (
        [](std::size_t &index, const Args &arg) {
          std::cerr << " " << NS::names[index++] << "=" << (arg);
        }(index, args),
        ...);
    std::cerr << std::endl;
  };
};

} // namespace default_logger

} // namespace ivl::logger

#define LOG(...)                                                        \
  do {                                                                  \
    using csl_t = ivl::logger::fixed_source_location<__LINE__, __FILE__, __func__>; \
    using names_t = ivl::logger::name_storage<#__VA_ARGS__>;            \
    logger<names_t, csl_t>::print(__VA_ARGS__);                         \
  } while (0)
