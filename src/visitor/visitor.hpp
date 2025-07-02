#pragma once

#include "ast.hpp"

namespace visit {

class NodeVisitor {
public:
  virtual ~NodeVisitor() = 0;

public:
  virtual void visit(ast::Prog &prog) = 0;
  virtual void visit(ast::Arg &arg) = 0;
  virtual void visit(ast::BlockStmt &bstmt) = 0;
  virtual void visit(ast::FuncHeaderDecl &fhdecl) = 0;
  virtual void visit(ast::FuncDecl &fdecl) = 0;
  virtual void visit(ast::ExprStmt &estmt) = 0;
  virtual void visit(ast::BracketExpr &bexpr) = 0;
  virtual void visit(ast::AssignElem &aelem) = 0;
  virtual void visit(ast::Variable &var) = 0;
  virtual void visit(ast::ArrayAccess &aacc) = 0;
  virtual void visit(ast::TupleAccess &tacc) = 0;
  virtual void visit(ast::Number &num) = 0;
  virtual void visit(ast::Factor &factor) = 0;
  virtual void visit(ast::ArrayElems &arrelems) = 0;
  virtual void visit(ast::TupleElems &tupelems) = 0;
  virtual void visit(ast::RetStmt &rstmt) = 0;
  virtual void visit(ast::VarDeclStmt &vdstmt) = 0;
  virtual void visit(ast::AssignStmt &astmt) = 0;
  virtual void visit(ast::CmpExpr &cexpr) = 0;
  virtual void visit(ast::AriExpr &aexpr) = 0;
  virtual void visit(ast::CallExpr &cexpr) = 0;
  virtual void visit(ast::ElseClause &eclause) = 0;
  virtual void visit(ast::IfStmt &istmt) = 0;
  virtual void visit(ast::WhileStmt &wstmt) = 0;
  virtual void visit(ast::ForStmt &fstmt) = 0;
  virtual void visit(ast::LoopStmt &lstmt) = 0;
  virtual void visit(ast::BreakStmt &bstmt) = 0;
  virtual void visit(ast::ContinueStmt &cstmt) = 0;
  virtual void visit(ast::NullStmt &nstmt) = 0;
  virtual void visit(ast::FuncExprBlockStmt &febstmt) = 0;
  virtual void visit(ast::IfExpr &iexpr) = 0;
};

class BaseVisitor : public NodeVisitor {
public:
  ~BaseVisitor() override = default;

public:
  void visit(ast::Prog &prog) override {};
  void visit(ast::Arg &arg) override {};
  void visit(ast::BlockStmt &bstmt) override {};
  void visit(ast::FuncHeaderDecl &fhdecl) override {};
  void visit(ast::FuncDecl &fdecl) override {};
  void visit(ast::ExprStmt &estmt) override {};
  void visit(ast::BracketExpr &bexpr) override {};
  void visit(ast::AssignElem &aelem) override {};
  void visit(ast::Variable &var) override {};
  void visit(ast::ArrayAccess &aacc) override {};
  void visit(ast::TupleAccess &tacc) override {};
  void visit(ast::Number &num) override {};
  void visit(ast::Factor &factor) override {};
  void visit(ast::ArrayElems &arrelems) override {};
  void visit(ast::TupleElems &tupelems) override {};
  void visit(ast::RetStmt &rstmt) override {};
  void visit(ast::VarDeclStmt &vdstmt) override {};
  void visit(ast::AssignStmt &astmt) override {};
  void visit(ast::CmpExpr &cexpr) override {};
  void visit(ast::AriExpr &aexpr) override {};
  void visit(ast::CallExpr &cexpr) override {};
  void visit(ast::ElseClause &eclause) override {};
  void visit(ast::IfStmt &istmt) override {};
  void visit(ast::WhileStmt &wstmt) override {};
  void visit(ast::ForStmt &fstmt) override {};
  void visit(ast::LoopStmt &lstmt) override {};
  void visit(ast::BreakStmt &bstmt) override {};
  void visit(ast::ContinueStmt &cstmt) override {};
  void visit(ast::NullStmt &nstmt) override {};
  void visit(ast::FuncExprBlockStmt &febstmt) override {};
  void visit(ast::IfExpr &iexpr) override {};
};

} // namespace ast
