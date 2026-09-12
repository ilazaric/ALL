extern int puts(const char*);
[[gnu::constructor]] static void init(void) { puts("foo1 init"); }
[[gnu::destructor]] static void fini(void) { puts("foo1 fini"); }
int foo(void) { return 1; }
