#include "grammar_utils.hpp"

using namespace ivl::cppp::grammar;

struct ConditionallySupportedDirective : UnimplementedTODO {};

struct GroupPart : Or<ControlLine, IfSection, TextLine, And<Literal<"#">, ConditionallySupportedDirective>> {};

struct Group : Or<GroupPart, And<Group, GroupPart>> {};

struct ModuleFile : UnimplementedTODO {};

struct PreprocessingFile : Or<Opt<Group>, ModuleFile> {};

int main() {}
