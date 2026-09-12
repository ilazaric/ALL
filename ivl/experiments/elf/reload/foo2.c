extern int puts(const char*);
[[gnu::constructor]] static void init(void) { puts("foo2 init"); }
[[gnu::destructor]] static void fini(void) { puts("foo2 fini"); }
int foo(void) { return 2; }
