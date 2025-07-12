#pragma once

#include "ast.hpp"
#include "visitor.hpp"
#include "type_factory.hpp"

namespace sem {

class BreakChecker : public ast::BaseVisitor {
public:
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
  bool has_break = false;
  type::TypePtr type = type::TypeFactory::UNKNOWN_TYPE;
};

} // namespace sem
