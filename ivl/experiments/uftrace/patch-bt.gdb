set follow-fork-mode child
set breakpoint pending on
set pagination off
set confirm off

set debuginfod enabled on
set debuginfod verbose 0

file /opt/uftrace/bin/uftrace
set args record -v -P mark_exp_read /usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp -mtune=generic -march=x86-64 -fasynchronous-unwind-tables -fstack-protector-strong -Wformat -Wformat-security -fstack-clash-protection -fcf-protection -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp

catch syscall execve
run
step
# in cc1plus now

break call_init
continue
delete

watch *((char*)mark_exp_read + 8)@12
commands
bt
continue
end

continue
quit

# outcome:
# #0  patch_code (mdi=0x291a830, info=0x7fffffffd188) at /usr/include/x86_64-linux-gnu/bits/string_fortified.h:59
# #1  patch_normal_func (disasm=<optimized out>, sym=0x2718e00, mdi=0x291a830) at /home/ilazaric/repos/uftrace/arch/x86_64/mcount-dynamic.c:525
# #2  mcount_patch_func (mdi=0x291a830, sym=0x2718e00, disasm=<optimized out>, min_size=<optimized out>) at /home/ilazaric/repos/uftrace/arch/x86_64/mcount-dynamic.c:616
# #3  0x00007fff85d5099e in mcount_patch_func_with_stats (mdi=mdi@entry=0x291a830, sym=sym@entry=0x2718e00) at /home/ilazaric/repos/uftrace/libmcount/dynamic.c:442
# #4  0x00007fff85d5108c in patch_normal_func_matched (mdi=0x291a830, map=0x24f0ad0) at /home/ilazaric/repos/uftrace/libmcount/dynamic.c:530
# #5  0x00007fff85d5185d in patch_func_matched (map=0x24f0ad0, mdi=<optimized out>) at /home/ilazaric/repos/uftrace/libmcount/dynamic.c:553
# #6  do_dynamic_update (ptype=<optimized out>, patch_funcs=<optimized out>, sinfo=<optimized out>) at /home/ilazaric/repos/uftrace/libmcount/dynamic.c:576
# #7  mcount_dynamic_update (sinfo=sinfo@entry=0x7fff85d98720 <mcount_sym_info>, patch_funcs=patch_funcs@entry=0x7fffffffed1f "mark_exp_read", ptype=PATT_REGEX) at /home/ilazaric/repos/uftrace/libmcount/dynamic.c:632
# #8  0x00007fff85d55ba2 in mcount_startup () at /home/ilazaric/repos/uftrace/libmcount/mcount.c:2091
# #9  0x00007fff85daa71f in call_init (l=<optimized out>, argc=argc@entry=18, argv=argv@entry=0x7fffffffd758, env=env@entry=0x7fffffffd7f0) at ./elf/dl-init.c:74
