#include "semantic_ir_builder.hpp"

namespace par {

//BUG: 四元式作为综合属性挂在 AST 结点上
//     因此所有结点都需要将子节点生成的四元式组合起来

void
SemanticIRBuilder::build(ast::Prog &prog)
{
  //TODO: 拼接子结点生成的四元式
  //      构造程序的完整四元式序列
}

void
SemanticIRBuilder::build(ast::FuncDecl &fdecl)
{
  //TODO: 检查返回语句是否实现了路径覆盖
  //      检查语句块表达式的类型是否和返回类型一致
  sema.visit(fdecl);
}

void
SemanticIRBuilder::build(ast::FuncHeaderDecl &fhdecl)
{
  sema.visit(fhdecl);
}

void
SemanticIRBuilder::build(ast::Arg &arg)
{
  sema.visit(arg);
}

void
SemanticIRBuilder::build(ast::StmtBlockExpr &sbexpr)
{
  //TODO: 设置语句块表达式类型
  //      检查特殊语句
  sema.visit(sbexpr);
}

void
SemanticIRBuilder::build(ast::EmptyStmt &estmt)
{
  sema.visit(estmt);
}

void
SemanticIRBuilder::build(ast::VarDeclStmt &vdstmt)
{
  sema.visit(vdstmt);
  // 是否要生成中间代码取决于是否要赋值！
}

void
SemanticIRBuilder::build(ast::ExprStmt &estmt)
{
  sema.visit(estmt);
}

void
SemanticIRBuilder::build(ast::RetExpr &rexpr)
{
  sema.visit(rexpr);
}

void
SemanticIRBuilder::build(ast::BreakExpr &bexpr)
{
  sema.visit(bexpr);
}

void
SemanticIRBuilder::build(ast::ContinueExpr &cexpr)
{
  sema.visit(cexpr);
}

void
SemanticIRBuilder::build(ast::AssignElem &aelem)
{
  sema.visit(aelem);
}

void
SemanticIRBuilder::build(ast::Variable &var)
{
  sema.visit(var);
}

void
SemanticIRBuilder::build(ast::ArrayAccess &aacc)
{
  sema.visit(aacc);
}

void
SemanticIRBuilder::build(ast::TupleAccess &tacc)
{
  sema.visit(tacc);
}

void
SemanticIRBuilder::build(ast::AssignExpr &aexpr)
{
  sema.visit(aexpr);
}

void
SemanticIRBuilder::build(ast::CmpExpr &cexpr)
{
  sema.visit(cexpr);
}

void
SemanticIRBuilder::build(ast::AriExpr &aexpr)
{
  sema.visit(aexpr);
}

void
SemanticIRBuilder::build(ast::ArrayElems &aelems)
{
  sema.visit(aelems);
}

void
SemanticIRBuilder::build(ast::TupleElems &telems)
{
  sema.visit(telems);
}

void
SemanticIRBuilder::build(ast::BracketExpr &bexpr)
{
  sema.visit(bexpr);
}

void
SemanticIRBuilder::build(ast::Number &num)
{
  sema.visit(num);
}

void
SemanticIRBuilder::build(ast::CallExpr &cexpr)
{
  sema.visit(cexpr);
}

void
SemanticIRBuilder::build(ast::IfExpr &iexpr)
{
  sema.visit(iexpr);
}

void
SemanticIRBuilder::build(ast::ElseClause &eclause)
{
  sema.visit(eclause);
}

void
SemanticIRBuilder::build(ast::WhileLoopExpr &wlexpr)
{
  sema.visit(wlexpr);
}

void
SemanticIRBuilder::build(ast::ForLoopExpr &flexpr)
{
  sema.visit(flexpr);
}

void
SemanticIRBuilder::build(ast::Interval &interval)
{
  sema.visit(interval);
}

void
SemanticIRBuilder::build(ast::IterableVal &iter)
{
  sema.visit(iter);
}

void
SemanticIRBuilder::build(ast::LoopExpr &lexpr)
{
  sema.visit(lexpr);
}

} // namespace par
