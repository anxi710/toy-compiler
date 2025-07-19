#include <algorithm>

#include "position.hpp"
#include "symbol_table.hpp"
#include "semantic_context.hpp"

namespace sem {

void
SemanticContext::enterFunc(const std::string &name, util::Position pos) {
  curfunc = std::make_shared<sym::Function>();
  // 注意：形参列表和返回类型此时还未设置！
  curfunc->pos = pos;
  curfunc->name = name;

  symtab.declareFunc(name, curfunc);
  symtab.enterScope(name, true);
  scopenum = 0;
  scopestack.emplace_back(Scope::Kind::FUNC, name);
}

// 统一的进入作用域函数，参数决定作用域类型
void
SemanticContext::enterScope(Scope::Kind kind)
{
  // 构造唯一作用域名称，计数器前缀 "L"
  ++scopenum;
  const std::string name = std::format("L{}", scopenum);
  // 通知符号表进入新作用域
  symtab.enterScope(name, true);
  // 记录作用域栈，方便后续上下文查询
  scopestack.emplace_back(kind, name);
}

void
SemanticContext::enterBlockExpr()
{
  enterScope(Scope::Kind::BLOCKEXPR);
}

void
SemanticContext::enterIf()
{
  enterScope(Scope::Kind::IF);
}

void
SemanticContext::enterElse()
{
  enterScope(Scope::Kind::ELSE);
}

void
SemanticContext::enterLoop()
{
  enterScope(Scope::Kind::LOOP);
}

void
SemanticContext::enterFor()
{
  enterScope(Scope::Kind::FOR);
}

void
SemanticContext::enterWhile()
{
  enterScope(Scope::Kind::WHILE);
}

void
SemanticContext::exitScope()
{
  symtab.exitScope();
  scopestack.pop_back();
}

std::optional<sym::FunctionPtr>
SemanticContext::lookupFunc(const std::string &name) const
{
  return symtab.lookupFunc(name);
}

std::optional<sym::ValuePtr>
SemanticContext::lookupVal(const std::string &name) const
{
   return symtab.lookupVal(name);
}

std::optional<sym::ConstantPtr>
SemanticContext::lookupConst(const std::string &name) const
{
  return symtab.lookupConst(name);
}

void
SemanticContext::declareArg(const std::string &name, bool mut,
  type::TypePtr type, util::Position pos)
{
  auto arg = std::make_shared<sym::Variable>();
  arg->pos    = pos;
  arg->name   = name;
  arg->mut    = mut;
  arg->formal = true;
  arg->init   = true;
  arg->type   = std::move(type);

  symtab.declareVal(name, arg);
  curfunc->argv.push_back(arg);
}

sym::VariablePtr
SemanticContext::declareVar(const std::string &name, bool mut, bool init,
  type::TypePtr type, util::Position pos)
{
  auto var = std::make_shared<sym::Variable>();
  var->pos    = pos;
  var->name   = name;
  var->mut    = mut;
  var->init   = init;
  var->formal = false;
  var->type   = std::move(type);

  symtab.declareVal(name, var);
  return var;
}

[[nodiscard]] sym::ConstantPtr
SemanticContext::declareConst(std::variant<int, bool> val, util::Position pos)
{
  std::string name;
  type::TypePtr type;
  if (std::holds_alternative<int>(val)) {
    name = std::to_string(std::get<int>(val));
    type = type::TypeFactory::INT_TYPE;
  } else if (std::holds_alternative<bool>(val)) {
    if (std::get<bool>(val)) {
      name = "true";
    } else {
      name = "false";
    }
    type = type::TypeFactory::BOOL_TYPE;
  }

  if (auto opt_const = lookupConst(name); opt_const.has_value()) {
    return opt_const.value();
  }

  auto constant = std::make_shared<sym::Constant>();
  constant->pos = pos;
  constant->mut = false;
  constant->init = true;
  constant->name = name;
  constant->type = type;

  symtab.declareConst(name, constant);

  return constant;
}

type::TypePtr
SemanticContext::produceArrType(int size, type::TypePtr etype)
{
  return type_factory.getArray(size, std::move(etype));
}

type::TypePtr
SemanticContext::produceTupType(std::vector<type::TypePtr> etypes)
{
  return type_factory.getTuple(std::move(etypes));
}

void
SemanticContext::resetTempCnt()
{
  temp_factory.reset();
}

sym::TempPtr
SemanticContext::produceTemp(util::Position pos, type::TypePtr type)
{
  return temp_factory.produce(pos, std::move(type));
}

void
SemanticContext::setRetValType(type::TypePtr type) {
  curfunc->type = std::move(type);
}

bool
SemanticContext::inLoopCtx()
{
  return std::ranges::any_of(
    std::views::reverse(scopestack),
    [](const auto &scope) {
      return scope.kind == Scope::Kind::LOOP
          || scope.kind == Scope::Kind::WHILE
          || scope.kind == Scope::Kind::FOR;
    }
  );
}

std::optional<SemanticContext::Scope*>
SemanticContext::getLoopCtx()
{
  for (auto &scope : std::views::reverse(scopestack)) {
    if (scope.kind == Scope::Kind::LOOP ||
        scope.kind == Scope::Kind::WHILE ||
        scope.kind == Scope::Kind::FOR) {
      return &scope;
    }
  }
  return std::nullopt;
}

SemanticContext::Scope
SemanticContext::getCurScope() const
{
  CHECK(!scopestack.empty(), "scope stack is empty");
  return scopestack.back();
}

std::optional<SemanticContext::Scope>
SemanticContext::getIfScope() const
{
  for (const auto &scope : std::views::reverse(scopestack)) {
    if (scope.kind == Scope::Kind::IF) {
      return scope;
    }
  }
  return std::nullopt;
}

void
SemanticContext::exitCtxScope()
{
  scopestack.pop_back();
}

void
SemanticContext::exitSymtabScope()
{
  symtab.exitScope();
}

void
SemanticContext::setCurCtxSymbol(sym::ValuePtr val)
{
  ASSERT_MSG(!scopestack.empty(), "context scope stack is empty!");
  scopestack.back().val = std::move(val);
}

std::vector<sym::ValuePtr>
SemanticContext::checkAutoTypeInfer() const
{
  return symtab.checkAutoTypeInfer();
}

std::string
SemanticContext::getCurFuncName() const
{
  return curfunc->name;
}

const type::TypePtr&
SemanticContext::getCurFuncType() const
{
  return curfunc->type;
}

std::string
SemanticContext::getCurScopeName() const
{
  return symtab.getCurScopeName().substr(
    std::string("global::").length()
  );
}

std::string
SemanticContext::getCurCtxName() const
{
  return scopestack.back().name;
}

} // namespace sem
