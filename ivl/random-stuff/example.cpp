#include <iostream>
#include <memory>
#include <utility>
#include <vector>

constexpr uint32_t N = 10'000'000;

std::vector<uint32_t> children[N];
uint64_t              value[N];
uint32_t              parent[N];

uint64_t recursive_calc(uint32_t id) {
  uint64_t ret = value[id];
  for (auto nxt : children[id])
    ret += recursive_calc(nxt);
  return ret;
}

uint64_t recursive2_calc(uint32_t id) {
  uint64_t ret = value[id];
  for (auto nxt : children[id]) {
    if (children[nxt].size() == 0) ret += value[nxt];
    else ret += recursive2_calc(nxt);
  }
  return ret;
}

uint64_t recursive3_calc_state;
void     recursive3_calc_impl(uint32_t id) {
  recursive3_calc_state += value[id];
  for (auto nxt : children[id])
    recursive3_calc_impl(nxt);
}

uint64_t recursive3_calc(uint32_t id) {
  recursive3_calc_state = 0;
  recursive3_calc_impl(id);
  return recursive3_calc_state;
}

uint64_t shit_iterative_calc(uint32_t root) {
  struct State {
    uint32_t id;
    uint32_t last_child;
    uint64_t ret;
  };
  std::vector<State> stack{{root, 0, value[root]}};
  while (true) {
    auto& current = stack.back();
    if (current.last_child == children[current.id].size()) {
      if (stack.size() == 1) return current.ret;
      auto child_ret = current.ret;
      stack.pop_back();
      stack.back().ret += child_ret;
      continue;
    }
    auto child = children[current.id][current.last_child++];
    stack.push_back({child, 0, value[child]});
  }
  std::unreachable();
}

uint64_t good_iterative_calc(uint32_t root) {
  std::vector<uint32_t> stack{root};
  uint64_t              ret = 0;
  while (!stack.empty()) {
    auto current = stack.back();
    stack.pop_back();
    ret += value[current];
    for (uint32_t child : children[current]) {
      if (children[child].size() == 0) ret += value[child];
      else stack.push_back(child);
    }
  }
  return ret;
}

uint64_t good_iterative2_calc(uint32_t root) {
  std::vector<uint32_t> stack{root};
  uint64_t              ret = value[root];
  while (!stack.empty()) {
    auto current = stack.back();
    stack.pop_back();
    for (uint32_t child : children[current]) {
      ret += value[child];
      if (children[child].size() != 0) stack.push_back(child);
    }
  }
  return ret;
}

uint64_t good_iterative3_calc(uint32_t root) {
  std::vector<uint32_t> stack{root};
  uint64_t              ret = 0;
  while (!stack.empty()) {
    auto current = stack.back();
    stack.pop_back();
    ret += value[current];
    for (uint32_t child : children[current]) {
      while (children[child].size() == 1) {
        ret += value[child];
        child = children[child][0];
      }
      if (children[child].size() == 0) ret += value[child];
      else stack.push_back(child);
    }
  }
  return ret;
}

uint64_t dunno_iterative4_calc(uint32_t root) {
  std::vector<uint32_t> stack{root};
  uint64_t              ret = 0;
  while (!stack.empty()) {
    auto current = stack.back();
    stack.pop_back();
    while (true) {
      ret += value[current];
      if (children[current].size() == 0) break;
      for (uint32_t idx = 1; idx < children[current].size(); ++idx) {
        if (children[children[current][idx]].size() == 0) ret += value[children[current][idx]];
        else stack.push_back(children[current][idx]);
      }
      current = children[current][0];
    }
  }
  return ret;
}

// cheater because uses parent[], unlike others
uint64_t cheater_iterative_calc(uint32_t root) {
  std::vector<uint32_t> stack{0};
  uint32_t              current = root;
  uint64_t              ret     = value[root];
  while (!stack.empty()) {
    if (stack.back() == children[current].size()) {
      stack.pop_back();
      current = parent[current];
      continue;
    }
    uint32_t nxt = children[current][stack.back()++];
    ret += value[nxt];
    if (children[nxt].size() == 0) continue;
    stack.push_back(0);
    current = nxt;
  }
  return ret;
}

int main() {
  srand(42);
  for (int i = 0; i < N; ++i)
    value[i] = rand();
  for (int i = 1; i < N; ++i) {
    parent[i] = rand() % i;
    children[parent[i]].push_back(i);
  }
  parent[0] = 0;

  auto timepoint = [] {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (int64_t)ts.tv_sec * 1'000'000'000 + (int64_t)ts.tv_nsec;
  };

#define EVAL(fn)                                                                                                       \
  do {                                                                                                                 \
    auto start = timepoint();                                                                                          \
    auto value = fn(0);                                                                                                \
    auto end   = timepoint();                                                                                          \
    std::cout << "Experiment for: " #fn << std::endl;                                                                  \
    std::cout << "Value: " << value << std::endl;                                                                      \
    std::cout << "Duration: " << (end - start) / 1e9 << " seconds" << std::endl;                                       \
    std::cout << std::endl;                                                                                            \
  } while (0)

  EVAL(recursive_calc);
  EVAL(recursive2_calc);
  EVAL(recursive3_calc);
  EVAL(shit_iterative_calc);
  EVAL(good_iterative_calc);
  EVAL(good_iterative2_calc);
  EVAL(good_iterative3_calc);
  EVAL(dunno_iterative4_calc);
  EVAL(cheater_iterative_calc);
  EVAL(recursive_calc); // just in case warming up cpu matters

  return 0;
}

/*

  Results:

Experiment for: recursive_calc
Value: 10736046639121121
Duration: 0.679979 seconds

Experiment for: recursive2_calc
Value: 10736046639121121
Duration: 0.676348 seconds

Experiment for: recursive3_calc
Value: 10736046639121121
Duration: 0.713164 seconds

Experiment for: shit_iterative_calc
Value: 10736046639121121
Duration: 0.930249 seconds

Experiment for: good_iterative_calc
Value: 10736046639121121
Duration: 0.552274 seconds

Experiment for: good_iterative2_calc
Value: 10736046639121121
Duration: 0.579998 seconds

Experiment for: good_iterative3_calc
Value: 10736046639121121
Duration: 0.548999 seconds

Experiment for: dunno_iterative4_calc
Value: 10736046639121121
Duration: 0.62533 seconds

Experiment for: cheater_iterative_calc
Value: 10736046639121121
Duration: 1.27104 seconds

Experiment for: recursive_calc
Value: 10736046639121121
Duration: 0.678587 seconds

 */
