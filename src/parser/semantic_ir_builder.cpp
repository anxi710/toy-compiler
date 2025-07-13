#include "semantic_ir_builder.hpp"

namespace par {

// NOTE: 四元式作为综合属性挂在 AST 结点上
//       因此所有结点都需要将子节点生成的四元式组合起来

void
SemanticIRBuilder::build(ast::Prog &prog)
{
  if (!reporter.hasErrs()) {
    ir.visit(prog);
  }
}

void
SemanticIRBuilder::build(ast::FuncDecl &fdecl)
{
  sema.visit(fdecl);
  if (!reporter.hasErrs()) {
    ir.visit(fdecl);
  }
}

void
SemanticIRBuilder::build(ast::FuncHeaderDecl &fhdecl)
{
  sema.visit(fhdecl);
  if (!reporter.hasErrs()) {
    ir.visit(fhdecl);
  }
}

void
SemanticIRBuilder::build(ast::Arg &arg)
{
  sema.visit(arg);
  if (!reporter.hasErrs()) {
    ir.visit(arg);
  }
}

void
SemanticIRBuilder::build(ast::StmtBlockExpr &sbexpr)
{
  sema.visit(sbexpr);
  if (!reporter.hasErrs()) {
    ir.visit(sbexpr);
  }
}

void
SemanticIRBuilder::build(ast::EmptyStmt &estmt)
{
  sema.visit(estmt);
  if (!reporter.hasErrs()) {
    ir.visit(estmt);
  }
}

void
SemanticIRBuilder::build(ast::VarDeclStmt &vdstmt)
{
  sema.visit(vdstmt);
  // 是否要生成中间代码取决于是否要赋值！
  if (!reporter.hasErrs()) {
    ir.visit(vdstmt);
  }
}

void
SemanticIRBuilder::build(ast::ExprStmt &estmt)
{
  sema.visit(estmt);
  if (!reporter.hasErrs()) {
    ir.visit(estmt);
  }
}

void
SemanticIRBuilder::build(ast::RetExpr &rexpr)
{
  sema.visit(rexpr);
  if (!reporter.hasErrs()) {
    ir.visit(rexpr);
  }
}

void
SemanticIRBuilder::build(ast::BreakExpr &bexpr)
{
  sema.visit(bexpr);
  if (!reporter.hasErrs()) {
    ir.visit(bexpr);
  }
}

void
SemanticIRBuilder::build(ast::ContinueExpr &cexpr)
{
  sema.visit(cexpr);
  if (!reporter.hasErrs()) {
    ir.visit(cexpr);
  }
}

void
SemanticIRBuilder::build(ast::AssignElem &aelem)
{
  sema.visit(aelem);
  if (!reporter.hasErrs()) {
    ir.visit(aelem);
  }
}

void
SemanticIRBuilder::build(ast::Variable &var)
{
  sema.visit(var);
  if (!reporter.hasErrs()) {
    ir.visit(var);
  }
}

void
SemanticIRBuilder::build(ast::ArrayAccess &aacc)
{
  sema.visit(aacc);
  if (!reporter.hasErrs()) {
    ir.visit(aacc);
  }
}

void
SemanticIRBuilder::build(ast::TupleAccess &tacc)
{
  sema.visit(tacc);
  if (!reporter.hasErrs()) {
    ir.visit(tacc);
  }
}

void
SemanticIRBuilder::build(ast::AssignExpr &aexpr)
{
  sema.visit(aexpr);
  if (!reporter.hasErrs()) {
    ir.visit(aexpr);
  }
}

void
SemanticIRBuilder::build(ast::CmpExpr &cexpr)
{
  sema.visit(cexpr);
  if (!reporter.hasErrs()) {
    ir.visit(cexpr);
  }
}

void
SemanticIRBuilder::build(ast::AriExpr &aexpr)
{
  sema.visit(aexpr);
  if (!reporter.hasErrs()) {
    ir.visit(aexpr);
  }
}

void
SemanticIRBuilder::build(ast::ArrayElems &aelems)
{
  sema.visit(aelems);
  if (!reporter.hasErrs()) {
    ir.visit(aelems);
  }
}

void
SemanticIRBuilder::build(ast::TupleElems &telems)
{
  sema.visit(telems);
  if (!reporter.hasErrs()) {
    ir.visit(telems);
  }
}

void
SemanticIRBuilder::build(ast::BracketExpr &bexpr)
{
  sema.visit(bexpr);
  if (!reporter.hasErrs()) {
    ir.visit(bexpr);
  }
}

void
SemanticIRBuilder::build(ast::Number &num)
{
  sema.visit(num);
  if (!reporter.hasErrs()) {
    ir.visit(num);
  }
}

void
SemanticIRBuilder::build(ast::CallExpr &cexpr)
{
  sema.visit(cexpr);
  if (!reporter.hasErrs()) {
    ir.visit(cexpr);
  }
}

void
SemanticIRBuilder::build(ast::IfExpr &iexpr)
{
  sema.visit(iexpr);
  if (!reporter.hasErrs()) {
    ir.visit(iexpr);
  }
}

void
SemanticIRBuilder::build(ast::ElseClause &eclause)
{
  sema.visit(eclause);
  if (!reporter.hasErrs()) {
    ir.visit(eclause);
  }
}

void
SemanticIRBuilder::build(ast::WhileLoopExpr &wlexpr)
{
  sema.visit(wlexpr);
  if (!reporter.hasErrs()) {
    ir.visit(wlexpr);
  }
}

void
SemanticIRBuilder::build(ast::ForLoopExpr &flexpr)
{
  sema.visit(flexpr);
  if (!reporter.hasErrs()) {
    ir.visit(flexpr);
  }
}

void
SemanticIRBuilder::build(ast::Interval &interval)
{
  sema.visit(interval);
  if (!reporter.hasErrs()) {
    ir.visit(interval);
  }
}

void
SemanticIRBuilder::build(ast::IterableVal &iter)
{
  sema.visit(iter);
  if (!reporter.hasErrs()) {
    ir.visit(iter);
  }
}

void
SemanticIRBuilder::build(ast::LoopExpr &lexpr)
{
  sema.visit(lexpr);
  if (!reporter.hasErrs()) {
    ir.visit(lexpr);
  }
}

} // namespace par
