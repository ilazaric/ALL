uftrace seems to cause gcc to segfault

```
uftrace record --no-libcall -P . /opt/GCC-release/libexec/gcc/x86_64-pc-linux-gnu/17.0.0/cc1plus -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp -quiet -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp -mtune=generic -march=x86-64 -std=c++26 -o /tmp/ccywqEBz.sfg
```

```
ubsan-bug.cpp: In member function 'consteval auto ptr::get() const':
ubsan-bug.cpp:3:39: internal compiler error: Segmentation fault
    3 |   consteval auto get() const { return p; }
      |                                       ^
WARN: child terminated by signal: 11: Segmentation fault
PROGRAM FAILED WITH EXIT CODE 2
```
