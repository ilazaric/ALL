#include "dl-hash"
#include <fstream>
#include <string>
#include <vector>

std::vector<std::string> atoms{
  "ppp0Pp0pP0PPPp000P", "pp0PPPPPPP000p0pPp", "pp00p0PpP0P0PPpP0p", "pPp0P00pPpP00PPPpP", "pPPPp00P0PpPPPP0Pp",
  "pP00PPpPPPp0pp000p", "p0pPP0p00Pp0pPPPp0", "p00Ppp0p0Pp0P0ppP0", "Pp0pP00pp0Pp0p00Pp", "Pp00p0ppPP0PP0ppP0",
  "PPppP0ppPp0p00PP00", "PPp0PpP00PPPpPpp00", "PPPpPP0p0p00pP0pPP", "PPPp00pPp0PPPp0Pp0", "PPPPp0PP0pPpP0PPp0",
  "PPPP000PpPP00pppPp", "P0p0ppPPP0p00pPp0P", "P0p000p0pP0PppP0pp", "P00PPpppp00ppP00p0", "P000Pp0ppP0PPPpPPp",
  "0ppPPpP0p000P0PppP", "0pp00PpppP000PpPp0", "0pPp0pp0PP0P0p0PpP", "0pPPPPpPP0PPPPp0PP", "0pPP0pPPp00pPpPp00",
  "0p00Pp00P0PpppPpp0", "0P0pP0P0PPPpPPppPP", "0P00P00pppppPPp00p", "00P000pPpppPpPpPP0",
};

std::string i2s(int i) {
  std::string s = "p";
  for (int r = 0; r < 3; ++r) {
    s += atoms[i % atoms.size()];
    s += atoms[i % atoms.size()];
    i /= atoms.size();
  }
  contract_assert(i == 0);
  return s;
}

int main() {
  std::ofstream exe("both-hash-collision-main.c");
  std::ofstream dso1("both-hash-collision-dso1.c");
  std::ofstream dso2("both-hash-collision-dso2.c");
  std::ofstream dso3("both-hash-collision-dso3.c");

  int n = 20000;

  auto eh = elf_hash(i2s(0).c_str());
  auto nh = new_hash(i2s(0).c_str());
  for (int i = 0; i <= n; ++i) {
    0;
    contract_assert(eh == elf_hash(i2s(i).c_str()));
    contract_assert(nh == new_hash(i2s(i).c_str()));
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
