#include "bump_up.hpp"

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <iostream>

template<typename Memory, template<typename> typename TAlloc>
struct Global {
  inline static Memory memory;

  template<typename T>
  struct Alloc {
    using value_type = T;
    T* allocate(size_t n){return TAlloc<T>(memory).allocate(n);}
    void deallocate(T* ptr, size_t n){}
  };
};

template<typename Alloc>
struct Experiment {
  template<typename T>
  using AllocFor = typename std::allocator_traits<Alloc>::rebind_alloc<T>;
  
  template<typename T>
  using Vec = std::vector<T, AllocFor<T>>;
  
  using Str = std::basic_string<char, std::char_traits<char>, AllocFor<char>>;

  template<typename K, typename V>
  using Map = std::map<K, V, std::less<K>, AllocFor<std::pair<const K, V>>>;

  size_t run(Alloc& alloc, size_t seed, size_t repcount){
    srand(seed);
    Map<size_t, Str> vec(std::less<size_t>{}, alloc);
    vec.emplace(0, Str(alloc));
    
    while (--repcount){
      if (rand() % 100 < 5){
        // std::cout << "adding new str" << std::endl;
        vec.emplace(vec.size(), Str(alloc));
      } else {
        size_t idx = rand() % vec.size();
        // std::cout << "adding char to str #" << idx << std::endl;
        vec.at(idx).push_back((char)(rand() % 100));
      }
    }

    size_t h = 0;
    for (auto& [idx, str] : vec){
      h = h * 101 + 42;
      for (auto c : str) h = h * 13337 + c;
    }

    return h;
  }
};

using namespace ivl::bump;

int main(){

  // {
  //   using Alloc = std::allocator<char>;
  //   Experiment<Alloc> exp;
  //   Alloc alloc;
  //   exp.run(alloc, 42, 10000000);
  // }

  {
    using Alloc = BumpUpAlloc<char>;
    Experiment<Alloc> exp;
    BumpUpAllocOwning<(1ULL<<28)> mem;
    Alloc alloc(mem);
    exp.run(alloc, 42, 10000000);
  }
  
}
