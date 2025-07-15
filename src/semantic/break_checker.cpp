#include "break_checker.hpp"

namespace sem {

// NOTE: 对于非 break expression 根据实际情况分发
//       对于 break expression 执行相应的检查逻辑

void
BreakChecker::visit(ast::StmtBlockExpr &sbexpr)
{
  // 检查各语句是否含有 break expression
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
    // break Expr
    if (type == type::TypeFactory::UNKNOWN_TYPE) {
      // 第一次检查到 break 表达式时，设置期望类型
      if (bexpr.value.value()->type.type == type::TypeFactory::ANY_TYPE) {
        // 不能让 ANY_TYPE 通过类型推断暴露出去！
        type = type::TypeFactory::UNIT_TYPE;
      } else {
        type = bexpr.value.value()->type.type;
      }
    } else if (type != bexpr.value.value()->type.type) {
      // 检查是否和期望类型一致
      reporter.report(
        err::SemErrType::BREAK_TYPE_MISMATCH,
        std::format(
          "break 期望的类型为 {}，但实际类型为 {}",
          type->str(),
          bexpr.value.value()->type.str()
        ),
        bexpr.pos,
        ctx.getCurScopeName()
      );
    }
  } else {
    // break
    if (type == type::TypeFactory::UNKNOWN_TYPE) {
      // 第一次检查到 break 表达式时，设置期望类型
      type = type::TypeFactory::UNIT_TYPE;
    } else if (type != type::TypeFactory::UNIT_TYPE) {
      // 检查是否和期望类型一致
      reporter.report(
        err::SemErrType::BREAK_TYPE_MISMATCH,
        std::format(
          "break 期望的类型为 {}，但实际类型为 ()",
          type->str()
        ),
        bexpr.pos,
        ctx.getCurScopeName()
      );
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
