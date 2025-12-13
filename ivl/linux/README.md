not sure how to handle errors
first approach was the or_syscall_error<T>
issue is an error only knows the error value, not the args
second approach is exceptions
they kinda suck sometimes though
what if first one, but we encode the exception ptr into it instead of just value?
would need to be long size, which is unfortunate
(currently or_syscall_error works with int and short)
is it possible to do on <long ?
maybe by relying on address of object as identifier?
also, is it possible to do this without allocation?
for example, a path can be represented as char[PATH_MAX] i think
nvm on PATH_MAX: https://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
nvm on nvm, looking at linux source, open() syscall cant handle > PATH_MAX (ENAMETOOLONG)
getname_flags fn in fs/namei.c
so what, an elf program header specifying a region for exceptions?
the exceptions would be pretty big :'(
this region cant be thread local imo

conclusion:
errors are reported either as just the error code or with args as well
they are reported as either std::expected-ish or via exception
exception can always be done over std::expected, .unwrap_or_throw()
the "just error code" is in fact important, because it is zero overhead
(same asm as if no type safety was introduced)
the "with args" is complex though
consider pathname
should it be std::string?
that means allocation
it could in theory be char[PATH_MAX]-ish
but still, the or_syscall_error<> type should be small
so the exception shouldnt be placed in it, but a handle
so maybe allocation
could be somewhat alleviated with a block of memory created at elf load time
to not worry about fragmentation, it would never release memory to the block,
but to a free list for that exception kind
(or exception size specific)
in fact, throwing the thin version is kinda meaningless maybe
damn, char[PATH_MAX] approach makes fat exceptions :(


https://www.andrew.cmu.edu/course/14-712-s20/applications/ln/Namespaces_Cgroups_Conatiners.pdf
^ actually might be bad, seems like cgroups v1

https://www.youtube.com/watch?v=sK5i-N34im8
^ again cgroups v1 :'(
TODO: might be good for namespaces though

https://man7.org/conf/ndctechtown2019/Linux-namespaces-NDC-TechTown-2019-Kerrisk.pdf
- https://lwn.net/Articles/531114/
- https://blog.lizzie.io/linux-containers-in-500-loc.html
this one is pretty sick

Linux Containers and Virtualization: A Kernel Perspective
https://link.springer.com/chapter/10.1007/978-1-4842-6283-2_3

