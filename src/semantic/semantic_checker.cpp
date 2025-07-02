#include <cassert>

#include "err_type.hpp"
#include "semantic_checker.hpp"

namespace sem {

void
SemanticChecker::check(const ast::FuncDecl &func)
{
  //TODO 检查返回值
}

void
SemanticChecker::check(const ast::FuncHeaderDecl &fhdecl)
{
  auto func = std::make_shared<sym::Function>();

  func->name = fhdecl.name;
  func->pos  = fhdecl.pos;

  stable.enterScope(func->name);

  // 声明形参
  for (const auto &arg : fhdecl.argv) {
    auto fparam = std::make_shared<sym::Variable>(arg->type);
    fparam->name   = arg->name;
    fparam->pos    = arg->pos;
    fparam->mut    = arg->mut;
    fparam->formal = true;
    stable.declareVar(fparam->name, fparam);

    (func->argv).push_back(fparam);
  }

  // 设置返回值类型
  type::TypePtr retval_type = std::make_shared<type::VoidType>();
  if (fhdecl.retval_type.has_value()) {
    retval_type = fhdecl.retval_type.value();
  }

  func->retval_type = retval_type;
  stable.declareFunc(func->name, func);
}

void
SemanticChecker::check(const ast::BlockStmt &bstmt)
{

}

void
SemanticChecker::check(const ast::FuncExprBlockStmt &febstmt)
{

}

void
SemanticChecker::check(const ast::RetStmt &rstmt)
{

}

void
SemanticChecker::check(const ast::VarDeclStmt &vdstmt)
{
  // 需要实现的产生式中，所有变量声明语句声明的变量只有两种情况
  // 1. let mut a : i32;
  // 2. let mut a;
  // 因此，我们这里简单的记录变量
  // 需要注意的是，由于存在自动类型推导，如果没有指定类型，理论上只能声明一个
  // Variable 符号，而不能直接声明一个 Variable 的子类
  // 简单起见，这里将所有变量声明为一个 i32 的变量

  // sym::VariablePtr p_var;
  // if (has_type) {
  //   // 1: let mut a : i32; 明确类型，直接构造变量
  //   p_var = std::make_shared<sym::Integer>(name, false, std::nullopt);
  // } else {
  //   // 2: let mut a; 自动类型推导 —— 类型未知
  //   p_var = std::make_shared<sym::Variable>(name, false, attr::VarType::UNKNOWN);
  // }
  // p_var->initialized = false;
  // p_var->setPos(pos);
  // stable.declareVar(name, p_var);
}

void
SemanticChecker::check(const ast::AssignStmt &astmt)
{
}

void
SemanticChecker::check(const ast::Variable &var)
{
}

void
SemanticChecker::check(const ast::ArrayAccess &aacc)
{
}

void
SemanticChecker::check(const ast::TupleAccess &aacc)
{
}

void
SemanticChecker::check(const ast::CmpExpr &cexpr)
{
}

void
SemanticChecker::check(const ast::AriExpr &aexpr)
{
}

void
SemanticChecker::check(const ast::ArrayElems &aelems)
{
}

void
SemanticChecker::check(const ast::TupleElems &telems)
{
}

void
SemanticChecker::check(const ast::BracketExpr &bexpr)
{
}

void
SemanticChecker::check(const ast::Factor &factor)
{
}

void
SemanticChecker::check(const ast::Number &num)
{
}

void
SemanticChecker::check(const ast::CallExpr &cexpr)
{
}

void
SemanticChecker::check(const ast::IfStmt &istmt)
{
}

void
SemanticChecker::check(const ast::ElseClause &eclause)
{
}

void
SemanticChecker::check(const ast::WhileStmt &wstmt)
{
}

void
SemanticChecker::check(const ast::ForStmt &fstmt)
{
}

void
SemanticChecker::check(const ast::LoopStmt &lstmt)
{
}

void
SemanticChecker::check(const ast::BreakStmt &bstmt)
{
}

void
SemanticChecker::check(const ast::IfExpr &iexpr)
{
}

} // namespace sem
