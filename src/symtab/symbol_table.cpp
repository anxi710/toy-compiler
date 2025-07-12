#include <print>
#include <regex>
#include <cassert>
#include <fstream>

#include "panic.hpp"
#include "symbol_table.hpp"

namespace sym {

/**
 * @brief 进入作用域
 * @param name   作用域限定符
 * @param create 是否新建作用域
 */
void
SymbolTable::enterScope(const std::string &name, bool create)
{
  std::string nextname = std::format("{}::{}", curname, name);

  if (!create) {
    assert(scopes.contains(nextname));
    curname  = nextname;
    curscope = scopes[curname];
    return;
  }

  assert(!scopes.contains(nextname));

  curname  = nextname;
  curscope = std::make_shared<Scope>();
  scopes[curname] = curscope;
}

/**
 * @brief  退出作用域
 */
void
SymbolTable::exitScope()
{
  std::size_t idx = curname.find_last_of(':');
  assert(idx < curname.length());

  std::string name = curname.substr(idx + 1);
  curname = curname.substr(0, idx - 1);
  assert(scopes.contains(curname));

  curscope = scopes[curname];
}

/**
 * @brief 声明函数
 * @param fname  函数名
 * @param func 函数符号指针
 */
void
SymbolTable::declareFunc(const std::string &fname, FunctionPtr func)
{
  if (funcs.contains(fname)) {
    util::runtime_error("function name already exists");
  }

  funcs[fname] = std::move(func);
}

/**
 * @brief 声明变量
 * @param vname 变量名
 * @param p_var 变量符号指针
 */
void
SymbolTable::declareVar(const std::string &vname, VariablePtr p_var)
{
  (*curscope)[vname] = std::move(p_var);
}

/**
 * @brief  查找函数符号
 * @param  name 函数名
 * @return std::optional<FunctionPtr> 需要检查是否能查到
 */
std::optional<FunctionPtr>
SymbolTable::lookupFunc(const std::string &name) const
{
  if (funcs.contains(name)) {
    return funcs.find(name)->second;
  }
  return std::nullopt;
}

/**
 * @brief  查找变量符号
 * @param  name 变量名
 * @return std::optional<VariablePtr> 需要检查是否能查到
 */
std::optional<VariablePtr>
SymbolTable::lookupVar(const std::string &name) const
{
  auto exit_scope = [](std::string &scope_name) -> bool {
    std::size_t idx = scope_name.find_last_of(':');
    if (idx >= scope_name.length()) {
      return false;
    }
    scope_name = scope_name.substr(0, idx - 1);
    return true;
  };

  auto scope_name = curname;

  bool flag = false;
  VariablePtr p_var;
  do {
    auto p_scope = scopes.find(scope_name)->second;
    if (p_scope->contains(name)) {
      flag = true;
      p_var = p_scope->find(name)->second;
      break;
    }
  } while (exit_scope(scope_name));

  return flag ? std::optional<VariablePtr>{p_var} : std::nullopt;
}

/**
 * @brief  取作用域名
 * @return 作用域名，含global
 */
std::string
SymbolTable::getCurScopeName() const
{
  return curname;
}

/**
 * @brief  取函数名
 * @return 函数名，不含global
 */
std::string
SymbolTable::getFuncName()
{
  std::regex re{R"(^global::(\w+))"};
  std::smatch match;
  std::regex_search(curname, match, re);
  return match[1];
}

/**
 * @brief 检查作用域下变量是否有类型
 * @return 未定义类型变量
 */
std::vector<VariablePtr>
SymbolTable::checkAutoTypeInfer() const
{
  std::vector<VariablePtr> failed_vars;

  for (const auto &[name, var] : *curscope) {
    if (var->type->kind == type::TypeKind::UNKNOWN) {
      failed_vars.push_back(var);
    }
  }

  return failed_vars;
}

/**
 * @brief 打印符号表
 * @param out 输出流
 */
void
SymbolTable::printSymbol(std::ofstream &out)
{
  std::println(out, "搜集到如下函数符号：");

  for (const auto &func : funcs) {
    std::println(out,
      "函数名：{}，参数个数：{}，返回值类型：",
      func.first, func.second->argv.size(), func.second->type->str()
    );
  }

  std::println(out, "搜集到如下变量符号：");

  for (const auto &scope : scopes) {
    auto scope_name = scope.first;
    auto p_scope = scope.second;

    for (const auto &var : *p_scope) {
      std::println(out,
        "变量名：{}::{}，类型：{}",
        scope_name, var.first, var.second->type->str()
      );
    }
  }
}

} // namespace sym
