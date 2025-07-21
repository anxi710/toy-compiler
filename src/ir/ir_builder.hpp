#pragma once

#include "ast.hpp"
#include "crtp_visitor.hpp"

namespace sem { class SemanticContext; }

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
  IRBuilder(sem::SemanticContext &ctx);

public:
  static void visit(ast::Prog&);
  static void visit(ast::FuncDecl&);
  static void visit(ast::FuncHeaderDecl&);
  static void visit(ast::StmtBlockExpr&);
  void visit(ast::VarDeclStmt&);
  static void visit(ast::ExprStmt&);
  void visit(ast::RetExpr&);
  void visit(ast::BreakExpr&);
  void visit(ast::ContinueExpr&);
  static void visit(ast::AssignExpr&);
  static void visit(ast::AssignElem&);
  void visit(ast::Variable&);
  void visit(ast::ArrAcc&);
  void visit(ast::TupAcc&);
  void visit(ast::CmpExpr&);
  void visit(ast::AriExpr&);
  void visit(ast::ArrElems&);
  void visit(ast::TupElems&);
  void visit(ast::Number&);
  static void visit(ast::BracketExpr&);
  void visit(ast::CallExpr&);
  void visit(ast::IfExpr&);
  void visit(ast::ElseClause&);
  void visit(ast::WhileLoopExpr&);
  void visit(ast::ForLoopExpr&);
  void visit(ast::RangeExpr&);
  void visit(ast::IterableVal&);
  void visit(ast::LoopExpr&);

private:
  sem::SemanticContext &ctx;
};

} // namespace ir
