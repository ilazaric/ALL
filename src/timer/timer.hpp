#pragma once

#include <chrono>
#include <iostream>
#include <time.h>
#include <iomanip>
#include <vector>

#ifdef IVL_USE_RDTSC
# include <x86intrin.h>
#endif

namespace ivl::timer {

  // doesnt make much diff
  // TODO: refactor, logger has same
  namespace detail {
    
    // a string that can be passed via template args
    template <unsigned N> struct fixed_string {
      char buf[N + 1]{};
      consteval fixed_string(char const *s) {
        for (unsigned i = 0; i != N; ++i)
          buf[i] = s[i];
      }
      consteval operator char const *() const { return buf; }
      consteval operator std::string_view() const { return {buf}; }
    };
    template <unsigned N> fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

    // a std::source_location that can be passed via template args
    template <std::uint_least32_t linet, // std::uint_least32_t columnt,
              fixed_string file_namet, fixed_string function_namet>
    struct fixed_source_location {
      inline constexpr static auto line = linet;
      // constexpr static inline auto column = columnt;
      inline constexpr static auto file_name = file_namet;
      inline constexpr static auto function_name = function_namet;
    };

  } // namespace detail

  std::int64_t gettimestamp(){
#ifdef IVL_USE_RDTSC
    return __rdtsc();
#else
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (std::int64_t)ts.tv_sec * 1'000'000'000 + (std::int64_t)ts.tv_nsec;
#endif
  }

  struct StaticCollection {
    std::string_view file;
    std::string_view fn;
    std::size_t line;

    // std::vector<std::uint64_t> data;
    std::uint64_t sum = 0;
    std::uint64_t cnt = 0;
    
    StaticCollection(std::string_view file, std::string_view fn, std::size_t line)
      : file(file),
        fn(fn),
        line(line)//,
        /*data()*/{}

    void add_timing(std::uint64_t duration_ns){
      sum += duration_ns;
      cnt += 1;
      // data.push_back(duration_ns);
    }

    ~StaticCollection(){
      std::cerr << std::endl;
      std::cerr << "TIMING DATA FOR " << file << ":" << fn << "(" << line << ")\n";
      std::cerr << "TOTAL CALL COUNT: " << cnt/*data.size()*/ << std::endl;
      // std::uint64_t sec = 0;
      std::uint64_t nsec = 0;
      nsec = sum;
      // for (auto el : data){
      //   sec += el / 1'000'000'000;
      //   nsec += el % 1'000'000'000;
      // }
      // sec += nsec / 1'000'000'000;
      // nsec %= 1'000'000'000;
      std::cerr << "TOTAL TIME SPENT: " << // sec << "s " << std::setfill('0') << std::setw(9) << 
        nsec // << "ns"
                << std::endl;
      std::cerr << "AVERAGE TIME SPENT: " << (double)nsec / cnt << std::endl;
    }
  };

  struct ScopeTimer {
    StaticCollection& collection;
    std::int64_t start_ts;
    
    ScopeTimer(StaticCollection& collection) : collection(collection), start_ts(gettimestamp()){}
    
    ~ScopeTimer(){
      collection.add_timing((std::uint64_t)(gettimestamp() - start_ts));
    }
  };

  template<typename CSL>
  struct BetterCollection : StaticCollection {
    BetterCollection() : StaticCollection(CSL::file_name, CSL::function_name, CSL::line){}
  };

  template<typename CSL>
  BetterCollection<CSL> global_better_collection;
  
} // namespace ivl::timer

#ifndef IVL_GLOBAL_SCOPE
# define SCOPE_TIMER \
  static ::ivl::timer::StaticCollection timer_static_collection(__FILE__, __func__, __LINE__); \
  ::ivl::timer::ScopeTimer timer_scope_timer(timer_static_collection);
#else
# define SCOPE_TIMER \
  using BLA = ::ivl::timer::detail::fixed_source_location<__LINE__, __FILE__, __func__>;\
  ::ivl::timer::ScopeTimer timer_scope_timer(                           \
    ::ivl::timer::global_better_collection< \
      BLA\
    >\
  );
#endif
