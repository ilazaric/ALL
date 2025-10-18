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
