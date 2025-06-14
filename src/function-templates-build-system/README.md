this is a POC of how one could split declaration
and definition of function templates into
header and source files

build stuff that depends of lib.hpp
scan for symbols (objdump -t)
generate intermediate lib.tpp that explicitly instantiates
what we need
build, link everything
