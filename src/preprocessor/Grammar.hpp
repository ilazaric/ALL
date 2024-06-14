#pragma once

#include <variant>
#include <ostream>
#include <memory>
#include <optional>

namespace ivl::pp {

  template<typename T>
  std::string name(){
    if constexpr (requires {T::name();}){
      return T::name();
    } else {
      std::string_view sv = __PRETTY_FUNCTION__;
      sv = sv.substr(47);
      sv = sv.substr(0, sv.size()-50);
      std::string out;
      out.push_back(std::tolower(sv[0]));
      for (auto c : sv.substr(1)){
        if (std::isupper(c)){
          out.push_back('-');
          out.push_back(std::tolower(c));
        } else {
          out.push_back(c);
        }
      }
      return out;
    }
  }

  template<typename T>
  void dump_components(std::ostream& os){
    if constexpr (requires {T::dump_components(os);}){
      T::dump_components(os);
      os << std::endl;
    } else {
      os << name<T>() << std::endl;
    }
  }

  template<typename T>
  void describe(std::ostream& os){
    os << name<T>() << ":" << std::endl;
    if constexpr (requires {T::describe_parts(os);}){
      T::describe_parts(os);
    } else {
      os << "  ";
      dump_components<T>(os);
    }
  }

template <typename... Ts> struct Or : std::variant<std::unique_ptr<Ts>...> {
  static void describe_parts(std::ostream& os){
    auto lam = [&]<typename T>{
      os << "  ";
      dump_components<T>(os);
    };
    (lam.template operator()<Ts>(), ...);
  }
};

template <typename T> struct Opt : std::optional<T> {
  static std::string name(){
    return name<T>() + "_opt";
  }
};

template <typename... Ts> struct And : std::tuple<Ts...> {
  static void dump_components(std::ostream& os){
    ((dump_components<Ts>(os), os << " "), ...);
  }
};

  namespace text {
    template <unsigned N> struct fixed_string {
      char buf[N + 1]{};
      constexpr fixed_string(char const *s) {
        for (unsigned i = 0; i != N; ++i)
          buf[i] = s[i];
      }
      constexpr operator char const *() const { return buf; }
    };
    template <unsigned N> fixed_string(char const (&)[N]) -> fixed_string<N - 1>;

    template<fixed_string Str>
    struct Text {
      static std::string_view name(){
        return (char const*)Str;
      }
    };

    struct Module : Text<"module"> {};
    struct Semicolon : Text<";"> {};
    struct Colon : Text<":"> {};
    struct Private : Text<"private"> {};
    struct Hash : Text<"#"> {};
    struct Include : Text<"include"> {};
    struct Define : Text<"define"> {};
    struct Rparen : Text<")"> {};
    struct Ellipsis : Text<"..."> {};
    struct Comma : Text<","> {};
    struct Undef : Text<"undef"> {};
    struct Line : Text<"line"> {};
    struct Error : Text<"error"> {};
    struct Pragma : Text<"pragma"> {};
    struct If : Text<"if"> {};
    struct Ifdef : Text<"ifdef"> {};
    struct Ifndef : Text<"ifndef"> {};
    struct Elif : Text<"elif"> {};
    struct Else : Text<"else"> {};
    struct Endif : Text<"endif"> {};
    struct Export : Text<"export"> {};
    struct Import : Text<"import"> {};
  } // namespace text


struct ModuleFileDefinition;
struct PpGlobalModuleFragmentDefinition;
struct PpPrivateModuleFragmentDefinition;
struct GroupDefinition;
struct GroupPartDefinition;
struct ControlLineDefinition;
struct IfSectionDefinition;
struct IfGroupDefinition;
struct ElifGroupsDefinition;
struct ElifGroupDefinition;
struct ElseGroupDefinition;
struct EndifLineDefinition;
struct TextLineDefinition;
struct ConditionallySupportedDirectiveDefinition;
struct LparenDefinition;
struct IdentifierListDefinition;
struct ReplacementListDefinition;
struct PpTokensDefinition;
struct NewLineDefinition;
  struct PpModuleDefinition;
  struct PpImportDefinition;
  struct ConstantExpressionDefinition;
  struct HCharSequenceDefinition;
  struct QCharSequenceDefinition;

  struct ModuleFile : std::unique_ptr<ModuleFileDefinition> {};
  struct PpGlobalModuleFragment : std::unique_ptr<PpGlobalModuleFragmentDefinition> {};
  struct PpPrivateModuleFragment : std::unique_ptr<PpPrivateModuleFragmentDefinition> {};
  struct Group : std::unique_ptr<GroupDefinition> {};
  struct GroupPart : std::unique_ptr<GroupPartDefinition> {};
  struct ControlLine : std::unique_ptr<ControlLineDefinition> {};
  struct IfSection : std::unique_ptr<IfSectionDefinition> {};
  struct IfGroup : std::unique_ptr<IfGroupDefinition> {};
  struct ElifGroups : std::unique_ptr<ElifGroupsDefinition> {};
  struct ElifGroup : std::unique_ptr<ElifGroupDefinition> {};
  struct ElseGroup : std::unique_ptr<ElseGroupDefinition> {};
  struct EndifLine : std::unique_ptr<EndifLineDefinition> {};
  struct TextLine : std::unique_ptr<TextLineDefinition> {};
  struct ConditionallySupportedDirective : std::unique_ptr<ConditionallySupportedDirectiveDefinition> {};
  struct Lparen : std::unique_ptr<LparenDefinition> {};
  struct IdentifierList : std::unique_ptr<IdentifierListDefinition> {};
  struct ReplacementList : std::unique_ptr<ReplacementListDefinition> {};
  struct PpTokens : std::unique_ptr<PpTokensDefinition> {};
  struct NewLine : std::unique_ptr<NewLineDefinition> {};
  struct PpModule : std::unique_ptr<PpModuleDefinition> {};
  struct PpImport : std::unique_ptr<PpImportDefinition> {};
  struct ConstantExpression : std::unique_ptr<ConstantExpressionDefinition> {};
  struct HCharSequence : std::unique_ptr<HCharSequenceDefinition> {};
  struct QCharSequence : std::unique_ptr<QCharSequenceDefinition> {};

