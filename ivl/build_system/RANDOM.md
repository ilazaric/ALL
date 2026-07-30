need better thinking

```json
{
    "name": "",
    "dependencies": [],
    "outputs": [],
    "exe": <path>,
    "argv": [],
    "envp": [],
    "imported_from": <path>,
    
    
}
```

pp source -> how to build, and what targets exist (regular / test)

`CXX f depends-on CPP f`
`CXX_TEST f depends-on CPP f`

kinda like configure -> build ? in the sense multiple stages

imo i need to be able to state "build\_if\_target"

/foo/{bla,truc}.cpp

build /foo ought to PP /foo/bla.cpp & /foo/truc.cpp,  
then CXX? /foo/bla.ii & CXX? /foo/truc.ii

if the selector is choosing a specific file, it probably should  
require it is buildable?

CXX /dir --> UNDERSTAND /dir --> PP /dir/bla.cpp & PP /dir/truc.cpp

arguably i should also have .o support  
executables are link targets if a header doesnt exist

do i want regexes? prob not  

CXX? /prog.cpp as-if

PP /prog.cpp
then
CXX /prog.cpp if can

EXECUTE /prog.cpp --args foo bar baz
depends-on
CXX /prog.cpp
depends-on
PP /prog.cpp

do i need to mutate my tasks as execution is running?

most of these are super similar, so emitting individual tasks  
is kinda stupid, probably should do ninja-esque rules with variables

build /prog.o , build /prog

are the PP tasks actually kinds of meta-tasks that mutate the dag?

associated: from /prog.o i need to understand i need to PP /prog.cpp

discovered: from PP /prog.cpp i need to learn about capabilities, like CXX /prog.ii or CXX_TEST /prog.ii

should a task be able to "attach" as a dependency onto another task?
