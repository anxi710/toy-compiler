#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <optional>

#include "temp_factory.hpp"
#include "type_factory.hpp"

namespace sym {

struct Function;
using FunctionPtr = std::shared_ptr<Function>;

struct Value;
using ValuePtr = std::shared_ptr<Value>;

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
  SemanticContext(sym::SymbolTable &symtab) : symtab(symtab) {}

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

  auto lookupFunc(const std::string &name) const -> std::optional<sym::FunctionPtr>;
  auto lookupVal(const std::string &name) const -> std::optional<sym::ValuePtr>;
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
  auto checkAutoTypeInfer() const -> std::vector<sym::ValuePtr>;

  bool inLoopCtx();
  auto getLoopCtx() -> std::optional<Scope*>;

  auto getCurScope() const -> Scope;
  auto getIfScope() const -> std::optional<Scope>;
  void exitCtxScope();
  void exitSymtabScope();

  void setCurCtxSymbol(sym::ValuePtr val);

  auto getCurFuncName() const -> std::string;
  auto getCurFuncType() const -> const type::TypePtr&;

  auto getCurScopeName() const -> std::string;

  auto getCurCtxName() const -> std::string;

private:
  void enterScope(Scope::Kind kind);

private:
  sym::SymbolTable  &symtab;
  type::TypeFactory  type_factory;
  ir::TempFactory    temp_factory;

  // 当前函数上下文
  sym::FunctionPtr curfunc;
  int scopenum = 0; // 函数内部子作用域个数

  // 作用域上下文
  std::vector<Scope> scopestack;
};

} // namespace sem
