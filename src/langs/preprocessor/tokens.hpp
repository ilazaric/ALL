#pragma once

namespace ivl::langs::pp {

  // ignoring modules

  struct HeaderName;
  struct Identifier;
  struct PpNumber;
  struct CharacterLiteral;
  struct UserDefinedCharacterLiteral;
  struct StringLiteral;
  struct UserDefinedStringLiteral;
  struct PreprocessingOpOrFunc;
  struct Character;

  struct Token {
    std::variant<
      HeaderName, Identifier, PpNumber, CharacterLiteral, UserDefinedCharacterLiteral, StringLiteral,
      UserDefinedStringLiteral, PreprocessingOpOrFunc, Character>
      kind;
  };

  struct GroupPart {
    std::variant<ControlLine, IfSection, TextLine, 
  };

  struct File {
    std::vector<GroupPart> group;
  };

} // namespace ivl::langs::pp
