#include <print>
#include <regex>
#include <fstream>
#include <generator>
#include <string_view>

#include "panic.hpp"
#include "type.hpp"
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
    ASSERT_MSG(
      scopes.contains(nextname),
      "scope didn't created"
    );
    curname  = nextname;
    curscope = scopes[curname];
    return;
  }

  ASSERT_MSG(
    !scopes.contains(nextname),
    "scope already exists"
  );

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
  std::size_t idx = curname.rfind("::");
  ASSERT_MSG(
    idx < curname.length(),
    "cann't exit scope"
  );

  std::string name = curname.substr(idx + 2);
  curname = curname.substr(0, idx);
  ASSERT_MSG(
    scopes.contains(curname),
    "scope doesn't exist"
  );

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
    UNREACHABLE("function name already exists");
  }

  funcs[fname] = std::move(func);
}

/**
 * @brief 声明变量
 * @param vname 变量名
 * @param p_var 变量符号指针
 */
void
SymbolTable::declareVal(const std::string &vname, ValuePtr val)
{
  // BUG: 如何支持重影还有待商榷！
  (*curscope)[vname] = std::move(val);
}

void
SymbolTable::declareConst(const std::string &cname, ConstantPtr con)
{
  if (constvals.contains(cname)) {
    UNREACHABLE("const value already exists");
  }

  constvals[cname] = std::move(con);
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

static std::generator<std::string_view>
reverseScopeRange(std::string_view scope_name)
{
  while (true) {
    co_yield scope_name;
    auto pos = scope_name.rfind("::");
    if (pos == std::string_view::npos) {
      break;
    }
    scope_name = scope_name.substr(0, pos);
  }
}

/**
 * @brief  查找变量符号
 * @param  name 变量名
 * @return std::optional<VariablePtr> 需要检查是否能查到
 */
std::optional<ValuePtr>
SymbolTable::lookupVal(const std::string &name) const
{
  for (auto scopename : reverseScopeRange(curname)) {
    if (auto it = scopes.find(scopename); it != scopes.end()) {
      auto scope = it->second;
      if (auto var_it = scope->find(name); var_it != scope->end()) {
        return var_it->second;
      }
    }
  }

  return std::nullopt;
}

std::optional<ConstantPtr>
SymbolTable::lookupConst(const std::string &name) const
{
  if (constvals.contains(name)) {
    return constvals.find(name)->second;
  }
  return std::nullopt;
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
std::vector<ValuePtr>
SymbolTable::checkAutoTypeInfer() const
{
  std::vector<ValuePtr> failed_vals;

  for (const auto &[name, val] : *curscope) {
    if (val->type->kind == type::TypeKind::UNKNOWN) {
      failed_vals.push_back(val);
    }
  }

  return failed_vals;
}


/**
 * @brief 打印符号表
 * @param out 输出流
 */
void
SymbolTable::dump(std::ofstream &out)
{
  std::println(out, "=============== Symbol Table ===============");

  dumpFunc(out);
  dumpLocalVar(out);
  dumpConstant(out);
}

void
SymbolTable::dumpFunc(std::ofstream &out)
{
  constexpr int delimiter_cnt = 44;

  std::println(out, "\nFunction:");
  std::println(out, "{}", std::string(delimiter_cnt, '-'));

  for (const auto &func : funcs) {
    std::println(out, "  function name: {}", func.first);
    std::println(out, "  argc: {}", func.second->argv.size());
    if (func.second->argv.size() > 0) {
      std::println(out, "  argv:");
      for (const auto &[idx, arg] : std::views::enumerate(func.second->argv)) {
        std::println(
          out,
          "    {}. name: {}, mutable: {}, type: {}",
          idx + 1,
          arg->name,
          arg->mut ? "true" : "false",
          arg->type->str()
        );
      }
    }
    std::println(out, "  return type: {}", func.second->type->str());
    std::println(out, "{}", std::string(delimiter_cnt, '-'));
  }
}

void
SymbolTable::dumpLocalVar(std::ofstream &out)
{
  std::println(out, "\nLocal Variable:");

  for (const auto &scope : scopes) {
    if (scope.first == "global") {
      continue;
    }

    std::string str = "global::";
    auto scopename = scope.first.substr(str.length());
    auto p_scope = scope.second;

    int cnt = 0;
    for (const auto &val : *p_scope) {
      if (val.second->kind == Value::Kind::LOCAL) {
        auto var = std::static_pointer_cast<Variable>(val.second);
        if (!var->formal) {
          std::println(out,
            " {:>2}. name: {}, mutable: {}, type: {}",
            ++cnt,
            std::format("{}::{}", scopename, var->name),
            var->mut ? "true" : "false",
            var->type->str()
          );
        }
      }
    }
  }
}

void
SymbolTable::dumpConstant(std::ofstream &out)
{
  std::println(out, "\nConstant:");

  for (const auto &[idx, constval] : std::views::enumerate(constvals)) {
    std::println(out, "{:>3}. {}", idx + 1, constval.first);
  }
}

} // namespace sym
