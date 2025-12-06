# heterogenous return functions

# basic

basic syntax:
```cpp
// A,B,C are types
A|B|C function(args...);
```

excended syntax with labels:
```
l1:A | l2:B | l3:C function(args...);
```

does it make sense to have heterogenous arg?
unsure atm

how would these be used?
1. just propagate
```cpp
A|B fn1();
A|B|C fn2() { if (cond) return fn1(); /* other code */ }
```

2. match?
```cpp
A|B fn1();
void foo();
void bar();
void fn2() {
  match (fn1()) {
    case A a: foo();
    case B b: bar();
  }
}
```

need to be careful around RVO

## relations to exceptions

```cpp
T fn_regular() {
  if (cond) return T{};
  else throw E{};
}

T|E fn_new1() {
  if (cond) return T{};
  else return E{};
}

// this more resembles the regular approach bc of std::exception_ptr
T|unique_ptr<E> fn_new2() {
  if (cont) return T{};
  else return std::make_unique<E>();
}
```

what about catching?
```cpp
T catch_regular() {
  try { fn_regular(); }
  catch (const E&) { ... }
}

T catch_new() {
  // need the ability to express "return T, do something locally with E"
  // TODO
  return fn_new() match E as e { ... };

  match (fn_new()) {
    case T t: return t:
    case E e: ...
  }
}
```

need to be able to match out alternatives as an expression
```cpp
fn_new() match (E e) { ... }
// decltype of ^ is T ?

// H is of T|A|B|...
// H match (T) { ... } , if the block is deduced into X|Y|...
// then decltype is A|B|...|X|Y|...
```

need to be able to return-out-of-surrounding-function, throw, return-out-of-current-match-expression
all with RVO

# impl - clang

branch: ivl_heterogenous_return_functions

how is the return spot chosen?
usually fns exit by asm instruction `ret`
now we have multiple exits though

usual:
caller aligns stack
caller does asm instruction `call`
`call` pushes rip to stack
callee does asm instruction `ret`
`ret` pops stack -> rip
caller de-aligns stack

so i guess i can manually push rip

new, for a function A|B|C fn(...):
caller aligns stack
push C exit
push B exit
caller does asm instruction `call`
`call` pushes rip to stack, A exit
callee pops as much of stack as needed (0 for A, 8 for B, 16 for C)
callee does asm instruction `ret`
`ret` pops stack -> rip
caller de-aligns stack

```
A|B fn1();
A|B|C fn2() {
  return fn1(); // this would be just a jmp
}
```