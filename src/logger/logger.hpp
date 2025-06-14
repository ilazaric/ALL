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
#include <cstdint>
#include <iostream> // only used for `default_logger`, remove if unnecessary
#include <string_view>

#ifdef IVL_LOCAL

namespace ivl::logger {

  consteval std::size_t find_comma(std::string_view names) {
    std::size_t openparencount = 0;
    char        instr          = 0;
    for (std::size_t idx = 0; idx < names.size(); ++idx) {
      if (!instr) {
        if (names[idx] == ',' && openparencount == 0)
          return idx;
        if (names[idx] == '(' || names[idx] == '[' || names[idx] == '{')
          ++openparencount;
        if (names[idx] == ')' || names[idx] == ']' || names[idx] == '}')
          --openparencount;
      }
      if (names[idx] == '\'' || names[idx] == '"') {
        if (!instr) {
          instr = names[idx];
          continue;
        }
        if (instr != names[idx])
          continue;
        if (names[idx - 1] == '\\')
          continue;
        instr = 0;
      }
    }
    return std::string_view::npos;
  }

  static_assert(find_comma("(,),x") == 3);
  static_assert(find_comma("',',x") == 3);
  static_assert(find_comma("\",\",x") == 3);
  static_assert(find_comma("find_counterexample(6, 3, ((1<<6)-1) & 0xAB, 1)") ==
                std::string_view::npos);

  consteval void callback_names(std::string_view allnames, auto& callback) {
    while (true) {
      std::size_t      commaloc = find_comma(allnames);
      std::string_view name     = allnames.substr(0, commaloc);
      if (name.starts_with(' '))
        name = name.substr(1);
      callback(name);
      if (commaloc == std::string_view::npos)
        return;
      allnames = allnames.substr(commaloc + 1);
    }
  }

  consteval std::size_t count_names(std::string_view allnames) {
    struct {
      std::size_t    count = 0;
      consteval void operator()(std::string_view) { ++count; }
    } callback;
    callback_names(allnames, callback);
    return callback.count;
  }

  // constexpr has become pretty powerful, love to see it
  template <std::size_t N>
  consteval auto generate_names(std::string_view allnames) {
    struct {
      std::size_t                     index = 0;
      std::array<std::string_view, N> names;
      consteval void                  operator()(std::string_view name) { names[index++] = name; }
    } callback;
    callback_names(allnames, callback);
    return callback.names;
  }

  // a string that can be passed via template args
  template <unsigned N>
  struct fixed_string {
    char buf[N + 1] {};
    consteval fixed_string(char const* s) {
      for (unsigned i = 0; i != N; ++i)
        buf[i] = s[i];
    }
    consteval operator char const*() const { return buf; }
  };
  template <unsigned N>
  fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

  consteval std::size_t length(const char* ptr) {
    std::size_t len = 0;
    while (ptr[len])
      ++len;
    return len;
  }

  template <fixed_string T>
  struct name_storage {
    inline static constexpr std::string_view allnames {(const char*)T, length((const char*)T)};
    inline static constexpr std::size_t      namecount {count_names(allnames)};
    inline static constexpr auto             names {generate_names<namecount>(allnames)};
  };

  // a std::source_location that can be passed via template args
  template <std::uint_least32_t linet, // std::uint_least32_t columnt,
            fixed_string file_namet, fixed_string function_namet>
  struct fixed_source_location {
    inline constexpr static auto line = linet;
    // constexpr static inline auto column = columnt;
    inline constexpr static auto file_name     = file_namet;
    inline constexpr static auto function_name = function_namet;
  };

  template <typename T>
  constexpr T&& discardable_forward(std::remove_reference_t<T>& t) {
    return static_cast<T&&>(t);
  }

  template <typename T>
  constexpr T&& discardable_forward(std::remove_reference_t<T>&& t) {
    return static_cast<T&&>(t);
  }

  namespace default_logger {

    template <typename NS, typename CSL>
    struct logger_hook {
      template <typename... Args>
      [[maybe_unused]] static decltype(auto) print(Args&&... args) {
        static_assert(NS::namecount == sizeof...(Args));
        std::cerr << "[LOG] " << CSL::file_name << ":" << CSL::function_name << "(" << CSL::line
                  << "):";
        std::size_t index = 0;
        ( // should be easy to inline
          [](std::size_t& index, const Args& arg) {
            std::cerr << " " << NS::names[index++] << "=" << (arg);
          }(index, args),
          ...);
        std::cerr << std::endl;
        // this makes `LOG` usable as a wrapper around sub-expressions
        // wrapper around `std::forward` due to `[[nodiscard]]`
        // technically not correct bc comma overloadable
        return (discardable_forward<Args>(args), ...);
      }

      template <typename... Args>
      [[maybe_unused]] static decltype(auto) print_raw(Args&&... args) {
        static_assert(NS::namecount == sizeof...(Args));
        std::cerr << "[LOG] " << CSL::file_name << ":" << CSL::function_name << "(" << CSL::line
                  << "):";
        ((std::cerr << " " << args), ...);
        std::cerr << std::endl;
        return (discardable_forward<Args>(args), ...);
      }
    };

  } // namespace default_logger

} // namespace ivl::logger

#define LOG(...)                                                                                   \
  logger_hook<                                                                                     \
    ::ivl::logger::name_storage<#__VA_ARGS__>,                                                     \
    ::ivl::logger::fixed_source_location<__LINE__, __FILE__, __func__>>::print(__VA_ARGS__)
#define LOG_RAW(...)                                                                               \
  logger_hook<                                                                                     \
    ::ivl::logger::name_storage<#__VA_ARGS__>,                                                     \
    ::ivl::logger::fixed_source_location<__LINE__, __FILE__, __func__>>::print_raw(__VA_ARGS__)
#else
#define LOG(...) (__VA_ARGS__)
#endif
