# running gdb under emacs gives me a weird warning:
# warning: could not convert 'produce' from the host encoding (ANSI_X3.4-1968) to UTF-32.
# This normally should not happen, please file a bug report.Breakpoint 1 at 0x401106: file foo.c, line 1.
# here i explore why
# procedure:
# run gdb under emacs: `M-x gdb -> gdb -i=mi ./foo`
# attach different gdb to emacs gdb: `gdb -p $PID -x selfgdb.gdb`
# add breakpoint in emacs gdb: `break produce`

# break convert_between_encodings
# commands
# print from
# print to
# continue
# end
# output:
# $1 = 0xca73f0 "ANSI_X3.4-1968"
# $2 = 0xaba325 "UTF-32LE"

# catch syscall write
# commands
# bt
# continue
# end
# outcome:
# ada_fold_name <- cooked_index_functions::expand_symtabs_matching
# because it loops over all encodings

catch throw
commands
info symbol iconv_open
bt
continue
end
# output:
# #0  0x00007ffff6ebb35a in __cxxabiv1::__cxa_throw (obj=0xf9b1e0, tinfo=0x9a9bd8 <typeinfo for gdb_exception_error>, dest=0x9098f0 <gdb_exception_error::~gdb_exception_error()>) at ../../../../src/libstdc++-v3/libsupc++/eh_throw.cc:81
# #1  0x000000000043ae6f in throw_it (ap=0x7fffffffbd48, fmt=0xabda6a "%s.", error=GENERIC_ERROR, reason=RETURN_ERROR) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdbsupport/common-exceptions.cc:203
# #2  throw_verror (error=error@entry=GENERIC_ERROR, fmt=0xabda6a "%s.", ap=ap@entry=0x7fffffffbd48) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdbsupport/common-exceptions.cc:211
# #3  0x000000000043af15 in verror (args=0x7fffffffbd48, string=<optimized out>) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdb/utils.c:190
# #4  error (fmt=<optimized out>) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdbsupport/errors.cc:45
# #5  0x0000000000912312 in perror_with_name (string=<optimized out>, errnum=<optimized out>) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdbsupport/errors.cc:94
# #6  0x0000000000501364 in iconv_wrapper::iconv_wrapper (this=<optimized out>, to=<optimized out>, from=<optimized out>) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdb/charset.c:478
# #7  iconv_wrapper::iconv_wrapper (from=0xca73f0 "ANSI_X3.4-1968", to=0xaba325 "UTF-32LE", this=<synthetic pointer>) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdb/charset.c:474
# #8  convert_between_encodings (from=0xca73f0 "ANSI_X3.4-1968", to=0xaba325 "UTF-32LE", bytes=0xfbc060 "produce", num_bytes=<optimized out>, width=1, output=0x7fffffffbfb0, translit=translit_none)
#     at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdb/charset.c:514
# #9  0x000000000045cd3a in ada_fold_name (name="produce", throw_on_error=false) at /usr/src/gdb-15.1-1ubuntu1~24.04.1/gdb/ada-lang.c:1060
#
# iconv_open(to, from) seems to fail
# it lives in /lib/x86_64-linux-gnu/libc.so.6

show environment

continue
