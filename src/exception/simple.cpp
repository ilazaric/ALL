

void thrower() {
  throw 123;
}

void invisible();

struct Super {
  ~Super();
};

void catcher() {
  try {
    Super s;
    invisible();
  } catch (int) {}
}
