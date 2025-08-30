#pragma once

#include "parse.hpp"
#include <map>

namespace ivl::langs::tiny {

  struct EvalValue {
    std::variant<int64_t, bool> kind;
    EvalValue(int64_t x) : kind(x){}
    EvalValue(bool x) : kind(x){}
  };

  struct EvalState {
    std::map<Identifier, EvalValue> variables;
  };

  EvalValue evaluate(const Expression&, const EvalState&);

  EvalValue evaluate(const PrimaryExpression& expr, const EvalState& state){
    return std::visit([&]<typename T>(const T& expr) -> EvalValue {
        if constexpr (std::same_as<T, Identifier>) return state.variables.at(expr);
        else if constexpr (std::same_as<T, Number>) return expr.number;
        else if constexpr (std::same_as<T, True>) return true;
        else if constexpr (std::same_as<T, False>) return false;
        else return evaluate(*expr.expression, state);
    }, expr.kind);
  }

  EvalValue evaluate(const UnaryExpression& expr, const EvalState& state){
    auto ret = evaluate(expr.expression, state);
    for (auto op : expr.operators){
      op.with(util::Overload{
          [&](op_not){ret = !std::get<bool>(ret.kind);},
          [&](op_minus){ret = -std::get<int64_t>(ret.kind);},
          [](auto){throw std::runtime_error("nope");}
        });
    }
    return ret;
  }

  EvalValue evaluate(const FactorExpression& expr, const EvalState& state){
    assert(!expr.expressions.empty());
    if (expr.expressions.size() == 1) return evaluate(expr.expressions[0], state);
    auto a = std::get<int64_t>(evaluate(expr.expressions[0], state).kind);
    for (size_t i = 0; i < expr.operators.size(); ++i){
      auto b = std::get<int64_t>(evaluate(expr.expressions[i+1], state).kind);
      expr.operators[i].with(util::Overload{
        [&](op_mul){a *= b;},
        [&](op_div){a /= b;},
        [&](op_mod){a %= b;},
        [](auto){throw std::runtime_error("nope");}
      });
    }
    return a;
  }

  EvalValue evaluate(const TermExpression& expr, const EvalState& state){
    assert(!expr.expressions.empty());
    if (expr.expressions.size() == 1) return evaluate(expr.expressions[0], state);
    auto a = std::get<int64_t>(evaluate(expr.expressions[0], state).kind);
    for (size_t i = 0; i < expr.operators.size(); ++i){
      auto b = std::get<int64_t>(evaluate(expr.expressions[i+1], state).kind);
      expr.operators[i].with(util::Overload{
        [&](op_plus){a += b;},
        [&](op_minus){a -= b;},
        [](auto){throw std::runtime_error("nope");}
      });
    }
    return a;
  }

  EvalValue evaluate(const ComparisonExpression& expr, const EvalState& state){
    assert(!expr.expressions.empty());
    if (expr.expressions.size() == 1) return evaluate(expr.expressions[0], state);
    assert(expr.expressions.size() == 2);
    auto a = std::get<int64_t>(evaluate(expr.expressions[0], state).kind);
    auto b = std::get<int64_t>(evaluate(expr.expressions[1], state).kind);
    return expr.operators[0].with(util::Overload{
        [&](op_le){return a <= b;},
        [&](op_lt){return a < b;},
        [&](op_ge){return a >= b;},
        [&](op_gt){return a > b;},
        [](auto) -> bool {throw std::runtime_error("nope");}
      });
  }

  EvalValue evaluate(const EqualsExpression& expr, const EvalState& state){
    assert(!expr.expressions.empty());
    if (expr.expressions.size() == 1) return evaluate(expr.expressions[0], state);
    assert(expr.expressions.size() == 2);
    auto a = std::get<int64_t>(evaluate(expr.expressions[0], state).kind);
    auto b = std::get<int64_t>(evaluate(expr.expressions[1], state).kind);
    return expr.operators[0].has<op_eq>() ? (a == b) : (a != b);
  }

  EvalValue evaluate(const AndExpression& expr, const EvalState& state){
    if (expr.expressions.size() == 1) return evaluate(expr.expressions[0], state);
    bool value = true;
    for (auto&& e : expr.expressions) value &= std::get<bool>(evaluate(e, state).kind);
    return value;
  }

  EvalValue evaluate(const OrExpression& expr, const EvalState& state){
    if (expr.expressions.size() == 1) return evaluate(expr.expressions[0], state);
    bool value = false;
    for (auto&& e : expr.expressions) value |= std::get<bool>(evaluate(e, state).kind);
    return value;
  }

  EvalValue evaluate(const Expression& expr, const EvalState& state){
    return evaluate(expr.expression, state);
  }

  void evaluate(const Statement&, EvalState&);

  void evaluate(const AssignStatement& stmt, EvalState& state){
    state.variables.insert_or_assign(stmt.identifier, evaluate(stmt.expression, state));
  }

  void evaluate(const LetStatement& stmt, EvalState& state){
    state.variables.insert_or_assign(stmt.identifier, evaluate(stmt.expression, state));
  }

  void evaluate(const WhileStatement& stmt, EvalState& state){
    while (true){
      auto cond = evaluate(stmt.condition, state);
      assert(std::holds_alternative<bool>(cond.kind));
      if (!std::get<bool>(cond.kind)) break;
      evaluate(*stmt.body, state);
    }
  }

  void evaluate(const IfStatement& stmt, EvalState& state){
    auto cond = evaluate(stmt.conditional, state);
    assert(std::holds_alternative<bool>(cond.kind));
    if (std::get<bool>(cond.kind)) evaluate(*stmt.true_branch, state);
    else if (stmt.false_branch) evaluate(*stmt.false_branch, state);
  }

  void evaluate(const PrintStatement& stmt, EvalState& state){
    auto value = evaluate(stmt.expression, state);
    std::visit([](auto value){ std::cout << std::boolalpha << value << std::endl;}, value.kind);
  }

  void evaluate(const BlockStatement& stmt, EvalState& state){
    for (auto&& s : stmt.statements) evaluate(s, state);
  }

  void evaluate(const Statement& stmt, EvalState& state){
    std::visit([&](auto&& stmt){evaluate(stmt, state);}, stmt.kind);
  }

  void evaluate(const Program& program){
    EvalState state;
    // int i = 0;
    for (auto&& stmt : program.statements){
      // std::cerr << "stmt #" << i++ << std::endl;
      evaluate(stmt, state);
    }
  }
  
} // namespace ivl::langs::tiny
