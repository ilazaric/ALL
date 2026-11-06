set follow-fork-mode child
set breakpoint pending on
set pagination off
set confirm off

set debuginfod enabled on
set debuginfod verbose 0

file /usr/libexec/gcc/x86_64-linux-gnu/13/cc1plus
set args -quiet -imultiarch x86_64-linux-gnu -D_GNU_SOURCE ubsan-bug.cpp -mtune=generic -march=x86-64 -fasynchronous-unwind-tables -fstack-protector-strong -Wformat -Wformat-security -fstack-clash-protection -fcf-protection -dumpbase ubsan-bug.cpp -dumpbase-ext .cpp

break main
run
delete

set $merc = 0
break mark_exp_read
commands
set $merc = $merc + 1
continue
end
continue
printf "merc = %i\n", $merc
quit

# merc = 410
