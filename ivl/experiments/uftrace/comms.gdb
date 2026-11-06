set follow-fork-mode child
set breakpoint pending on
set pagination off
# define hook-quit
#     set confirm off
# end
set confirm off

set debuginfod enabled on
set debuginfod verbose 0

file /usr/bin/uftrace
set args record -v -P mark_exp_read /usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp -mtune=generic -march=x86-64 -fasynchronous-unwind-tables -fstack-protector-strong -Wformat -Wformat-security -fstack-clash-protection -fcf-protection -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp

catch syscall execve
run
step
# cc1plus

break main
continue
# run
delete

layout asm

set $merc = 0
break mark_exp_read
commands
set $merc = $merc + 1
# bt
if $merc != 401
continue
else
bt
# break *__dentry__ + 56
# break *__dentry__ + 70
# continue
end
end
continue
# printf "merc = %i\n", $merc
# quit

# break call_init
# break __dentry__
# continue
# break mark_exp_read
# watch *((char*)mark_exp_read + 4)
# watch *((char*)mark_exp_read + 8)
# # break *__dentry__ + 56
# # break *__dentry__ + 70

# show environment
# quit

# # backtrace
# # quit
