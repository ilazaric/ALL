#include <mutex>

std::mutex m;

int main(){
  m.lock();
  m.lock();
}
