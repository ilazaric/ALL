#include <ivl/command_line_argument_parsing/annotations>
// IVL add_compiler_flags("-c")
struct [[=ivl::cmdline_parsing::class_basic]]
args {};
int fn();
int ivl_main(args) { return fn(); }
