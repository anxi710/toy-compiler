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
  void visit(ast::FuncDecl &fdecl);
  void visit(ast::FuncHeaderDecl &fhdecl);
  void visit(ast::Arg &arg);
  void visit(ast::StmtBlockExpr &sbexpr);
  void visit(ast::EmptyStmt &estmt);
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
  void visit(ast::LoopExpr &lexpr);

private:
  void checkInit(ast::Expr &expr);
  void checkCondition(ast::Expr &cond, util::Position pos);

private:
  SemanticContext  &ctx;      // semantic context
  err::ErrReporter &reporter; // error reporter
};

} // namespace sem
