#include <ivl/logger>
#include <ivl/proc>

int main() {
  auto counts = ivl::proc::CtxtSwitchCounts::self();
  LOG(counts.voluntary);
  LOG(counts.nonvoluntary);
}
