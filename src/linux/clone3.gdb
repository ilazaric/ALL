set debuginfod enabled off
set follow-fork-mode child 
set detach-on-fork off
set disable-randomization on
set pagination off
# catch syscall clone3
layout asm
run
