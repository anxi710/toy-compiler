#pragma once

#include <memory>

#include "error/err_report.hpp"
#include "ast/ast.hpp"
#include "symbol/symbol_table.hpp"

namespace sem {

class SemanticChecker {
public:
  SemanticChecker() = default;
  ~SemanticChecker() = default;

  void setErrReporter(std::shared_ptr<err::ErrReporter> p_ereporter);
  void setSymbolTable(std::shared_ptr<sym::SymbolTable> p_stable);

public:
  void checkProg(const par::ast::ProgPtr &p_prog);

private:
  void checkFuncDecl(const par::ast::FuncDeclPtr &p_fdecl);
  void checkFuncHeaderDecl(const par::ast::FuncHeaderDeclPtr &p_fhdecl);
  auto checkBlockStmt(const par::ast::BlockStmtPtr &p_bstmt) -> bool;
  void checkVarDeclStmt(const par::ast::VarDeclStmtPtr &p_vdstmt);
  void checkRetStmt(const par::ast::RetStmtPtr &p_rstmt);
  auto checkExprStmt(const par::ast::ExprStmtPtr &p_estmt) -> sym::VarType;
  auto checkExpr(const par::ast::ExprPtr &p_expr) -> sym::VarType;
  auto checkCallExpr(const par::ast::CallExprPtr &p_caexpr) -> sym::VarType;
  auto checkComparExpr(const par::ast::ComparExprPtr &p_coexpr) -> sym::VarType;
  auto checkArithExpr(const par::ast::ArithExprPtr &p_aexpr) -> sym::VarType;
  auto checkFactor(const par::ast::FactorPtr &p_factor) -> sym::VarType;
  auto checkVariable(const par::ast::VariablePtr &p_variable) -> sym::VarType;
  auto checkNumber(const par::ast::NumberPtr &p_number) -> sym::VarType;
  void checkAssignStmt(const par::ast::AssignStmtPtr &p_astmt);
  void checkIfStmt(const par::ast::IfStmtPtr &p_istmt);
  void checkWhileStmt(const par::ast::WhileStmtPtr &p_wstmt);

private:
  std::shared_ptr<sym::SymbolTable> p_stable;
  std::shared_ptr<err::ErrReporter> p_ereporter;
};

} // namespace semantic
