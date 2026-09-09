struct base { int a, b, c; };
struct agg : base {};
int main() { agg x{.a = 1, .b = 2, .c = 3}; }
