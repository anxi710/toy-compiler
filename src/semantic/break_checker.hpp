#pragma once

#include "ast.hpp"
#include "err_report.hpp"
#include "oop_visitor.hpp"
#include "type_factory.hpp"
#include "semantic_context.hpp"

namespace sem {

// 检查 statement block expression 中是否含有 break 语句
// 并检查各 break 语句是否具有相同的返回值类型
// 存储检查结果以便外部直接访问
class BreakChecker : public ast::BaseVisitor {
public:
  BreakChecker(const sem::SemanticContext &ctx,
    err::ErrReporter &reporter) : ctx(ctx), reporter(reporter) {}
  ~BreakChecker() override = default;

public:
  void visit(ast::StmtBlockExpr &sbexpr) override;
  void visit(ast::VarDeclStmt &vdstmt) override;
  void visit(ast::ExprStmt &estmt) override;
  void visit(ast::RetExpr &rexpr) override;
  void visit(ast::BreakExpr &bexpr) override;
  void visit(ast::AssignExpr &aexpr) override;
  void visit(ast::BracketExpr &bexpr) override;
  void visit(ast::IfExpr &iexpr) override;
  void visit(ast::ElseClause &eclause) override;

public:
  const sem::SemanticContext &ctx;      // semantic context
  err::ErrReporter           &reporter; // error reporter

  bool has_break = false; // 是否含有 break expression
  // 存储 break 表达式携带的返回值类型
  type::TypePtr type = type::TypeFactory::UNKNOWN_TYPE;
};

} // namespace sem
