#include "fileview.hpp"

#include <ivl/logger/logger.hpp>
using namespace ivl::logger::default_logger;

int main(){
  ivl::fs::FileView fv("data");
  std::int64_t sum = 0;
  // for (auto b : fv.get_remaining())
  //   sum += (char)b;
  LOG(sum);
}
