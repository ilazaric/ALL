#include "api.hpp"

#include <iostream>
#include <vector>
#include <time.h>

struct Experiment {
  std::vector<Node> nodes;

  Experiment(uint64_t n) : nodes(n){
    std::vector<Node**> free_locations{
      &nodes[0].children[0],
      &nodes[0].children[1]
    };
    nodes[0].data = rand();
    for (uint64_t i = 1; i < n; ++i){
      auto idx = rand() % free_locations.size();
      auto slot = free_locations[idx];
      free_locations[idx] = free_locations.back();
      free_locations.pop_back();
      *slot = &nodes[i];
      free_locations.push_back(&nodes[i].children[0]);
      free_locations.push_back(&nodes[i].children[1]);
    }
  }
};

int main(){
  srand(101);
  constexpr uint64_t N = 1000;
  constexpr uint64_t M = 10000;
  std::vector<Experiment> experiments;
  std::vector<uint64_t> results(N);
  for (uint64_t i = 0; i < N; ++i)
    experiments.emplace_back(M);

  auto timepoint = []{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
  };

  auto start = timepoint();
  
  for (uint64_t i = 0; i < N; ++i)
    results[i] = tree_sum(&experiments[i].nodes[0]);

  auto stop = timepoint();

  std::cout << "Duration: " << stop-start << " seconds" << std::endl;
  
  return 0;
}
