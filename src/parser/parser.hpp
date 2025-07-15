#pragma once

#include <optional>

#include "ast.hpp"
#include "lexer.hpp"
#include "position.hpp"
#include "err_report.hpp"
#include "semantic_ir_builder.hpp"

namespace par {

class Parser {
public:
  Parser(lex::Lexer &lexer, SemanticIRBuilder &builder,
    err::ErrReporter &reporter
  ) : lexer(lexer), builder(builder), reporter(reporter) {
    advance(); // 初始化，使 current 指向第一个 token
  }
  ~Parser() = default;

public:
  auto parseProgram() -> ast::ProgPtr;

private:
  auto nextToken() -> lex::Token;
  void advance();
  bool match(lex::TokenType type);
  bool check(lex::TokenType type) const;
  bool checkAhead(lex::TokenType type);
  void consume(lex::TokenType type, const std::string &msg);

  auto parseID() -> std::pair<std::string, util::Position>;
  auto parseInnerVarDecl() -> std::tuple<bool, std::string, util::Position>;
  auto parseFuncDecl() -> ast::FuncDeclPtr;
  auto parseFuncHeaderDecl() -> ast::FuncHeaderDeclPtr;
  auto parseArg() -> ast::ArgPtr;
  auto parseType() -> ast::Type;
  auto parseStmtBlockExpr() -> ast::StmtBlockExprPtr;
  auto parseStmt() -> ast::StmtPtr;
  auto parseVarDeclStmt() -> ast::VarDeclStmtPtr;
  auto parseExprStmt() -> ast::ExprStmtPtr;
  auto parseExpr() -> ast::ExprPtr;
  auto parseRetExpr() -> ast::RetExprPtr;
  auto parseBreakExpr() -> ast::BreakExprPtr;
  auto parseContinueExpr() -> ast::ContinueExprPtr;
  auto parseAssignExpr(ast::AssignElemPtr &&lvalue) -> ast::AssignExprPtr;
  auto parseAssignElem(std::optional<ast::ExprPtr> val = std::nullopt) -> ast::AssignElemPtr;
  auto parseArrAcc(ast::ExprPtr val) -> ast::ArrAccPtr;
  auto parseTupAcc(ast::ExprPtr val) -> ast::TupAccPtr;
  auto parseValue() -> ast::ExprPtr;
  auto parseCmpExpr(std::optional<ast::ExprPtr> expr = std::nullopt) -> ast::ExprPtr;
  auto parseAddExpr(std::optional<ast::ExprPtr> expr = std::nullopt) -> ast::ExprPtr;
  auto parseMulExpr(std::optional<ast::ExprPtr> expr = std::nullopt) -> ast::ExprPtr;
  auto parseFactor(std::optional<ast::ExprPtr> expr = std::nullopt) -> ast::ExprPtr;
  auto parseArrElems() -> ast::ExprPtr;
  auto parseTupElems() -> ast::ExprPtr;
  auto parseElement(std::optional<ast::ExprPtr> expr = std::nullopt) -> ast::ExprPtr;
  auto parseCallExpr() -> ast::CallExprPtr;
  auto parseIfExpr() -> ast::IfExprPtr;
  auto parseElseClause() -> ast::ElseClausePtr;
  auto parseForLoopExpr() -> ast::ForLoopExprPtr;
  auto parseIterable() -> ast::ExprPtr;
  auto parseWhileLoopExpr() -> ast::WhileLoopExprPtr;
  auto parseLoopExpr() -> ast::LoopExprPtr;

private:
  lex::Token                cur;  // current token
  std::optional<lex::Token> la; // look ahead token

  lex::Lexer        &lexer;
  SemanticIRBuilder &builder;  // semantic ir builder
  err::ErrReporter  &reporter; // error reporter
};

} // namespace par
