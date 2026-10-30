struct S { int x; };
void do_something();
void fn(S s){
    [[assume(s.x != 0)]];
    if (s.x == 0) do_something();
}
