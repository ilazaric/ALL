#include <ivl/logger>
#include "default"

int main() {
  auto counts = ivl::proc::CtxtSwitchCounts::self();
  LOG(counts.voluntary);
  LOG(counts.nonvoluntary);
}
