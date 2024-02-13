#include <map>
#include <memory>
#include <iostream>
#include <type_traits>

template<typename T>
struct LogPtr {
  T* ptr;
  static LogPtr pointer_to(auto&& r){
    std::cerr << "pointer_to: " << &r << std::endl;
    return {&r};
  }
  LogPtr() : ptr(nullptr){}
  LogPtr(std::nullptr_t) : ptr(nullptr){}
  template<typename U> LogPtr(const LogPtr<U>& p) : ptr(static_cast<T*>(p.ptr)){}
  LogPtr(T* ptr) : ptr(ptr){}
  LogPtr& operator=(const LogPtr&) = default;
  bool operator==(const LogPtr&) const = default;
  T* operator->(){return ptr;}
};

template<typename T>
struct LogAlloc {
  using pointer = LogPtr<T>;
  using size_type = std::size_t;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  pointer allocate(size_type n){throw;}
  void deallocate(pointer p, size_type n){throw;}
};

using Map = std::map<int, int, std::less<int>, LogAlloc<std::pair<const int, int>>>;

int main(){
  int x = 1;
  LogPtr<int>::pointer_to(x);
  Map m;
}
