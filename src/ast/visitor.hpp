#pragma once

#include "ast.hpp"

namespace ast {

class NodeVisitor {
public:
  virtual ~NodeVisitor() = default;

public:
  virtual void visit(Prog &prog) = 0;
  virtual void visit(Type &type) = 0;
  virtual void visit(Arg &arg) = 0;
  virtual void visit(StmtBlockExpr &sbexpr) = 0;
  virtual void visit(FuncHeaderDecl &fhdecl) = 0;
  virtual void visit(FuncDecl &fdecl) = 0;
  virtual void visit(ExprStmt &estmt) = 0;
  virtual void visit(EmptyExpr &eexpr) = 0;
  virtual void visit(BracketExpr &bexpr) = 0;
  virtual void visit(AssignElem &aelem) = 0;
  virtual void visit(Variable &var) = 0;
  virtual void visit(ArrAcc &aacc) = 0;
  virtual void visit(TupAcc &tacc) = 0;
  virtual void visit(Number &num) = 0;
  virtual void visit(ArrElems &arrelems) = 0;
  virtual void visit(TupElems &tupelems) = 0;
  virtual void visit(RetExpr &rstmt) = 0;
  virtual void visit(VarDeclStmt &vdstmt) = 0;
  virtual void visit(AssignExpr &aexpr) = 0;
  virtual void visit(CmpExpr &cexpr) = 0;
  virtual void visit(AriExpr &aexpr) = 0;
  virtual void visit(CallExpr &cexpr) = 0;
  virtual void visit(ElseClause &eclause) = 0;
  virtual void visit(IfExpr &iexpr) = 0;
  virtual void visit(WhileLoopExpr&wlexpr) = 0;
  virtual void visit(Interval &interval) = 0;
  virtual void visit(IterableVal &iter) = 0;
  virtual void visit(ForLoopExpr &flexpr) = 0;
  virtual void visit(LoopExpr&lexpr) = 0;
  virtual void visit(BreakExpr &bexpr) = 0;
  virtual void visit(ContinueExpr &cexpr) = 0;
  virtual void visit(EmptyStmt &estmt) = 0;
};

// 继承自 NodeVisitor 并实现了所有虚函数
// 其余 Visitor 继承该类，而非纯虚类 NodeVisitor
// 可以按需实现虚函数，而非所有虚函数都实现
class BaseVisitor : public NodeVisitor {
public:
  ~BaseVisitor() override = default;

public:
  void visit(Prog &prog) override {};
  void visit(Type &type) override {};
  void visit(Arg &arg) override {};
  void visit(StmtBlockExpr &sbexpr) override {};
  void visit(FuncHeaderDecl &fhdecl) override {};
  void visit(FuncDecl &fdecl) override {};
  void visit(ExprStmt &estmt) override {};
  void visit(EmptyExpr &eexpr) override {};
  void visit(BracketExpr &bexpr) override {};
  void visit(AssignElem &aelem) override {};
  void visit(Variable &var) override {};
  void visit(ArrAcc &aacc) override {};
  void visit(TupAcc &tacc) override {};
  void visit(Number &num) override {};
  void visit(ArrElems &arrelems) override {};
  void visit(TupElems &tupelems) override {};
  void visit(RetExpr &rstmt) override {};
  void visit(VarDeclStmt &vdstmt) override {};
  void visit(AssignExpr &aexpr) override {};
  void visit(CmpExpr &cexpr) override {};
  void visit(AriExpr &aexpr) override {};
  void visit(CallExpr &cexpr) override {};
  void visit(ElseClause &eclause) override {};
  void visit(IfExpr &iexpr) override {};
  void visit(WhileLoopExpr&wlexpr) override {};
  void visit(Interval &interval) override {};
  void visit(IterableVal &iter) override {};
  void visit(ForLoopExpr &flexpr) override {};
  void visit(LoopExpr&lexpr) override {};
  void visit(BreakExpr &bexpr) override {};
  void visit(ContinueExpr &cexpr) override {};
  void visit(EmptyStmt &estmt) override {};
};

} // namespace ast
