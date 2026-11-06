[[gnu::noinline, gnu::weak]]
int opaque(int* ptr) { return *ptr; }

[[gnu::noinline]]
void foo(int* ptr) {
  switch (opaque(ptr)) {
  case 0: foo(ptr + 1); break;
  default: break;
  }
}

int main() {
  int data[] = {0, 2, 3, 7};
  foo(data);
  return data[0];
}
