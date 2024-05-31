#include <thread>
#include <iostream>
#include <map>
#include <mutex>

struct mycout_expr {
  inline static std::mutex mtx{};
  
  mycout_expr(){mtx.lock();}
  ~mycout_expr(){mtx.unlock();}

  friend const mycout_expr& operator<<(const mycout_expr& self, const auto& val){
    std::cout << val;
    return self;
  }
};

struct mycout_t {
  friend const mycout_expr& operator<<(const mycout_expr&, const auto&);
  operator mycout_expr(){return {};}
} mycout;

struct thread_id_shrink_t {
  std::mutex mtx;
  std::map<std::thread::id, int> cache;
  int operator()(std::thread::id id){
    std::scoped_lock _(mtx);
    auto [it, success] = cache.try_emplace(id, cache.size());
    return it->second;
  }
} thread_id_shrink;

struct Magic {
  std::string_view sv;
  Magic(std::string_view sv) : sv(sv){
    mycout << "constructed " << sv << " at " << thread_id_shrink(std::this_thread::get_id()) << "\n";
  }
  ~Magic(){
    mycout << "destructed " << sv << " at " << thread_id_shrink(std::this_thread::get_id()) << "\n";
  }
};

thread_local Magic foo{"foo"};
thread_local Magic bar{"bar"};

void bla(){
  (void)foo;
  (void)bar;
}

void truc(){
  (void)bar;
  (void)foo;
}

void noop(){
}

void half(){
  (void)bar;
}

int main(){

  std::thread a{bla}, b{truc}, c{noop}, d{half};
  a.join();
  b.join();
  c.join();
  d.join();

  return 0;
}
