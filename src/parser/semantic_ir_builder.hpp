#pragma once

#include <memory>

#include "ast.hpp"
#include "err_report.hpp"
#include "ir_builder.hpp"
#include "symbol_table.hpp"
#include "semantic_checker.hpp"

namespace par {

class SemanticIRBuilder {
public:
  SemanticIRBuilder(sym::SymbolTable &symtab, err::ErrReporter &reporter)
    : ctx(std::make_unique<sem::SemanticContext>(symtab)), reporter(reporter),
      sema(*ctx, reporter), ir(*ctx) {}
  ~SemanticIRBuilder() = default;

public:
  void build(ast::Prog &prog);
  void build(ast::FuncDecl &fdecl);
  void build(ast::FuncHeaderDecl &fhdecl);
  void build(ast::Arg &arg);
  void build(ast::StmtBlockExpr &sbexpr);
  void build(ast::EmptyStmt &estmt);
  void build(ast::VarDeclStmt &vdstmt);
  void build(ast::ExprStmt &estmt);
  void build(ast::RetExpr &rexpr);
  void build(ast::BreakExpr &bexpr);
  void build(ast::ContinueExpr &cexpr);
  void build(ast::AssignElem &aelem);
  void build(ast::AssignExpr &aexpr);
  void build(ast::Variable &var);
  void build(ast::ArrayAccess &aacc);
  void build(ast::TupleAccess &tacc);
  void build(ast::CmpExpr &cexpr);
  void build(ast::AriExpr &aexpr);
  void build(ast::ArrayElems &aelems);
  void build(ast::TupleElems &telems);
  void build(ast::BracketExpr &bexpr);
  void build(ast::Number &num);
  void build(ast::CallExpr &cexpr);
  void build(ast::IfExpr &iexpr);
  void build(ast::ElseClause &eclause);
  void build(ast::WhileLoopExpr &wlexpr);
  void build(ast::ForLoopExpr &flexpr);
  void build(ast::Interval &interval);
  void build(ast::IterableVal &iter);
  void build(ast::LoopExpr &lexpr);

public:
  std::unique_ptr<sem::SemanticContext> ctx;

private:
  err::ErrReporter     &reporter;
  sem::SemanticChecker  sema;
  ir::IRBuilder         ir;
};

} // namespace par
