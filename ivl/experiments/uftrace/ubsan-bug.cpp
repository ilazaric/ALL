using info = decltype(^^int);

struct refl_span {
  const info* data;
  int length;
  consteval auto begin() const { return data; }
};
