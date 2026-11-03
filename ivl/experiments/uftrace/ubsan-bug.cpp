struct ptr {
  int* p;
  consteval auto get() const { return p; }
};
