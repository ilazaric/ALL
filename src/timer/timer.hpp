#pragma once

#include <chrono>
#include <iostream>
#include <time.h>
#include <iomanip>

namespace ivl::timer {

  std::int64_t gettimestamp(){
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (std::int64_t)ts.tv_sec * 1'000'000'000 + (std::int64_t)ts.tv_nsec;
  }

  struct StaticCollection {
    std::string_view file;
    std::string_view fn;
    std::size_t line;

    std::vector<std::uint64_t> data;
    
    StaticCollection(std::string_view file, std::string_view fn, std::size_t line)
      : file(file),
        fn(fn),
        line(line),
        data(){}

    void add_timing(std::uint64_t duration_ns){
      data.push_back(duration_ns);
    }

    ~StaticCollection(){
      std::cerr << "TIMING DATA FOR " << file << ":" << fn << "(" << line << ")\n";
      std::cerr << "TOTAL CALL COUNT: " << data.size() << std::endl;
      std::uint64_t sec = 0;
      std::uint64_t nsec = 0;
      for (auto el : data){
        sec += el / 1'000'000'000;
        nsec += el % 1'000'000'000;
      }
      sec += nsec / 1'000'000'000;
      nsec %= 1'000'000'000;
      std::cerr << "TOTAL TIME SPENT: " << sec << "s " << std::setfill('0') << std::setw(9) << nsec << "ns" << std::endl;
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
  
} // namespace ivl::timer

#define SCOPE_TIMER \
  static ::ivl::timer::StaticCollection timer_static_collection(__FILE__, __func__, __LINE__); \
  ::ivl::timer::ScopeTimer timer_scope_timer(timer_static_collection);
