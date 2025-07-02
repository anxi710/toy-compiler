#pragma once

#include <cstdint>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>

#include "ast.hpp"
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
  bool mut{true}; // 变量本身是否可变
  bool formal{false};
  bool initialized{false}; // 是否已经初始化

  type::TypePtr type; // 变量类型

  Variable() = default;
  Variable(type::TypePtr type) : type(std::move(type)) {}
  ~Variable() override = default;
};
using VariablePtr = std::shared_ptr<Variable>;

struct Function : Symbol {
  std::vector<VariablePtr> argv;
  type::TypePtr              retval_type;

  ~Function() override = default;
};
using FunctionPtr = std::shared_ptr<Function>;

class SymbolTable {
public:
  SymbolTable()
  {
    p_cscope = std::make_shared<Scope>();
    cscope_name = "global";
    scopes[cscope_name] = p_cscope;
  }
  ~SymbolTable() = default;

public:
  void enterScope(const std::string &name, bool create_scope = true);
  auto exitScope() -> std::string;

  void declareFunc(const std::string &fname, FunctionPtr p_func);
  void declareVar(const std::string &vname, VariablePtr p_var);

  [[nodiscard]]
  auto lookupFunc(const std::string &name) const -> std::optional<FunctionPtr>;
  [[nodiscard]]
  auto lookupVar(const std::string &name) const -> std::optional<VariablePtr>;

  void printSymbol(std::ofstream &out);

  auto getCurScope() const -> const std::string &;
  auto getTempValName() -> std::string;
  auto getFuncName() -> std::string;

  auto checkAutoTypeInfer() const -> std::vector<VariablePtr>;

private:
  using Scope = std::unordered_map<std::string, VariablePtr>;
  using ScopePtr = std::shared_ptr<Scope>;

  ScopePtr p_cscope;       // pointer (current scope)
  std::string cscope_name; // 作用域限定符

  std::stack<std::pair<int, int>> s_cnt; // (if_cnt, while_cnt)

  int tv_cnt; // temp value counter

  std::unordered_map<std::string, ScopePtr> scopes;
  std::unordered_map<std::string, FunctionPtr> funcs;
};

} // namespace symbol
