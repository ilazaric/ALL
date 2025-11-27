# random stuff

## useful apt repos

`sudo add-apt-repository ppa:ubuntu-toolchain-r/test`

## install rust

`curl --proto '=https' --tlsv1.2 https://sh.rustup.rs -sSf | sh`

## install emacs rust-mode

https://github.com/rust-lang/rust-mode
`package-install rust-mode`

## cmake stuff

https://cliutils.gitlab.io/modern-cmake/chapters/intro/installing.html



what if functions could return different types

A|B|C fn();

references and values are kinda weird

rvo?

return deduction?

same type but different return path?

Left(Node)|Right(Node)|Empty traverse();

left:Node | right:Node | empty:void traverse();

to just pass it up `return traverse();`

to access it, a match statement/expression?
would still prefer to be able to do rvo on some of them
but how does that even work

```
match (traverse()) {
  left => return
}
```

how would one be generic over labels?

```
A|B fn();
-->
void fn(A* __A_ret, B* __B_ret);

match fn() {
  A a => ?
  B b => ?
}
```

hmm
one approach is multiple sret ptrs
that feels heavy/bloated though
we eventually only use one of them

i also need to jmp to correct instruction addr
