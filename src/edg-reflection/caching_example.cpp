
template<typename Wrapped>
struct Cached : Wrapped {
  template<typename... Ts>
  
};

struct Example {
  int f(this auto& self, int a){
    std::cerr << "Called f() for a=" << a << std::endl;
    return a >= 2 ? self.g(a-1) + self.g(a-2) : 0;
  }
  int g(this auto& self, int a){
    std::cerr << "Called g() for a=" << a << std::endl;
    return a >= 1 ? self.f(a-1) + 5 : 0;
  }
};
