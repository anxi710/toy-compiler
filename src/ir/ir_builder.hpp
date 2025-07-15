#pragma once

#include "visitor.hpp"
#include "semantic_context.hpp"

namespace ir {

// NOTE: IR 生成中使用静态表达式 SSA 形式
//
// Q: 什么是 SSA?
// A: SSA 全称 Static Single Assignment，即静态单赋值
//    其指的是每个变量在程序中只赋值一次，每次赋值都创建一个新的版本变量
//    SSA 的优势：
//      1. 易于做优化（常量传播、死代码消除、复制传播等）
//      2. 明确每个变量的定义和使用（Def-Use Chain）
//      3. 能精确定位活跃变量，方便做寄存器分配
//
// 在我的设计中，对于用户定义的变量，使用非 SSA 形式，即可以多次赋值；
// 而表达式的中间值则使用 SSA 形式，即只能赋值一次
//
// 总得来说就是，**只有中间表达式结果是静态单赋值的，而用户变量是多赋值形式**
//

class IRBuilder : public ast::BaseVisitor {
public:
  IRBuilder(sem::SemanticContext &ctx) : ctx(ctx) {}

public:
  void visit(ast::Prog &prog) override;
  void visit(ast::FuncDecl &fdecl) override;
  void visit(ast::FuncHeaderDecl &fhdecl) override;
  void visit(ast::Arg &arg) override;
  void visit(ast::StmtBlockExpr &sbexpr) override;
  void visit(ast::VarDeclStmt &vdstmt) override;
  void visit(ast::ExprStmt &estmt) override;
  void visit(ast::RetExpr &rexpr) override;
  void visit(ast::BreakExpr &bexpr) override;
  void visit(ast::ContinueExpr &cexpr) override;
  void visit(ast::AssignExpr &aexpr) override;
  void visit(ast::AssignElem &aelem) override;
  void visit(ast::Variable &var) override;
  void visit(ast::ArrAcc &aacc) override;
  void visit(ast::TupAcc &tacc) override;
  void visit(ast::CmpExpr &cexpr) override;
  void visit(ast::AriExpr &aexpr) override;
  void visit(ast::ArrElems &aelems) override;
  void visit(ast::TupElems &telems) override;
  void visit(ast::Number &num) override;
  void visit(ast::BracketExpr &bexpr) override;
  void visit(ast::CallExpr &cexpr) override;
  void visit(ast::IfExpr &iexpr) override;
  void visit(ast::ElseClause &eclause) override;
  void visit(ast::WhileLoopExpr&wlexpr) override;
  void visit(ast::ForLoopExpr &flexpr) override;
  void visit(ast::Interval &interval) override;
  void visit(ast::IterableVal &iter) override;
  void visit(ast::LoopExpr&lexpr) override;

private:
  sem::SemanticContext &ctx;

  int tmpcnt = 0;
};

} // namespace ir
