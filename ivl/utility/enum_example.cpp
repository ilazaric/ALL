#include <ivl/utility/enum>

enum class colors { RED = 2, GREEN = 4, BLUE = 7 };
using checked_colors = ivl::checked_enum<colors>;

static_assert(std::to_underlying(checked_colors::GREEN.get()) == 4);

enum class flags { READ = 1, WRITE = 2, EXEC = 4, SETUID = 0x800 };
using checked_flags = ivl::checked_flag_enum<flags>;

static_assert(std::to_underlying((checked_flags::WRITE | checked_flags::EXEC).get()) == 6);

int main() {}
