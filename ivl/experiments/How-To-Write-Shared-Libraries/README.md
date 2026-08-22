# "How To Write Shared Libraries" by Ulrich Drepper

exploring the pdf here

https://github.com/ilazaric/ALL/issues/30

## how ld.so is built

last step:
```
gcc   -nostdlib -nostartfiles -shared -o /home/ilazaric/repos/ALL/submodules/objdir/glibc/elf/ld.so.new		\
	  -Wl,-z,relro -Wl,-z,nomark-plt -Wl,-z,defs 	\
	  -Wl,-z,pack-relative-relocs \
	  /home/ilazaric/repos/ALL/submodules/objdir/glibc/elf/librtld.os -Wl,--version-script=/home/ilazaric/repos/ALL/submodules/objdir/glibc/ld.map -Wl,--undefined-version		\
	  -Wl,-soname=ld-linux-x86-64.so.2
```
