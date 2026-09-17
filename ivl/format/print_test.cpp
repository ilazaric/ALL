#define IVL_FMT_VIA_STD
#include "default"
#include <filesystem>
#include <map>
#include <string>
#include <vector>

// IVL test_only()

void use1(const std::filesystem::path& p) { ivl::fmt::println(stderr, "{:?}", p); }
void use2(const std::vector<std::string>& v) { ivl::fmt::println(stderr, "{::?}", v); }
void use3(const std::map<std::string, std::string>& m) { ivl::fmt::println(stderr, "{}", m); }
