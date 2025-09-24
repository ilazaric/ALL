// #include "stdio.h"
// #include "unistd.h"

/* int main(void){ */
/*   // printf("Hello World\n"); */
/*   // write(1, "Hello World\n", 12); */
/* } */


int entry_point(void) {
  asm("mov $231, %rax\n"
      "mov $123, %rdi\n"
      "syscall\n");
}
