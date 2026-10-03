#include <vector>

struct E {
  inline static thread_local std::vector<int> t{};

  inline E() { t.emplace_back(0); }
};

// IVL disable_ivl_main_handler()

int main() {}

// /home/ilazaric/repos/ALL/submodules/objdir/edg/bin/eccp --c++26 -O3 -o bug bug.cpp
