#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <optional>

#include "temp_factory.hpp"
#include "type_factory.hpp"

namespace util {

struct Position;

} // namespace util

namespace sym {

struct Function;
using FunctionPtr = std::shared_ptr<Function>;

struct Value;
using ValuePtr = std::shared_ptr<Value>;

struct Temp;
using TempPtr = std::shared_ptr<Temp>;

struct Variable;
using VariablePtr = std::shared_ptr<Variable>;

struct Constant;
using ConstantPtr = std::shared_ptr<Constant>;

class SymbolTable;

} // namespace sym

namespace sem {

class SemanticContext {
public:
  struct Scope {
    enum class Kind : std::uint8_t {
      FUNC, BLOCKEXPR, IF, ELSE,
      LOOP, FOR, WHILE,
    } kind;
    std::string name;
    std::optional<sym::ValuePtr> val;
  };

public:
  SemanticContext(sym::SymbolTable &symtab)
    : symtab(symtab),
      type_factory(std::make_unique<type::TypeFactory>()),
      temp_factory(std::make_unique<ir::TempFactory>()) {}


public:
  // utils
  void enterFunc(const std::string &name, util::Position pos);
  void enterBlockExpr();
  void enterIf();
  void enterElse();
  void enterLoop();
  void enterFor();
  void enterWhile();

  void exitScope();

  [[nodiscard]]
  auto lookupFunc(const std::string &name) const -> std::optional<sym::FunctionPtr>;
  [[nodiscard]]
  auto lookupVal(const std::string &name) const -> std::optional<sym::ValuePtr>;
  [[nodiscard]]
  auto lookupConst(const std::string &name) const -> std::optional<sym::ConstantPtr>;

  void declareArg(const std::string &name, bool mut,
    type::TypePtr type, util::Position pos);
  auto declareVar(const std::string &name, bool mut, bool init,
    type::TypePtr type, util::Position pos) -> sym::VariablePtr;
  [[nodiscard]]
  auto declareConst(std::variant<int, bool> val, util::Position pos) -> sym::ConstantPtr;

  auto produceArrType(int size, type::TypePtr etype) -> type::TypePtr;
  auto produceTupType(std::vector<type::TypePtr> etypes) -> type::TypePtr;

  void resetTempCnt();
  auto produceTemp(util::Position pos, type::TypePtr type) -> sym::TempPtr;

  void setRetValType(type::TypePtr type);
  [[nodiscard]]
  auto checkAutoTypeInfer() const -> std::vector<sym::ValuePtr>;

  bool inLoopCtx();
  auto getLoopCtx() -> std::optional<Scope*>;

  [[nodiscard]]
  auto getCurScope() const -> Scope;
  [[nodiscard]]
  auto getIfScope() const -> std::optional<Scope>;
  void exitCtxScope();
  void exitSymtabScope();

  void setCurCtxSymbol(sym::ValuePtr val);

  [[nodiscard]]
  auto getCurFuncName() const -> std::string;
  [[nodiscard]]
  auto getCurFuncType() const -> const type::TypePtr&;
  [[nodiscard]]
  auto getCurScopeName() const -> std::string;
  [[nodiscard]]
  auto getCurCtxName() const -> std::string;

private:
  void enterScope(Scope::Kind kind);

private:
  sym::SymbolTable  &symtab;

  std::unique_ptr<type::TypeFactory> type_factory;
  std::unique_ptr<ir::TempFactory>   temp_factory;

  // 当前函数上下文
  sym::FunctionPtr curfunc;
  int scopenum = 0; // 函数内部子作用域个数

  // 作用域上下文
  std::vector<Scope> scopestack;
};

} // namespace sem
