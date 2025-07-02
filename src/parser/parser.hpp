#pragma once

#include <optional>

#include "ast.hpp"
#include "lexer.hpp"
#include "err_report.hpp"
#include "semantic_checker.hpp"

namespace par::base {

class Parser {
public:
  Parser(lex::base::Lexer& lexer, sym::SymbolTable& stable,
    sem::SemanticChecker& schecker, err::ErrReporter& ereporter
  );
  ~Parser() = default;

public:
  auto parseProgram() -> ast::ProgPtr;

private:
  auto nextToken() -> std::optional<lex::token::Token>;
  void advance();
  bool match(lex::token::TokenType type);
  bool check(lex::token::TokenType type) const;
  bool checkAhead(lex::token::TokenType type);
  void expect(lex::token::TokenType type, const std::string &msg);

  auto parseFuncDecl() -> ast::FuncDeclPtr;
  auto parseFuncHeaderDecl() -> ast::FuncHeaderDeclPtr;
  auto parseArg() -> ast::ArgPtr;
  auto parseType() -> type::TypePtr;
  auto parseBlockStmt() -> ast::BlockStmtPtr;
  auto parseStmtOrExpr() -> ast::StmtOrExprPtr;
  auto parseRetStmt() -> ast::RetStmtPtr;
  auto parseVarDeclStmt() -> ast::VarDeclStmtPtr;
  auto parseAssignStmt(ast::AssignElemPtr &&lvalue) -> ast::AssignStmtPtr;
  auto parseAssignElem() -> ast::AssignElemPtr;
  auto parseExpr(std::optional<ast::AssignElemPtr> elem = std::nullopt) -> ast::ExprPtr;
  auto parseCmpExpr(std::optional<ast::AssignElemPtr> elem = std::nullopt) -> ast::ExprPtr;
  auto parseAddExpr(std::optional<ast::AssignElemPtr> elem = std::nullopt) -> ast::ExprPtr;
  auto parseMulExpr(std::optional<ast::AssignElemPtr> elem = std::nullopt) -> ast::ExprPtr;
  auto parseFactor(std::optional<ast::AssignElemPtr> elem = std::nullopt) -> ast::ExprPtr;
  auto parseElement(std::optional<ast::AssignElemPtr> elem = std::nullopt) -> ast::ExprPtr;
  auto parseCallExpr() -> ast::CallExprPtr;
  auto parseIfStmt() -> ast::IfStmtPtr;
  auto parseElseClause() -> ast::ElseClausePtr;
  auto parseWhileStmt() -> ast::WhileStmtPtr;
  auto parseForStmt() -> ast::ForStmtPtr;
  auto parseLoopStmt() -> ast::LoopStmtPtr;
  auto parseBreakStmt() -> ast::BreakStmtPtr;
  auto parseFuncExprBlockStmt() -> ast::FuncExprBlockStmtPtr;
  auto parseIfExpr() -> ast::IfExprPtr;

private:
  lex::token::Token                ctoken;  // current token
  std::optional<lex::token::Token> latoken; // look ahead token

  lex::base::Lexer&     lexer;
  sym::SymbolTable&     stable;    // symbol table
  sem::SemanticChecker& schecker;  // semantic checker
  err::ErrReporter&     ereporter; // error reporter
};

} // namespace parser::base
