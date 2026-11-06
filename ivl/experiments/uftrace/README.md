uftrace seems to cause gcc to segfault

```
ubsan-bug.cpp: In member function 'consteval auto ptr::get() const':
ubsan-bug.cpp:3:39: internal compiler error: Segmentation fault
    3 |   consteval auto get() const { return p; }
      |                                       ^
WARN: child terminated by signal: 11: Segmentation fault
PROGRAM FAILED WITH EXIT CODE 2
```

conclusion:
`uftrace` overwrites some bytes of `mark_exp_read`,
but a later jmp wants to go to those bytes

reduced demonstration via `simple.cpp`

```
$ g++-13 -march=x86-64 -Oz -g simple.cpp -o simple
$ ./simple
$ uftrace record -P foo ./simple 
WARN: Segmentation fault: address not mapped (addr: 0xffffffffffe9e890)
WARN:  if this happens only with uftrace, please consider -e/--estimate-return option.

WARN: Backtrace from uftrace v0.15 ( x86_64 dwarf python3 luajit tui perf sched dynamic kernel )
WARN: =====================================
WARN: [0] (foo[5555555551b9] <= main[55555555509e])

Please report this bug to https://github.com/namhyung/uftrace/issues.

WARN: child terminated by signal: 11: Segmentation fault
PROGRAM FAILED WITH EXIT CODE 2
```

this is because:
```
$ objdump --disassemble=_Z3fooPi simple
[...]
00000000000011b0 <_Z3fooPi>:
    11b0:	f3 0f 1e fa          	endbr64
    11b4:	53                   	push   %rbx
    11b5:	48 89 fb             	mov    %rdi,%rbx
    11b8:	48 89 df             	mov    %rbx,%rdi
    11bb:	e8 e9 ff ff ff       	call   11a9 <_Z6opaquePi>
    11c0:	85 c0                	test   %eax,%eax
    11c2:	75 06                	jne    11ca <_Z3fooPi+0x1a>
    11c4:	48 83 c3 04          	add    $0x4,%rbx
    11c8:	eb ee                	jmp    11b8 <_Z3fooPi+0x8>
    11ca:	5b                   	pop    %rbx
    11cb:	c3                   	ret
```

the `jmp    11b8 <_Z3fooPi+0x8>` jumps onto chunk `uftrace` overwrites
