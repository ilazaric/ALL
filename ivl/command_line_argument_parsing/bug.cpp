#include <vector>

struct base_exception {
  inline static thread_local std::vector<int> inflight_exceptions{};

  inline base_exception() { inflight_exceptions.emplace_back(0); }
};

// IVL disable_ivl_main_handler()

int main() {}

// /home/ilazaric/repos/ALL/submodules/objdir/edg/bin/eccp --c++26 -O3 --preinclude bug.cpp -o bug /home/ilazaric/repos/ALL/build/empty.cpp
