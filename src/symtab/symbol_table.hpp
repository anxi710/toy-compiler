#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "type.hpp"
#include "position.hpp"

namespace sym {

struct Symbol {
  std::string    name;
  util::Position pos{0, 0}; // 符号声明时的位置

  virtual ~Symbol() = default;
};
using SymbolPtr = std::shared_ptr<Symbol>;

struct Variable : Symbol {
  bool mut;    // 变量本身是否可变
  bool formal; // 是否是形参
  bool init;   // 是否已经初始化

  type::TypePtr type; // 变量类型

  ~Variable() override = default;
};
using VariablePtr = std::shared_ptr<Variable>;

struct Function : Symbol {
  std::vector<VariablePtr> argv;
  type::TypePtr            type; // return value type

  ~Function() override = default;
};
using FunctionPtr = std::shared_ptr<Function>;

class SymbolTable {
public:
  SymbolTable() {
    curname  = "global";
    curscope = std::make_shared<Scope>();
    scopes[curname] = curscope;
  }
  ~SymbolTable() = default;

public:
  void enterScope(const std::string &name, bool create);
  void exitScope();

  void declareFunc(const std::string &fname, FunctionPtr p_func);
  void declareVar(const std::string &vname, VariablePtr p_var);

  auto lookupFunc(const std::string &name) const -> std::optional<FunctionPtr>;
  auto lookupVar(const std::string &name) const -> std::optional<VariablePtr>;

  void printSymbol(std::ofstream &out);

  auto getCurScopeName() const -> std::string;
  auto getFuncName() -> std::string;

  auto checkAutoTypeInfer() const -> std::vector<VariablePtr>;

private:
  using Scope    = std::unordered_map<std::string, VariablePtr>;
  using ScopePtr = std::shared_ptr<Scope>;

  ScopePtr    curscope; // current scope
  std::string curname;  // 作用域限定符

  std::unordered_map<std::string, ScopePtr>    scopes;
  std::unordered_map<std::string, FunctionPtr> funcs;
};

} // namespace symbol
