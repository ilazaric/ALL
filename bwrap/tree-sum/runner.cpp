#include "api.hpp"

#include <cassert>
#include <iostream>
#include <sys/resource.h>
#include <time.h>
#include <vector>

struct Experiment {
  std::vector<Node> nodes;

  Experiment(uint64_t n) : nodes(n) {
    std::vector<Node**> free_locations{&nodes[0].children[0], &nodes[0].children[1]};
    nodes[0].data = rand();
    for (uint64_t i = 1; i < n; ++i) {
      auto idx            = rand() % free_locations.size();
      auto slot           = free_locations[idx];
      free_locations[idx] = free_locations.back();
      free_locations.pop_back();
      *slot = &nodes[i];
      free_locations.push_back(&nodes[i].children[0]);
      free_locations.push_back(&nodes[i].children[1]);
    }
  }
};

int main() {
  srand(101);
  constexpr uint64_t      N = 1000;
  constexpr uint64_t      M = 10000;
  std::vector<Experiment> experiments;
  std::vector<uint64_t>   results(N);
  for (uint64_t i = 0; i < N; ++i)
    experiments.emplace_back(M);

  auto timepoint = [] {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
  };

  struct rusage usage;
  assert(-1 != getrusage(RUSAGE_SELF, &usage));
  auto start_vol_ctxt   = usage.ru_nvcsw;
  auto start_invol_ctxt = usage.ru_nivcsw;
  auto start            = timepoint();

  for (int j = 0; j < 20; ++j)
    for (uint64_t i = 0; i < N; ++i)
      results[i] = tree_sum(&experiments[i].nodes[0]);

  auto stop = timepoint();
  assert(-1 != getrusage(RUSAGE_SELF, &usage));
  auto end_vol_ctxt   = usage.ru_nvcsw;
  auto end_invol_ctxt = usage.ru_nivcsw;

  std::cout << "Duration: " << stop - start << " seconds" << std::endl;
  std::cout << "Voluntary ctxt switches: " << end_vol_ctxt - start_vol_ctxt << std::endl;
  std::cout << "Nonvoluntary ctxt switches: " << end_invol_ctxt - start_invol_ctxt << std::endl;

  return 0;
}
