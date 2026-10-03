// #include <string>

struct S {
  S(){}
  ~S(){}
};

struct E {
  // TODO
  // inline static thread_local std::string t = "";
  
  inline static thread_local S t{};

  E() { auto&& _ = t; }
};

// IVL disable_ivl_main_handler()

int main() {}

// /home/ilazaric/repos/ALL/submodules/objdir/edg/bin/eccp --c++26 -O3 -o bug bug.cpp
