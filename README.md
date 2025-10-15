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

all artifacts should probably end up in build/
unfortunately
kinda like having files in cwd

so i want .ivl files
if root/.ivl and root/subdir/.ivl , it should load them in that order
what would they contain?
currently they are shell files
i dont really want to go into dsl
python seems like an okay choice
but i dont like python
so why dont i stick with shell?
wait
i want this to work relative to target, not cwd
whoops
