#include "return_checker.hpp"

namespace sem {

void
ReturnChecker::visit(ast::StmtBlockExpr &sbexpr)
{
  for (const auto &stmt : sbexpr.stmts) {
    if (!has_ret) {
      stmt->accept(*this);
    }
    // TODO: 可以选择报告一个 warning
    stmt->unreachable = has_ret;
  }
}

void
ReturnChecker::visit(ast::VarDeclStmt &vdstmt)
{
  if (vdstmt.value.has_value()) {
    vdstmt.value.value()->accept(*this);
  }
}

void
ReturnChecker::visit(ast::ExprStmt &estmt)
{
  estmt.expr->accept(*this);
}

void
ReturnChecker::visit(ast::RetExpr &rexpr)
{
  has_ret = true;
}

void
ReturnChecker::visit(ast::BreakExpr &bexpr)
{
  if (bexpr.value.has_value()) {
    bexpr.value.value()->accept(*this);
  }
}

void
ReturnChecker::visit(ast::AssignExpr &aexpr)
{
  aexpr.rval->accept(*this);
}

void
ReturnChecker::visit(ast::BracketExpr &bexpr)
{
  if (bexpr.expr.has_value()) {
    bexpr.expr.value()->accept(*this);
  }
}

void
ReturnChecker::visit(ast::IfExpr &iexpr)
{
  if (iexpr.elses.empty() || iexpr.elses.back()->cond.has_value()) {
    has_ret = false;
    return;
  }

  // 路径覆盖
  ReturnChecker rchecker;
  iexpr.body->accept(rchecker);
  has_ret = rchecker.has_ret;
  for (const auto &eclause : iexpr.elses) {
    rchecker.has_ret = false;
    eclause->accept(rchecker);
    has_ret &= rchecker.has_ret;
  }
}

void
ReturnChecker::visit(ast::ElseClause &eclause)
{
  eclause.body->accept(*this);
}

void
ReturnChecker::visit(ast::WhileLoopExpr &wlexpr)
{
  // 认为 while 表达式可能不会执行
  has_ret = false;
}

void
ReturnChecker::visit(ast::ForLoopExpr &flexpr)
{
  // 认为 for 表达式可能不会执行
  has_ret = false;
}

void
ReturnChecker::visit(ast::LoopExpr &lexpr)
{
  // 认为 loop 表达式一定会执行
  lexpr.body->accept(*this);
}

} // namespace sem
