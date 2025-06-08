#include <ivl/proc>
#include <ivl/logger>

int main(){
  auto counts = ivl::proc::CtxtSwitchCounts::self();
  LOG(counts.voluntary);
  LOG(counts.nonvoluntary);
}
