#pragma once

#include "ast.hpp"
#include "oop_visitor.hpp"

namespace sem {

class ReturnChecker : public ast::BaseVisitor {
public:
  ~ReturnChecker() override = default;

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
  void visit(ast::WhileLoopExpr&wlexpr) override;
  void visit(ast::ForLoopExpr &flexpr) override;
  void visit(ast::LoopExpr&lexpr) override;

public:
  bool has_ret = false;
};

} // namespace sem
