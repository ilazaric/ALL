#include <atomic>

std::atomic<bool> a{false};

int main() {
  a.notify_all();
  a.wait(false);
}
