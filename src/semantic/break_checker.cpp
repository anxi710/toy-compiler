#include <iostream>
#include "break_checker.hpp"

namespace sem {

void
BreakChecker::visit(ast::StmtBlockExpr &sbexpr)
{
  for (const auto &stmt : sbexpr.stmts) {
    stmt->accept(*this);
  }
}

void
BreakChecker::visit(ast::VarDeclStmt &vdstmt)
{
  if (vdstmt.value.has_value()) {
    vdstmt.value.value()->accept(*this);
  }
}

void
BreakChecker::visit(ast::ExprStmt &estmt)
{
  estmt.expr->accept(*this);
}

void
BreakChecker::visit(ast::RetExpr &rexpr)
{
  if (rexpr.retval.has_value()) {
    rexpr.retval.value()->accept(*this);
  }
}

void
BreakChecker::visit(ast::BreakExpr &bexpr)
{
  has_break = true;
  if (bexpr.value.has_value()) {
    if (type == type::TypeFactory::UNKNOWN_TYPE) {
      type = bexpr.value.value()->type.type;
    } else {
      //TODO: 各 break 返回的值的类型应该一致
      std::cerr << "各 break 返回的值的类型应该一致" << std::endl;
    }
  } else {
    if (type == type::TypeFactory::UNKNOWN_TYPE) {
      type = type::TypeFactory::UNIT_TYPE;
    } else {
      //TODO: 各 break 返回的值的类型应该一致
      std::cerr << "各 break 返回的值的类型应该一致" << std::endl;
    }
  }
}

void
BreakChecker::visit(ast::AssignExpr &aexpr)
{
  aexpr.rval->accept(*this);
}

void
BreakChecker::visit(ast::BracketExpr &bexpr)
{
  if (bexpr.expr.has_value()) {
    bexpr.expr.value()->accept(*this);
  }
}

void
BreakChecker::visit(ast::IfExpr &iexpr)
{
  iexpr.body->accept(*this);
  for (const auto &eclause : iexpr.elses) {
    eclause->accept(*this);
  }
}

void
BreakChecker::visit(ast::ElseClause &eclause)
{
  eclause.body->accept(*this);
}

} // namespace sem
