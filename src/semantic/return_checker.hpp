#pragma once

#include "ast.hpp"
#include "visitor.hpp"

namespace sem {

class ReturnChecker : public ast::BaseVisitor {
public:
  ~ReturnChecker() override = default;

public:
  void visit(ast::Prog &prog) override {};
  void visit(ast::FuncDecl &fdecl) override {};
  void visit(ast::FuncHeaderDecl &fhdecl) override {};
  void visit(ast::Arg &arg) override {};
  void visit(ast::StmtBlockExpr &sbexpr) override;
  void visit(ast::VarDeclStmt &vdstmt) override {};
  void visit(ast::ExprStmt &estmt) override;
  void visit(ast::RetExpr &rexpr) override;
  void visit(ast::BreakExpr &bexpr) override;
  void visit(ast::ContinueExpr &cexpr) override {};
  void visit(ast::AssignExpr &aexpr) override;
  void visit(ast::AssignElem &aelem) override {};
  void visit(ast::Variable &var) override {};
  void visit(ast::ArrayAccess &aacc) override {};
  void visit(ast::TupleAccess &tacc) override {};
  void visit(ast::CmpExpr &cexpr) override {};
  void visit(ast::AriExpr &aexpr) override {};
  void visit(ast::ArrayElems &aelems) override {};
  void visit(ast::TupleElems &telems) override {};
  void visit(ast::Number &num) override {};
  void visit(ast::BracketExpr &bexpr) override;
  void visit(ast::CallExpr &cexpr) override {};
  void visit(ast::IfExpr &iexpr) override;
  void visit(ast::ElseClause &eclause) override;
  void visit(ast::WhileLoopExpr&wlexpr) override;
  void visit(ast::ForLoopExpr &flexpr) override;
  void visit(ast::Interval &interval) override {};
  void visit(ast::IterableVal &iter) override {};
  void visit(ast::LoopExpr&lexpr) override;

public:
  bool has_ret = false;
};

} // namespace sem
