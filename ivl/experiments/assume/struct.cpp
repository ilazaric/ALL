struct S { bool x; };
void do_something();
void fn(S s){
    [[assume(!s.x)]];
    if (s.x) do_something();
}
