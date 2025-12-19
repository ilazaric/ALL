// generated, dont ask how
// format: X(NUMBER_OF_ARGUMENTS, NAME, TYPE_OF_ARG1, NAME_OF_ARG1, TYPE_OF_ARG2, NAME_OF_ARG2, ...)
// these need to be treated carefully usually via asm call directly after

X(0, vfork)
X(5, clone, unsigned long, clone_flags, unsigned long, newsp, int *, parent_tidptr, int *, child_tidptr, unsigned long, tls)
X(2, clone3, struct clone_args *, uargs, size_t, size)

#undef X
