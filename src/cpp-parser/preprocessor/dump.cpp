#include <memory>
#include <tuple>
#include <variant>

struct PreprocessingFile;
struct ModuleFile;
struct PpGlobalModuleFragment;
struct PpPrivateModuleFragment;
struct Group;
struct GroupPart;
struct ControlLine;
struct IfSection;
struct IfGroup;
struct ElifGroups;
struct ElifGroup;
struct ElseGroup;
struct EndifLine;
struct TextLine;
struct ConditionallySupportedDirective;
struct Lparen;
struct IdentifierList;
struct ReplacementList;
struct PpTokens;
struct NewLine;
struct DefinedMacroExpression;
struct HPreprocessingToken;
struct HPpTokens;
struct HeaderNameTokens;
struct HasIncludeExpression;
struct HasAttributeExpression;
struct PpModule;
struct PpImport;
struct VaOptReplacement;

struct PreprocessingFile {
  std::unique_ptr<std::variant<std::tuple<std::optional<Group>>, std::tuple<ModuleFile>>> data;
};

struct ModuleFile {
  std::unique_ptr<
    std::variant<std::tuple<std::optional<PpGlobalModuleFragment>, PpModule, std::optional<Group>,
                            std::optional<PpPrivateModuleFragment>>>>
    data;
};

struct PpGlobalModuleFragment {
  std::unique_ptr<
    std::variant<std::tuple<Keyword<"module">, Terminal<";">, NewLine, std::optional<Group>>>>
    data;
};

struct PpPrivateModuleFragment {
  std::unique_ptr<std::variant<std::tuple<Keyword<"module">, Terminal<":">, Keyword<"private">,
                                          Terminal<";">, NewLine, std::optional<Group>>>>
    data;
};

struct Group {
  std::unique_ptr<std::variant<std::tuple<GroupPart>, std::tuple<Group, GroupPart>>> data;
};

struct GroupPart {
  std::unique_ptr<std::variant<std::tuple<ControlLine>, std::tuple<IfSection>, std::tuple<TextLine>,
                               std::tuple<Terminal<"#">, ConditionallySupportedDirective>>>
    data;
};

struct ControlLine {
  std::unique_ptr<
    std::variant<std::tuple<Terminal<"#", "include">, PpTokens, NewLine>, std::tuple<PpImport>,
                 std::tuple<Terminal<"#", "define">, Identifier, ReplacementList, NewLine>,
                 std::tuple<Terminal<"#", "define">, Identifier, Lparen,
                            std::optional<IdentifierList>, Terminal<")">, ReplacementList, NewLine>,
                 std::tuple<Terminal<"#", "define">, Identifier, Lparen, Terminal<"...", ")">,
                            ReplacementList, NewLine>,
                 std::tuple<Terminal<"#", "define">, Identifier, Lparen, IdentifierList,
                            Terminal<",", "...", ")">, ReplacementList, NewLine>,
                 std::tuple<Terminal<"#", "undef">, Identifier, NewLine>,
                 std::tuple<Terminal<"#", "line">, PpTokens, NewLine>,
                 std::tuple<Terminal<"#", "error">, std::optional<PpTokens>, NewLine>,
                 std::tuple<Terminal<"#", "warning">, std::optional<PpTokens>, NewLine>,
                 std::tuple<Terminal<"#", "pragma">, std::optional<PpTokens>, NewLine>,
                 std::tuple<Terminal<"#">, NewLine>>>
    data;
};

struct IfSection {
  std::unique_ptr<std::variant<
    std::tuple<IfGroup, std::optional<ElifGroups>, std::optional<ElseGroup>, EndifLine>>>
    data;
};

struct IfGroup {
  std::unique_ptr<
    std::variant<std::tuple<Terminal<"#", "if">, ConstantExpression, NewLine, std::optional<Group>>,
                 std::tuple<Terminal<"#", "ifdef">, Identifier, NewLine, std::optional<Group>>,
                 std::tuple<Terminal<"#", "ifndef">, Identifier, NewLine, std::optional<Group>>>>
    data;
};

