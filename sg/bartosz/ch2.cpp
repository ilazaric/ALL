#include <map>
#include <type_traits>
#include <vector>
#include <any>

#include <ivl/logger>

struct AnyComparable {
  AnyComparable(const auto& arg) requires(!std::is_same_v<AnyComparable, std::decay_t<decltype(arg)>>){
    using T = std::decay_t<decltype(arg)>;
    data = new T (std::forward<decltype(arg)>(arg));
    cmper = [](void* lhs, void* rhs){return *static_cast<T*>(lhs) <=> *static_cast<T*>(rhs);};
    copyer = [](void* arg){return new T (*static_cast<T*>(arg));};
    deleter = [](void* arg){delete static_cast<T*>(arg);};
  }

  AnyComparable(const AnyComparable& o){
    if (o.data == nullptr){
      data = nullptr;
      cmper = nullptr;
      copyer = nullptr;
      deleter = nullptr;
      return;
    }

    data = o.copyer(o.data);
    cmper = o.cmper;
    copyer = o.copyer;
    deleter = o.deleter;
  }

  void unsafe_clear(){
    data = nullptr;
    cmper = nullptr;
    copyer = nullptr;
    deleter = nullptr;
  }

  void clear(){
    if (data) deleter(data);
    unsafe_clear();
  }

  AnyComparable(AnyComparable&& o){
    data = o.data;
    cmper = o.cmper;
    copyer = o.copyer;
    deleter = o.deleter;
    o.unsafe_clear();
  }

  AnyComparable& operator=(const AnyComparable& o){
    if (this == &o) return *this;
    clear();
    data = o.copyer(o.data);
    cmper = o.cmper;
    copyer = o.copyer;
    deleter = o.deleter;
    return *this;
  }
  
  AnyComparable& operator=(AnyComparable&& o){
    if (this == &o) return *this;
    clear();
    data = o.data;
    cmper = o.cmper;
    copyer = o.copyer;
    deleter = o.deleter;
    o.unsafe_clear();
    return *this;
  }

  ~AnyComparable(){
    if (data) deleter(data);
  }

  friend std::strong_ordering operator<=>(const AnyComparable& lhs, const AnyComparable& rhs){
    if (lhs.data == nullptr) return std::strong_ordering::less;
    if (rhs.data == nullptr) return std::strong_ordering::greater;
    if (lhs.cmper != rhs.cmper) return reinterpret_cast<void*>(lhs.cmper) <=> reinterpret_cast<void*>(rhs.cmper);
    return lhs.cmper(lhs.data, rhs.data);
  }

  void* data;
  std::strong_ordering (*cmper)(void*, void*);
  void* (*copyer)(void*);
  void (*deleter)(void*);
};

auto memoize = [](auto&& f){
  return [&, cache = std::map<std::vector<AnyComparable>, std::any>{}](auto&& ... args) mutable {
    using Ret = decltype(f(args...));
    std::vector<AnyComparable> vec{AnyComparable(args)...};
    auto it = cache.find(vec);
    if (it != cache.end()) return std::any_cast<Ret>(it->second);
    std::any generated = f(args...);
    {auto [it, succ] = cache.emplace(std::move(vec), std::move(generated));
      return std::any_cast<Ret>(it->second);}
  };
 };

int main(){


  // "{expr=}";
  // LOG(LOG(3, 2, 1) + LOG(1, 2, 3));
  

  auto mrand = memoize(rand);
  LOG(mrand());
  LOG(mrand());
  LOG(mrand());
  LOG(mrand());
  LOG(mrand());

  LOG(rand());
  LOG(rand());
  LOG(rand());
  LOG(rand());
  LOG(rand());
  
}
