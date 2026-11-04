int* produce() { return 0; }
int deref(int* ptr) { return *ptr; }
int main() { return deref(produce()); }
