#pragma once

#include "ast.hpp"
#include "position.hpp"
#include "err_report.hpp"
#include "symbol_table.hpp"

namespace sem {

class SemanticChecker {
public:
  SemanticChecker(sym::SymbolTable &stable, err::ErrReporter &ereporter)
    : stable(stable), ereporter(ereporter) {}
  ~SemanticChecker() = default;

public:
  void check(const ast::FuncDecl &func);
  void check(const ast::FuncHeaderDecl &fhdecl);
  void check(const ast::BlockStmt &bstmt);
  void check(const ast::FuncExprBlockStmt &febstmt);
  void check(const ast::RetStmt &rstmt);
  void check(const ast::VarDeclStmt &vdstmt);
  void check(const ast::AssignStmt &astmt);
  void check(const ast::Variable &var);
  void check(const ast::ArrayAccess &aacc);
  void check(const ast::TupleAccess &aacc);
  void check(const ast::CmpExpr &cexpr);
  void check(const ast::AriExpr &aexpr);
  void check(const ast::ArrayElems &aelems);
  void check(const ast::TupleElems &telems);
  void check(const ast::BracketExpr &bexpr);
  void check(const ast::Factor &factor);
  void check(const ast::Number &num);
  void check(const ast::CallExpr &cexpr);
  void check(const ast::IfStmt &istmt);
  void check(const ast::ElseClause &eclause);
  void check(const ast::WhileStmt &wstmt);
  void check(const ast::ForStmt &fstmt);
  void check(const ast::LoopStmt &lstmt);
  void check(const ast::BreakStmt &bstmt);
  void check(const ast::IfExpr &iexpr);

private:
  sym::SymbolTable&      stable;    // Symbol Table
  err::ErrReporter&      ereporter; // Error Reporter
};

} // namespace semantic
