#pragma once

#include "crtp_visitor.hpp"
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

class IRBuilder : public ast::CRTPVisitor<IRBuilder> {
public:
  IRBuilder(sem::SemanticContext &ctx) : ctx(ctx) {}

public:
  void visit(ast::Prog &prog);
  void visit(ast::FuncDecl &fdecl);
  void visit(ast::FuncHeaderDecl &fhdecl);
  void visit(ast::Arg &arg);
  void visit(ast::StmtBlockExpr &sbexpr);
  void visit(ast::VarDeclStmt &vdstmt);
  void visit(ast::ExprStmt &estmt);
  void visit(ast::RetExpr &rexpr);
  void visit(ast::BreakExpr &bexpr);
  void visit(ast::ContinueExpr &cexpr);
  void visit(ast::AssignExpr &aexpr);
  void visit(ast::AssignElem &aelem);
  void visit(ast::Variable &var);
  void visit(ast::ArrAcc &aacc);
  void visit(ast::TupAcc &tacc);
  void visit(ast::CmpExpr &cexpr);
  void visit(ast::AriExpr &aexpr);
  void visit(ast::ArrElems &aelems);
  void visit(ast::TupElems &telems);
  void visit(ast::Number &num);
  void visit(ast::BracketExpr &bexpr);
  void visit(ast::CallExpr &cexpr);
  void visit(ast::IfExpr &iexpr);
  void visit(ast::ElseClause &eclause);
  void visit(ast::WhileLoopExpr&wlexpr);
  void visit(ast::ForLoopExpr &flexpr);
  void visit(ast::Interval &interval);
  void visit(ast::IterableVal &iter);
  void visit(ast::LoopExpr&lexpr);

private:
  sem::SemanticContext &ctx;

  int tmpcnt = 0;
};

} // namespace ir
