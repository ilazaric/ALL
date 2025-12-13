#pragma once

#include <ivl/util>
#include "tokenize.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ivl::langs::tiny {

  struct Identifier {
    std::string name;
    auto        operator<=>(const Identifier&) const = default;
  };

  struct Number {
    std::int64_t number;
  };

  struct True {};
  struct False {};

  struct Expression;
  struct ParenExpression {
    std::unique_ptr<Expression> expression; // not null
  };
  struct PrimaryExpression {
    std::variant<Identifier, Number, True, False, ParenExpression> kind;
  };
  struct UnaryExpression {
    // ! -
    std::vector<Token> operators;
    PrimaryExpression  expression;
  };
  struct FactorExpression {
    std::vector<UnaryExpression> expressions;
    // * / %
    std::vector<Token> operators; // operators.size() + 1 == expressions.size()
  };
  struct TermExpression {
    std::vector<FactorExpression> expressions;
    // + -
    std::vector<Token> operators; // operators.size() + 1 == expressions.size()
  };
  struct ComparisonExpression {
    std::vector<TermExpression> expressions;
    // < > <= >=
    std::vector<Token> operators; // operators.size() + 1 == expressions.size()
  };
  struct EqualsExpression {
    std::vector<ComparisonExpression> expressions;
    // == !=
    std::vector<Token> operators; // operators.size() + 1 == expressions.size()
  };
  struct AndExpression {
    std::vector<EqualsExpression> expressions;
  };
  struct OrExpression {
    std::vector<AndExpression> expressions;
  };
  struct Expression {
    OrExpression expression;
  };

  struct ParserState {
    std::vector<Token> token_storage;
    size_t             current = 0;
    bool               empty() const { return current == token_storage.size(); }
    Token              top() const { return token_storage[current]; }
    template <typename... Ts>
    bool has() const {
      return !empty() && (false || ... || token_storage[current].has<Ts>());
    }
    template <typename T>
    bool consume_if() {
      if (!has<T>()) return false;
      pop();
      return true;
    }
    void pop() {
      assert(!empty());
      ++current;
    }

    template <typename T>
    T parse();
  };

  struct Statement;

  struct LetStatement {
    Identifier identifier;
    Expression expression;
  };

  struct AssignStatement {
    Identifier identifier;
    Expression expression;
  };

  struct PrintStatement {
    Expression expression;
  };

  struct IfStatement {
    Expression                 conditional;
    std::unique_ptr<Statement> true_branch;  // can't be null
    std::unique_ptr<Statement> false_branch; // can be null
  };

  struct WhileStatement {
    Expression                 condition;
    std::unique_ptr<Statement> body; // can't be null
  };

  struct BlockStatement {
    std::vector<Statement> statements;
  };

  struct Statement {
    std::variant<LetStatement, AssignStatement, PrintStatement, IfStatement, WhileStatement, BlockStatement> kind;
  };

  template <>
  Identifier ParserState::parse() {
    assert(consume_if<identifier_start>());
    Identifier identifier;
    while (!consume_if<identifier_end>()) {
      assert(!empty());
      top().with(
        util::Overload{
          [&]<auto c>(identifier_char<c>) { identifier.name += c; },
          [](auto&&) { throw std::runtime_error("??? identifier"); }
        }
      );
      pop();
    }
    return identifier;
  }

  template <>
  Expression ParserState::parse();

  template <>
  ParenExpression ParserState::parse() {
    assert(consume_if<op_paren_open>());
    util::AtScopeEnd _{[&] { assert(consume_if<op_paren_close>()); }};
    return ParenExpression{std::make_unique<Expression>(parse<Expression>())};
  }

  template <typename T, typename... Ops>
  T parse_generic_2(ParserState& state) {
    T ret;
    while (true) {
      ret.expressions.emplace_back(state.parse<std::remove_cvref_t<decltype(ret.expressions[0])>>());
      if (!state.has<Ops...>()) break;
      ret.operators.push_back(state.top());
      state.pop();
    }
    return ret;
  }

  template <>
  PrimaryExpression ParserState::parse() {
    if (consume_if<kw_false>()) return PrimaryExpression{False{}};
    if (consume_if<kw_true>()) return PrimaryExpression{True{}};
    if (has<identifier_start>()) return PrimaryExpression{parse<Identifier>()};

    if (consume_if<number_start>()) {
      Number number;
      while (!consume_if<number_end>()) {
        assert(!empty());
        top().with(
          util::Overload{
            [&]<auto c>(number_digit<c>) { number.number = number.number * 10 + c; },
            [](auto&&) { throw std::runtime_error("??? number"); }
          }
        );
        pop();
      }
      return PrimaryExpression{number};
    }

    return PrimaryExpression{parse<ParenExpression>()};
  }

  template <>
  UnaryExpression ParserState::parse() {
    std::vector<Token> operators;
    while (has<op_not, op_minus>()) {
      operators.push_back(top());
      pop();
    }
    return UnaryExpression{operators, parse<PrimaryExpression>()};
  }

  template <>
  FactorExpression ParserState::parse() {
    return parse_generic_2<FactorExpression, op_mul, op_div, op_mod>(*this);
  }

  template <>
  TermExpression ParserState::parse() {
    return parse_generic_2<TermExpression, op_plus, op_minus>(*this);
  }

  template <>
  ComparisonExpression ParserState::parse() {
    return parse_generic_2<ComparisonExpression, op_lt, op_le, op_gt, op_ge>(*this);
  }

  template <>
  EqualsExpression ParserState::parse() {
    return parse_generic_2<EqualsExpression, op_eq, op_ne>(*this);
  }

  template <>
  AndExpression ParserState::parse() {
    AndExpression ret;
    do {
      ret.expressions.emplace_back(parse<EqualsExpression>());
    } while (consume_if<op_and>());
    return ret;
  }

  template <>
  OrExpression ParserState::parse() {
    OrExpression ret;
    do {
      ret.expressions.emplace_back(parse<AndExpression>());
    } while (consume_if<op_or>());
    return ret;
  }

  template <>
  Expression ParserState::parse() {
    return Expression{parse<OrExpression>()};
  };

  template <>
  Statement ParserState::parse();

  template <>
  LetStatement ParserState::parse() {
    assert(consume_if<kw_let>());
    auto i = parse<Identifier>();
    assert(consume_if<op_assign>());
    auto e = parse<Expression>();
    assert(consume_if<op_semicolon>());
    return LetStatement{std::move(i), std::move(e)};
  }

  template <>
  AssignStatement ParserState::parse() {
    auto i = parse<Identifier>();
    assert(consume_if<op_assign>());
    auto e = parse<Expression>();
    assert(consume_if<op_semicolon>());
    return AssignStatement{std::move(i), std::move(e)};
  }

  template <>
  PrintStatement ParserState::parse() {
    assert(consume_if<kw_print>());
    assert(consume_if<op_paren_open>());
    auto e = parse<Expression>();
    assert(consume_if<op_paren_close>());
    assert(consume_if<op_semicolon>());
    return PrintStatement{std::move(e)};
  }

  template <>
  IfStatement ParserState::parse() {
    assert(consume_if<kw_if>());
    assert(consume_if<op_paren_open>());
    auto e = parse<Expression>();
    assert(consume_if<op_paren_close>());
    auto                       tb = std::make_unique<Statement>(parse<Statement>());
    std::unique_ptr<Statement> fb;
    if (consume_if<kw_else>()) fb = std::make_unique<Statement>(parse<Statement>());
    return IfStatement{std::move(e), std::move(tb), std::move(fb)};
  }

  template <>
  WhileStatement ParserState::parse() {
    assert(consume_if<kw_while>());
    assert(consume_if<op_paren_open>());
    auto e = parse<Expression>();
    assert(consume_if<op_paren_close>());
    auto b = std::make_unique<Statement>(parse<Statement>());
    return WhileStatement{std::move(e), std::move(b)};
  }

  template <>
  BlockStatement ParserState::parse() {
    assert(consume_if<op_curly_open>());
    std::vector<Statement> statements;
    while (!consume_if<op_curly_close>()) {
      statements.emplace_back(parse<Statement>());
    }
    return BlockStatement{std::move(statements)};
  }

  template <>
  Statement ParserState::parse() {
    if (has<kw_let>()) return Statement{parse<LetStatement>()};
    if (has<kw_while>()) return Statement{parse<WhileStatement>()};
    if (has<kw_if>()) return Statement{parse<IfStatement>()};
    if (has<op_curly_open>()) return Statement{parse<BlockStatement>()};
    if (has<kw_print>()) return Statement{parse<PrintStatement>()};
    return Statement{parse<AssignStatement>()};
  }

  struct Program {
    std::vector<Statement> statements;
  };

  Program parse_program(std::vector<Token> tokens) {
    ParserState state;
    state.token_storage = std::move(tokens);
    Program program;
    while (!state.empty())
      program.statements.emplace_back(state.parse<Statement>());
    return program;
  }

} // namespace ivl::langs::tiny
