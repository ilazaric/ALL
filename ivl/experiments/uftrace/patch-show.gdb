set follow-fork-mode child
set breakpoint pending on
set pagination off
set confirm off

set debuginfod enabled on
set debuginfod verbose 0

file /opt/uftrace/bin/uftrace
set args record -v -P foo ./simple

catch syscall execve
run
step
# in cc1plus now

break call_init
continue
delete

break mcount_patch_func
continue
disassemble foo
finish
disassemble foo

delete
continue
quit

# outcome:
# Thread 2.1 "simple" hit Breakpoint 3, mcount_patch_func (mdi=0x5555557b16b0, sym=0x55555556bd28, disasm=0x7fff85d98ba0 <disasm>, min_size=0) at /home/ilazaric/repos/uftrace/arch/x86_64/mcount-dynamic.c:593
# 593	{
# Dump of assembler code for function _Z3fooPi:
#    0x00005555555551b0 <+0>:	endbr64
#    0x00005555555551b4 <+4>:	push   %rbx
#    0x00005555555551b5 <+5>:	mov    %rdi,%rbx
#    0x00005555555551b8 <+8>:	mov    %rbx,%rdi
#    0x00005555555551bb <+11>:	call   0x5555555551a9 <_Z6opaquePi>
#    0x00005555555551c0 <+16>:	test   %eax,%eax
#    0x00005555555551c2 <+18>:	jne    0x5555555551ca <_Z3fooPi+26>
#    0x00005555555551c4 <+20>:	add    $0x4,%rbx
#    0x00005555555551c8 <+24>:	jmp    0x5555555551b8 <_Z3fooPi+8>
#    0x00005555555551ca <+26>:	pop    %rbx
#    0x00005555555551cb <+27>:	ret
# End of assembler dump.
# 0x00007fff85d5099e in mcount_patch_func_with_stats (mdi=mdi@entry=0x5555557b16b0, sym=sym@entry=0x55555556bd28) at /home/ilazaric/repos/uftrace/libmcount/dynamic.c:442
# 442		switch (mcount_arch_ops.patch_func(mdi, sym, &disasm, min_size)) {
# Value returned is $1 = 0
# Dump of assembler code for function _Z3fooPi:
#    0x00005555555551b0 <+0>:	endbr64
#    0x00005555555551b4 <+4>:	call   0x555555555ff0
#    0x00005555555551b9 <+9>:	nop
#    0x00005555555551ba <+10>:	nop
#    0x00005555555551bb <+11>:	call   0x5555555551a9 <_Z6opaquePi>
#    0x00005555555551c0 <+16>:	test   %eax,%eax
#    0x00005555555551c2 <+18>:	jne    0x5555555551ca <_Z3fooPi+26>
#    0x00005555555551c4 <+20>:	add    $0x4,%rbx
#    0x00005555555551c8 <+24>:	jmp    0x5555555551b8 <_Z3fooPi+8>
#    0x00005555555551ca <+26>:	pop    %rbx
#    0x00005555555551cb <+27>:	ret
# End of assembler dump.
