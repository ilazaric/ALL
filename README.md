# ALL

everything, usually smallish

## TODO

- [ ] add .bashrc
- [ ] if there is a corresponding .cpp , including the header implies linking with the source file
  - [ ] if a source file doesnt have a corresponding header, it is implicitly an executable\
        currently .cpp implies executable
- [ ] what about non-cpp files?\
      .sh makes sense to execute\
      .py as well, with a PYTHONPATH maybe\
      also other languages probably
- [ ] how does one introduce tests?\
      maybe tests are regular executables\
      but would be nice to run tests specifically maybe\
      could be "all targets that end with `_test`"
- [ ] low importance, benchmarky tests
- [ ] how to introduce dependencies?\
      like foo/bar wants to run bla/truc, so need to compile bla/truc.cpp first\
      err, not first but as well\
      probably do whatever comment syntax for the language
      
      ```
      // IVL DEPENDS_ON(foo/bar)
      # IVL DEPENDS_ON(foo/bar)
      ```

or maybe a macro that gives access
IVL_TARGET("foobar")
then the custom preprocessor could figure out the deps

all artifacts should probably end up in build/
unfortunately
kinda like having files in cwd

^ currently only artifacts are executables, and they are placed alongside .cpp

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

^ `ivl build` goes to a python script




consider unique_ptr<S>
also consider S*
or optional<S&>
one is owning, one is non-owning
we could represent both with bits in pointer




exceptions might be bad
suppose a function, we hit return, construct the object, and some destructors throw
IMO the caller should have access to both the return object and all exceptions
currently multiple exceptions terminate, and a single exception kills the return object
so what could the caller do?
one flow:
just continue, use the return value, and let the exceptions propagate up
what does that even mean though, someone needs to take care of the exceptions
maybe caller should be forced to acknowledge exceptions before touching return value
any exception not resolved -> propagate up -> return value gets lost
(which is good imo)

```cpp
A fn(){
  B b; // throws
  C c; // throws
  return A{};
}

auto a = fn() catch (){} catch (){};
```

maybe
whatever, rewind back to multiple exceptions concept
how do we handle that?
