uftrace seems to cause gcc to segfault

```
ubsan-bug.cpp: In member function 'consteval auto ptr::get() const':
ubsan-bug.cpp:3:39: internal compiler error: Segmentation fault
    3 |   consteval auto get() const { return p; }
      |                                       ^
WARN: child terminated by signal: 11: Segmentation fault
PROGRAM FAILED WITH EXIT CODE 2
```

TODO: reduced to filtered-wide-record.sh , continue chasing why it segfaults

conclusion:
`uftrace` overwrites some bytes of `mark_exp_read`,
but a later jmp wants to go to those bytes
