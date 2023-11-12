#pragma once

#include <vector>
#include <string>
#include <functional>
#include <cstddef>
#include <iostream>
#include <cmath>

template<typename T>
struct Tester {
  using Fn = std::function<bool(const T&)>;
  std::vector<Fn> tests;
  std::vector<std::string> names;
  std::vector<std::size_t> fail_counts;
  std::vector<std::array<T, 5>> fail_examples;
  bool started_testing = false;
  std::size_t count = 0;

  void attach(Fn test, std::string name){
    assert(!started_testing);
    tests.push_back(std::move(test));
    names.push_back(std::move(name));
    fail_counts.push_back(0);
    fail_examples.emplace_back();
  }

  void test(const T& f){
    started_testing = true;
    for (std::size_t idx = 0; idx < tests.size(); ++idx)
      if (!(tests[idx](f))){
        auto fidx = fail_counts[idx]++;
        if (fidx >= fail_examples[idx].size())
          continue;
        fail_examples[idx][fidx] = f;
      }
    ++count;
  }

  ~Tester(){
    std::cout << "TESTER RESULTS (count=" << count << "):" << std::endl;
    for (std::size_t idx = 0; idx < tests.size(); ++idx){
      std::cout << names[idx] << " | " << fail_counts[idx] << " | " << round((double)fail_counts[idx] / (double)count * 10000) / 100 << std::endl;
      for (std::size_t fidx = 0; fidx < fail_counts[idx] && fidx < fail_examples[idx].size(); ++fidx){
        std::cout << "FAILURE EXAMPLE #" << fidx << std::endl;
        std::cout << fail_examples[idx][fidx] << std::endl;
      }
    }
  }
};

#define ATTACH_TEST(tester, test) tester.attach(test, #test)
