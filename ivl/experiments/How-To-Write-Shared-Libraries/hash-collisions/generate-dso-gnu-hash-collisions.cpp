#include "dl-hash"
#include <fstream>
#include <string>
#include <vector>

const std::vector<std::string> atoms{"0rrr", "0rsQ", "0rt0", "0sQr", "0sRQ", "0sS0", "0t0r", "0t1Q", "0t20",
                                     "1Qrr", "1QsQ", "1Qt0", "1RQr", "1RRQ", "1RS0", "1S0r", "1S1Q", "1S20",
                                     "20rr", "20sQ", "20t0", "21Qr", "21RQ", "21S0", "220r", "221Q", "2220"};

std::string i2s(int i) {
  std::string s = "p";
  for (int r = 0; r < 4; ++r) {
    s += atoms[i % atoms.size()];
    i /= atoms.size();
  }
  contract_assert(i == 0);
  return s;
}

int main() {
  std::ofstream exe("gnu-hash-collision-main.c");
  std::ofstream dso1("gnu-hash-collision-dso1.c");
  std::ofstream dso2("gnu-hash-collision-dso2.c");
  std::ofstream dso3("gnu-hash-collision-dso3.c");

  int n = 20000;

  auto h = new_hash(i2s(0).c_str());
  for (int i = 0; i <= n; ++i) {
    0;
    contract_assert(h == new_hash(i2s(i).c_str()));
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