  // dunno
  // struct PpModule;
  struct Identifier;
  struct PreprocessingToken;

  struct PreprocessingFileDefinition : Or<Opt<Group>, ModuleFile> {};
  struct ModuleFileDefinition : And<Opt<PpGlobalModuleFragment>, PpModule, Opt<Group>, Opt<PpPrivateModuleFragment>> {};
  struct PpGlobalModuleFragmentDefinition : And<text::Module, text::Semicolon, NewLine, Opt<Group>> {};
  struct PpPrivateModuleFragmentDefinition : And<text::Module, text::Colon, text::Private, text::Semicolon, NewLine, Opt<Group>> {};
  struct GroupDefinition : Or<GroupPart, And<Group, GroupPart>> {};
  struct GroupPartDefinition : Or<ControlLine, IfSection, TextLine, And<text::Hash, ConditionallySupportedDirective>> {};
  struct ControlLineDefinition : Or<
    And<text::Hash, text::Include, PpTokens, NewLine>,
    PpImport,
    And<text::Hash, text::Define, Identifier, ReplacementList, NewLine>,
    And<text::Hash, text::Define, Identifier, Lparen, Opt<IdentifierList>, text::Rparen, ReplacementList, NewLine>,
    And<text::Hash, text::Define, Identifier, Lparen, text::Ellipsis, text::Rparen, ReplacementList, NewLine>,
    And<text::Hash, text::Define, Identifier, Lparen, IdentifierList, text::Comma, text::Ellipsis, text::Rparen, ReplacementList, NewLine>,
    And<text::Hash, text::Undef, Identifier, NewLine>,
    And<text::Hash, text::Line, Opt<PpTokens>, NewLine>,
    And<text::Hash, text::Error, Opt<PpTokens>, NewLine>,
    And<text::Hash, text::Pragma, Opt<PpTokens>, NewLine>,
    And<text::Hash, NewLine>
    > {};

  struct IfSectionDefinition : And<IfGroup, Opt<ElifGroups>, Opt<ElseGroup>, EndifLine> {};
  struct IfGroupDefinition : Or<
    And<text::Hash, text::If, ConstantExpression, NewLine, Opt<Group>>,
    And<text::Hash, text::Ifdef, Identifier, NewLine, Opt<Group>>,
    And<text::Hash, text::Ifndef, Identifier, NewLine, Opt<Group>>
    > {};

  struct ElifGroupsDefinition : Or<
    ElifGroup,
    And<ElifGroups, ElifGroup>
    > {};

  struct ElifGroupDefinition : And<text::Hash, text::Elif, ConstantExpression, NewLine, Opt<Group>> {};

  struct ElseGroupDefinition : And<text::Hash, text::Else, NewLine, Opt<Group>> {};

  struct EndifLineDefinition : And<text::Hash, text::Endif, NewLine> {};

  struct TextLineDefinition : And<Opt<PpTokens>, NewLine> {};

  struct ConditionallySupportedDirectiveDefinition : And<PpTokens, NewLine> {};

  // TODO
  struct Lparen;

  struct IdentifierListDefinition : Or<Identifier, And<IdentifierList, text::Comma, Identifier>> {};

  struct ReplacementListDefinition : Opt<PpTokens> {};

  struct PpTokensDefinition : Or<PreprocessingToken, And<PpTokens, PreprocessingToken>> {};

  // TODO: this can probably be text::Text<"\n">
  struct NewLineDefinition : text::Text<"\n"> {};

  struct PpModuleDefinition : And<Opt<text::Export>, text::Module, Opt<PpTokens>, text::Semicolon, NewLine> {};

  struct PpImportDefinition : Or<
    And<Opt<text::Export>, text::Import, HeaderName, Opt<PpTokens>, text::Semicolon, NewLine>,
    And<Opt<text::Export>, text::Import, HeaderNameTokens, Opt<PpTokens>, text::Semicolon, NewLine>,
    And<Opt<text::Export>, text::Import, PpTokens, text::Semicolon, NewLine>
    > {};

  struct HeaderNameDefinition : Or<
    And<text::Less, HCharSequence, text::Greater>,
    And<text::DoubleQuote, QCharSequence, text::DoubleQuote>
    > {};

  struct HCharSequenceDefinition : Or<
    HChar,
    And<HCharSequence, HChar>
    > {};

  struct QCharSequenceDefinition : Or<
    QChar,
    And<QCharSequence, QChar>
    > {};
  
} // namespace ivl::pp
