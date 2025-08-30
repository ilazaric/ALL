#pragma once

#include "tokenize.hpp"
#include <cstdint>
#include <ivl/util>
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
      if (!has<T>())
        return false;
      pop();
      return true;
    }
    void pop() {
      assert(!empty());
      ++current;
    }
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
    std::variant<LetStatement, AssignStatement, PrintStatement, IfStatement, WhileStatement,
                 BlockStatement>
      kind;
  };

  template <typename T>
  T parse(ParserState&);

  // struct ParenExpression;
  // struct PrimaryExpression;
  // struct UnaryExpression;
  // struct FactorExpression;
  // struct TermExpression;
  // struct ComparisonExpression;
  // struct EqualsExpression;
  // struct AndExpression;
  // struct OrExpression;
  // struct Expression;
  // struct LetStatement;
  // struct AssignStatement;
  // struct PrintStatement;
  // struct IfStatement;
  // struct WhileStatement;
  // struct BlockStatement;
  // struct Statement;

  template <>
  Identifier parse(ParserState& state);
  template <>
  ParenExpression parse(ParserState& state);
  template <>
  PrimaryExpression parse(ParserState& state);
  template <>
  UnaryExpression parse(ParserState& state);
  template <>
  FactorExpression parse(ParserState& state);
  template <>
  TermExpression parse(ParserState& state);
  template <>
  ComparisonExpression parse(ParserState& state);
  template <>
  EqualsExpression parse(ParserState& state);
  template <>
  AndExpression parse(ParserState& state);
  template <>
  OrExpression parse(ParserState& state);
  template <>
  Expression parse(ParserState& state);
  template <>
  LetStatement parse(ParserState& state);
  template <>
  AssignStatement parse(ParserState& state);
  template <>
  PrintStatement parse(ParserState& state);
  template <>
  IfStatement parse(ParserState& state);
  template <>
  WhileStatement parse(ParserState& state);
  template <>
  BlockStatement parse(ParserState& state);
  template <>
  Statement parse(ParserState& state);

  template <>
  Identifier parse(ParserState& state) {
    assert(state.consume_if<identifier_start>());
    Identifier identifier;
    while (!state.consume_if<identifier_end>()) {
      assert(!state.empty());
      state.top().with(util::Overload {[&]<auto c>(identifier_char<c>) { identifier.name += c; },
                                       [](auto&&) { throw std::runtime_error("??? identifier"); }});
      state.pop();
    }
    return identifier;
  }

  template <typename T, typename... Ops>
  T parse_generic_2(ParserState& state) {
    T ret;
    while (true) {
      ret.expressions.emplace_back(parse<std::remove_cvref_t<decltype(ret.expressions[0])>>(state));
      if (!state.has<Ops...>())
        break;
      ret.operators.push_back(state.top());
      state.pop();
    }
    return ret;
  }

  template <>
  ParenExpression parse(ParserState& state) {
    assert(state.consume_if<op_paren_open>());
    util::AtScopeEnd _ {[&] { assert(state.consume_if<op_paren_close>()); }};
    return ParenExpression {std::make_unique<Expression>(parse<Expression>(state))};
  }

  template <>
  PrimaryExpression parse(ParserState& state) {
    if (state.has<kw_false>()) {
      state.pop();
      return PrimaryExpression {False {}};
    }
    if (state.has<kw_true>()) {
      state.pop();
      return PrimaryExpression {True {}};
    }
    if (state.has<identifier_start>()) {
      return PrimaryExpression {parse<Identifier>(state)};
    }

    if (state.has<number_start>()) {
      state.pop();
      Number number;
      while (!state.consume_if<number_end>()) {
        assert(!state.empty());
        state.top().with(
          util::Overload {[&]<auto c>(number_digit<c>) { number.number = number.number * 10 + c; },
                          [](auto&&) { throw std::runtime_error("??? number"); }});
        state.pop();
      }
      return PrimaryExpression {number};
    }

    return PrimaryExpression {parse<ParenExpression>(state)};
  }

  template <>
  UnaryExpression parse(ParserState& state) {
    std::vector<Token> operators;
    while (state.has<op_not, op_minus>()) {
      operators.push_back(state.top());
      state.pop();
    }
    return UnaryExpression {operators, parse<PrimaryExpression>(state)};
  }

  template <>
  FactorExpression parse(ParserState& state) {
    return parse_generic_2<FactorExpression, op_mul, op_div, op_mod>(state);
  }

  template <>
  TermExpression parse(ParserState& state) {
    return parse_generic_2<TermExpression, op_plus, op_minus>(state);
  }

  template <>
  ComparisonExpression parse(ParserState& state) {
    return parse_generic_2<ComparisonExpression, op_lt, op_le, op_gt, op_ge>(state);
  }

  template <>
  EqualsExpression parse(ParserState& state) {
    return parse_generic_2<EqualsExpression, op_eq, op_ne>(state);
  }

  template <>
  AndExpression parse(ParserState& state) {
    AndExpression ret;
    do {
      ret.expressions.emplace_back(parse<EqualsExpression>(state));
    } while (state.consume_if<op_and>());
    return ret;
  }

  template <>
  OrExpression parse(ParserState& state) {
    OrExpression ret;
    do {
      ret.expressions.emplace_back(parse<AndExpression>(state));
    } while (state.consume_if<op_or>());
    return ret;
  }

  template <>
  Expression parse(ParserState& state) {
    return Expression {parse<OrExpression>(state)};
  };

  // struct Statement;
  // struct LetStatement;
  // struct AssignStatement;
  // struct PrintStatement;
  // struct IfStatement;
  // struct WhileStatement;
  // struct BlockStatement;

  template <>
  LetStatement parse(ParserState& state) {
    assert(state.consume_if<kw_let>());
    auto i = parse<Identifier>(state);
    assert(state.consume_if<op_assign>());
    auto e = parse<Expression>(state);
    assert(state.consume_if<op_semicolon>());
    return LetStatement {std::move(i), std::move(e)};
  }

  template <>
  AssignStatement parse(ParserState& state) {
    auto i = parse<Identifier>(state);
    assert(state.consume_if<op_assign>());
    auto e = parse<Expression>(state);
    assert(state.consume_if<op_semicolon>());
    return AssignStatement {std::move(i), std::move(e)};
  }

  template <>
  PrintStatement parse(ParserState& state) {
    assert(state.consume_if<kw_print>());
    assert(state.consume_if<op_paren_open>());
    auto e = parse<Expression>(state);
    assert(state.consume_if<op_paren_close>());
    assert(state.consume_if<op_semicolon>());
    return PrintStatement {std::move(e)};
  }

  template <>
  IfStatement parse(ParserState& state) {
    assert(state.consume_if<kw_if>());
    assert(state.consume_if<op_paren_open>());
    auto e = parse<Expression>(state);
    assert(state.consume_if<op_paren_close>());
    auto                       tb = std::make_unique<Statement>(parse<Statement>(state));
    std::unique_ptr<Statement> fb;
    if (state.has<kw_else>()) {
      state.pop();
      fb = std::make_unique<Statement>(parse<Statement>(state));
    }
    return IfStatement {std::move(e), std::move(tb), std::move(fb)};
  }

  template <>
  WhileStatement parse(ParserState& state) {
    assert(state.consume_if<kw_while>());
    assert(state.consume_if<op_paren_open>());
    auto e = parse<Expression>(state);
    assert(state.consume_if<op_paren_close>());
    auto b = std::make_unique<Statement>(parse<Statement>(state));
    return WhileStatement {std::move(e), std::move(b)};
  }

  template <>
  BlockStatement parse(ParserState& state) {
    assert(state.consume_if<op_curly_open>());
    std::vector<Statement> statements;
    while (!state.consume_if<op_curly_close>()) {
      statements.emplace_back(parse<Statement>(state));
    }
    return BlockStatement {std::move(statements)};
  }

  template <>
  Statement parse(ParserState& state) {
    if (state.has<kw_let>())
      return Statement {parse<LetStatement>(state)};
    if (state.has<kw_while>())
      return Statement {parse<WhileStatement>(state)};
    if (state.has<kw_if>())
      return Statement {parse<IfStatement>(state)};
    if (state.has<op_curly_open>())
      return Statement {parse<BlockStatement>(state)};
    if (state.has<kw_print>())
      return Statement {parse<PrintStatement>(state)};
    return Statement {parse<AssignStatement>(state)};
  }

  struct Program {
    std::vector<Statement> statements;
  };

  Program parse_program(std::vector<Token> tokens) {
    ParserState state;
    state.token_storage = std::move(tokens);
    state.token_storage.emplace(state.token_storage.begin(), op_curly_open {});
    state.token_storage.emplace(state.token_storage.end(), op_curly_close {});
    return Program {parse<BlockStatement>(state).statements};
  }

} // namespace ivl::langs::tiny
