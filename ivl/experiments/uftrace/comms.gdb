set follow-fork-mode child
set breakpoint pending on
set pagination off
define hook-quit
    set confirm off
end

set debuginfod enabled on
set debuginfod verbose 0

file /usr/bin/uftrace
set args record -v -P mark_exp_read /usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp -mtune=generic -march=x86-64 -fasynchronous-unwind-tables -fstack-protector-strong -Wformat -Wformat-security -fstack-clash-protection -fcf-protection -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp

# set progress-bars enabled off

catch syscall execve
run
step
# cc1plus

# layout asm

# show environment
# quit

# continue

# watch *mark_exp_read + 

break call_init
break __dentry__
continue
break mark_exp_read
watch *((char*)mark_exp_read + 4)
watch *((char*)mark_exp_read + 8)
# break *__dentry__ + 56
# break *__dentry__ + 70

# show environment
# quit

# # backtrace
# # quit
