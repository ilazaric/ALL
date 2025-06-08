#pragma once

#include <cstdint>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <cassert>

#include <sys/resource.h>

#include <ivl/errno>

namespace ivl::proc {

  struct CtxtSwitchCounts {
    size_t voluntary;
    size_t nonvoluntary;

    static CtxtSwitchCounts self(){
//       auto fd = CHECKED(open)("/proc/self/status", O_RDONLY);
//       const auto buf_size = 1ull << 16;
//       char data[buf_size + 1];
//       auto read_cnt = CHECKED(read)(fd, &data[0], buf_size);
//       data[read_cnt] = '\n';
//       CtxtSwitchCounts ret{(size_t)-1, (size_t)-1};
//       for (auto ptr_begin = data, ptr_end = data; ptr_begin != data + read_cnt + 1; ptr_begin = ptr_end + 1){
//         ptr_end = strchr(ptr_begin, '\n');
//         *ptr_end = '\0';
// #define BLA(str, kind) if (strncmp(str, ptr_begin, sizeof(str) - 1) == 0) ret.kind = atoi(ptr_begin + sizeof(str) + 1)
// #define TRUC(kind) BLA(#kind "_ctxt_switches", kind)
//         TRUC(voluntary);
//         TRUC(nonvoluntary);
// #undef BLA
// #undef TRUC
//       }
//       assert(ret.voluntary != -1);
//       assert(ret.nonvoluntary != -1);
//       return ret;
      struct rusage usage;
      CHECKED(getrusage)(RUSAGE_SELF, &usage);
      return {(size_t)usage.ru_nvcsw, (size_t)usage.ru_nivcsw};
    }
  };

} // namespace ivl::proc
