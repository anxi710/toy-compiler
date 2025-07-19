#pragma once

#include <string>
#include <unordered_map>

#include "symbol.hpp"

namespace sym {

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

  void declareFunc(const std::string &fname, FunctionPtr func);
  void declareVal(const std::string &vname, ValuePtr val);
  void declareConst(const std::string &cname, ConstantPtr con);

  auto lookupFunc(const std::string &name) const -> std::optional<FunctionPtr>;
  auto lookupVal(const std::string &name) const -> std::optional<ValuePtr>;
  auto lookupConst(const std::string &name) const -> std::optional<ConstantPtr>;

  auto getCurScopeName() const -> std::string;
  auto getFuncName() -> std::string;

  auto checkAutoTypeInfer() const -> std::vector<ValuePtr>;

  void dump(std::ofstream &out);

private:
  void dumpFunc(std::ofstream &out);
  void dumpLocalVar(std::ofstream &out);
  void dumpConstant(std::ofstream &out);

private:
  using Scope    = std::unordered_map<std::string, ValuePtr>;
  using ScopePtr = std::shared_ptr<Scope>;

  ScopePtr    curscope; // current scope
  std::string curname;  // 作用域限定符

  struct TransparentHash {
    using is_transparent = void;

    std::size_t operator()(std::string_view str) const noexcept {
      return std::hash<std::string_view>{}(str);
    }
    std::size_t operator()(const std::string &str) const noexcept {
      return std::hash<std::string_view>{}(str);
    }
  };

  std::unordered_map<
    std::string,
    ScopePtr,
    TransparentHash,
    std::equal_to<> // 透明比较器
  > scopes; // TempVal && LocalVal
  std::unordered_map<std::string, ConstantPtr> constvals;
  std::unordered_map<std::string, FunctionPtr> funcs;
};

} // namespace symbol
