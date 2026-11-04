#include <iconv.h>
#include <stdio.h>

const char from[] = "ANSI_X3.4-1968";
const char to[] = "UTF-32LE";

int main() {
  auto ret = iconv_open(to, from);
  if (ret == (iconv_t)-1) {
    perror("iconv_open");
    return 1;
  }
}
