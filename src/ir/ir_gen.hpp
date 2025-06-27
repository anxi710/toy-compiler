#pragma once

#include <vector>

#include "ast/ast.hpp"
#include "symbol/symbol_table.hpp"

namespace ir {

enum class OpCode : std::int8_t {
  Add, Sub, Mul, Div,

  Jeq, Jne, Jge, Jgt, Jle, Jlt,

  Eq, Neq, Geq, Gne, Leq, Lne,

  Decl, Assign,

  Label, Goto,

  Push, Pop, Call, Return
};

struct Operand {
  std::string name;

  Operand() = default;
  Operand(std::string name) : name(std::move(name)) {}
};

struct Quad {
  OpCode  op;
  Operand arg1;
  Operand arg2;
  Operand res;
};

class IrGenerator {
public:
  IrGenerator() = default;
  ~IrGenerator() = default;

  void setSymbolTable(std::shared_ptr<sym::SymbolTable> p_stable);

public:
  void generateProg(const par::ast::ProgPtr &p_prog);

  void printQuads(std::ofstream &out) const;

private:
  void generateFuncDecl(const par::ast::FuncDeclPtr &p_fdecl);
  void generateFuncHeaderDecl(const par::ast::FuncHeaderDeclPtr &p_fhdecl);
  auto generateBlockStmt(const par::ast::BlockStmtPtr &p_bstmt) -> bool;
  void generateVarDeclStmt(const par::ast::VarDeclStmtPtr &p_vdstmt);
  void generateRetStmt(const par::ast::RetStmtPtr &p_rstmt);
  void generateExprStmt(const par::ast::ExprStmtPtr &p_estmt);
  auto generateExpr(const par::ast::ExprPtr &p_expr) -> std::string;
  auto generateCallExpr(const par::ast::CallExprPtr &p_caexpr) -> std::string;
  auto generateComparExpr(const par::ast::ComparExprPtr &p_coexpr) -> std::string;
  auto generateArithExpr(const par::ast::ArithExprPtr &p_aexpr) -> std::string;
  auto generateFactor(const par::ast::FactorPtr &p_factor) -> std::string;
  auto generateElement(const par::ast::ExprPtr &p_element) -> std::string;
  auto generateParenthesisExpr(const par::ast::ParenthesisExprPtr &p_pexpr) -> std::string;
  auto generateNumber(const par::ast::NumberPtr &p_number) -> std::string;
  auto generateVariable(const par::ast::VariablePtr &p_variable) -> std::string;
  void generateAssignStmt(const par::ast::AssignStmtPtr &p_astmt);
  void generateIfStmt(const par::ast::IfStmtPtr &p_istmt);
  void generateWhileStmt(const par::ast::WhileStmtPtr &p_wstmt);

  [[nodiscard]]
  auto getVarName(const std::string &var_name) const -> std::string;
  [[nodiscard]]
  auto getFuncName() const -> std::string;

  void pushQuads(OpCode op, const Operand &arg1, const Operand &arg2,
                 const Operand &res);

private:
  std::vector<Quad> quads;

  std::shared_ptr<sym::SymbolTable> p_stable;
};

} // namespace ir