struct ElifGroups {
  std::unique_ptr<std::variant<std::tuple<ElifGroup>, std::tuple<ElifGroups, ElifGroup>>> data;
};

struct ElifGroup {
  std::unique_ptr<std::variant<
    std::tuple<Terminal<"#", "elif">, ConstantExpression, NewLine, std::optional<Group>>,
    std::tuple<Terminal<"#", "elifdef">, Identifier, NewLine, std::optional<Group>>,
    std::tuple<Terminal<"#", "elifndef">, Identifier, NewLine, std::optional<Group>>>>
    data;
};

struct ElseGroup {
  std::unique_ptr<std::variant<std::tuple<Terminal<"#", "else">, NewLine, std::optional<Group>>>>
    data;
};

struct EndifLine {
  std::unique_ptr<std::variant<std::tuple<Terminal<"#", "endif">, NewLine>>> data;
};

struct TextLine {
  std::unique_ptr<std::variant<std::tuple<std::optional<PpTokens>, NewLine>>> data;
};

struct ConditionallySupportedDirective {
  std::unique_ptr<std::variant<std::tuple<PpTokens, NewLine>>> data;
};

struct IdentifierList {
  std::unique_ptr<
    std::variant<std::tuple<Identifier>, std::tuple<IdentifierList, Terminal<",">, Identifier>>>
    data;
};

struct ReplacementList {
  std::unique_ptr<std::variant<std::tuple<std::optional<PpTokens>>>> data;
};

struct PpTokens {
  std::unique_ptr<
    std::variant<std::tuple<PreprocessingToken>, std::tuple<PpTokens, PreprocessingToken>>>
    data;
};

struct DefinedMacroExpression {
  std::unique_ptr<std::variant<std::tuple<Terminal<"defined">, Identifier>,
                               std::tuple<Terminal<"defined", "(">, Identifier, Terminal<")">>>>
    data;
};

struct HPpTokens {
  std::unique_ptr<
    std::variant<std::tuple<HPreprocessingToken>, std::tuple<HPpTokens, HPreprocessingToken>>>
    data;
};

struct HeaderNameTokens {
  std::unique_ptr<
    std::variant<std::tuple<StringLiteral>, std::tuple<Terminal<"<">, HPpTokens, Terminal<">">>>>
    data;
};

struct HasIncludeExpression {
  std::unique_ptr<std::variant<
    std::tuple<Terminal<"__has_include">, Terminal<"(">, HeaderName, Terminal<")">>,
    std::tuple<Terminal<"__has_include">, Terminal<"(">, HeaderNameTokens, Terminal<")">>>>
    data;
};

struct HasAttributeExpression {
  std::unique_ptr<
    std::variant<std::tuple<Terminal<"__has_cpp_attribute", "(">, PpTokens, Terminal<")">>>>
    data;
};

struct PpModule {
  std::unique_ptr<std::variant<std::tuple<std::optional<Keyword<"export">>, Keyword<"module">,
                                          std::optional<PpTokens>, Terminal<";">, NewLine>>>
    data;
};

struct PpImport {
  std::unique_ptr<
    std::variant<std::tuple<std::optional<Keyword<"export">>, Keyword<"import">, HeaderName,
                            std::optional<PpTokens>, Terminal<";">, NewLine>,
                 std::tuple<std::optional<Keyword<"export">>, Keyword<"import">, HeaderNameTokens,
                            std::optional<PpTokens>, Terminal<";">, NewLine>,
                 std::tuple<std::optional<Keyword<"export">>, Keyword<"import">, PpTokens,
                            Terminal<";">, NewLine>>>
    data;
};

struct VaOptReplacement {
  std::unique_ptr<
    std::variant<std::tuple<Terminal<"__VA_OPT__", "(">, std::optional<PpTokens>, Terminal<")">>>>
    data;
};
