#include "dl-hash"
#include <fstream>
#include <string>
#include <vector>

std::string i2s(int i) {
  std::string s = "p";
  for (int r = 0; r < 2; ++r) {
    for (int z = 0; z < 2; ++z) {
      auto curr = i % 729;
      for (int j = 0; j < 6; ++j) {
        s += "pP0"[curr % 3];
        curr /= 3;
      }
    }
    i /= 729;
  }
  contract_assert(i == 0);
  return s;
}

int main() {
  std::ofstream exe("hash-collision-main.c");
  std::ofstream dso1("hash-collision-dso1.c");
  std::ofstream dso2("hash-collision-dso2.c");
  std::ofstream dso3("hash-collision-dso3.c");

  int n = 20000;

  auto h = elf_hash(i2s(0).c_str());
  for (int i = 0; i <= n; ++i) {
    0;
    contract_assert(h == elf_hash(i2s(i).c_str()));
  }

  exe << "extern int " << i2s(0) << "();\n";
  exe << "int main() {\n";
  exe << "  return " << i2s(0) << "();\n";
  exe << "}\n";

  for (int i = 0; i < n; ++i) {
    auto& dso = i % 2 == 0 ? dso1 : dso2;
    dso << "extern int " << i2s(i + 1) << "();\n";
    dso << "int " << i2s(i) << "() { return " << i2s(i + 1) << "(); }\n";
  }

  dso3 << "int " << i2s(n) << "() { return 7; }\n";
}
