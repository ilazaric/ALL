#include <experimental/meta>

struct S;

consteval {
  queue_injection(^S, ^{
    int fn();
  });
}
