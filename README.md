# ALL
everything, usually smallish



## TODO

move src/ to ivl/

move ivl script to an isolated dir so it can be added to PATH

add .bashrc


header src/bla/truc/znj.hpp can be included as <ivl/bla/truc/znj>
header src/bla/truc/default.hpp can be included as <ivl/bla/truc>
if there is a corresponding .cpp , including the header implies linking with the source file TODO NOT IMPLEMENTED
if a source file doesnt have a corresponding header, it is implicitly an executable TODO NOT IMPLEMENTED


what about non-cpp files?
.sh makes sense to execute
.py as well, with a PYTHONPATH maybe
also other languages probably


how does one introduce tests?
maybe tests are regular executables
but would be nice to run tests specifically maybe

low importance, benchmarky tests

how to introduce dependencies?
like foo/bar wants to run bla/truc, so need to comple bla/truc.cpp first
probably do whatever comment syntax for the language
```
// IVL DEPENDS_ON(foo/bar)
# IVL DEPENDS_ON(foo/bar)
```
