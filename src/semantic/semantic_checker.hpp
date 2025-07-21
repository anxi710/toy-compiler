#pragma once

#include "ast.hpp"
#include "crtp_visitor.hpp"
#include "semantic_context.hpp"

namespace err {

class ErrReporter;

} // namespace err

namespace sem {

class SemanticChecker : public ast::CRTPVisitor<SemanticChecker> {
public:
  SemanticChecker(sem::SemanticContext &ctx, err::ErrReporter &reporter)
    : ctx(ctx), reporter(reporter) {}
  ~SemanticChecker() override = default;

public:
  void visit(ast::FuncDecl&);
  void visit(ast::FuncHeaderDecl&);
  void visit(ast::Arg&);
  void visit(ast::StmtBlockExpr&);
  static void visit(ast::EmptyStmt&);
  void visit(ast::VarDeclStmt&);
  void visit(ast::ExprStmt&);
  void visit(ast::RetExpr&);
  void visit(ast::BreakExpr&);
  void visit(ast::ContinueExpr&);
  void visit(ast::AssignExpr&);
  static void visit(ast::AssignElem&);
  void visit(ast::Variable&);
  void visit(ast::ArrAcc&);
  void visit(ast::TupAcc&);
  void visit(ast::CmpExpr&);
  void visit(ast::AriExpr&);
  void visit(ast::ArrElems&);
  void visit(ast::TupElems&);
  void visit(ast::Number&);
  void visit(ast::BracketExpr&);
  void visit(ast::CallExpr&);
  void visit(ast::IfExpr&);
  void visit(ast::ElseClause&);
  void visit(ast::WhileLoopExpr&);
  void visit(ast::ForLoopExpr&);
  void visit(ast::RangeExpr&);
  void visit(ast::IterableVal&);
  void visit(ast::LoopExpr&);

private:
  void checkInit(ast::Expr &expr);
  void checkCondition(ast::Expr &cond, util::Position pos);

private:
  SemanticContext  &ctx;      // semantic context
  err::ErrReporter &reporter; // error reporter
};

} // namespace sem
