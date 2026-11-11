struct A { int z; };
struct B { int x; };
struct D : A, B { int y; };

__attribute__((noinline))
static int f(B& b, bool is_derived) {
    if (__builtin_expect(is_derived, true))
        return static_cast<D&>(b).y;
    return 0;
}

int call(B& b, bool is_derived) {
    return f(b, is_derived);
}

