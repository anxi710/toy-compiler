#pragma once

#include <expected>
#include <functional>
#include <memory>
#include <optional>

#include "ast/ast.hpp"
#include "lexer/token.hpp"

namespace err {

struct LexErr;
class ErrReporter;

} // namespace error

namespace par::base {

class Parser {
public:
  Parser() = delete;
  explicit Parser(
    std::function<std::expected<lex::token::Token, err::LexErr>()> nextTokenFunc
  );
  ~Parser() = default;

public:
  [[nodiscard]] auto parseProgram() -> ast::ProgPtr;

private:
  void advance();
  auto match(lex::token::TokenType type) -> bool;
  [[nodiscard]] auto check(lex::token::TokenType type) const -> bool;
  auto checkAhead(lex::token::TokenType type) -> bool;
  void expect(lex::token::TokenType type, const std::string &error_msg);

  [[nodiscard]] auto parseArg() -> ast::ArgPtr;
  [[nodiscard]] auto parseIfExpr() -> ast::IfExprPtr;
  [[nodiscard]] auto parseIfStmt() -> ast::IfStmtPtr;
  [[nodiscard]] auto parseForStmt() -> ast::ForStmtPtr;
  [[nodiscard]] auto parseRetStmt() -> ast::RetStmtPtr;
  [[nodiscard]] auto parseVarType() -> ast::VarTypePtr;
  [[nodiscard]] auto parseFuncDecl() -> ast::FuncDeclPtr;
  [[nodiscard]] auto parseCallExpr() -> ast::CallExprPtr;
  [[nodiscard]] auto parseLoopStmt() -> ast::LoopStmtPtr;
  [[nodiscard]] auto parseBlockStmt() -> ast::BlockStmtPtr;
  [[nodiscard]] auto parseBreakStmt() -> ast::BreakStmtPtr;
  [[nodiscard]] auto parseWhileStmt() -> ast::WhileStmtPtr;
  [[nodiscard]] auto parseElseClause() -> ast::ElseClausePtr;
  [[nodiscard]] auto parseStmtOrExpr() -> ast::NodePtr;
  [[nodiscard]] auto parseVarDeclStmt() -> ast::VarDeclStmtPtr;
  [[nodiscard]] auto parseAssignElement() -> ast::AssignElementPtr;
  [[nodiscard]] auto parseFuncHeaderDecl() -> ast::FuncHeaderDeclPtr;
  [[nodiscard]] auto parseFuncExprBlockStmt() -> ast::FuncExprBlockStmtPtr;
  [[nodiscard]] auto parseAssignStmt(ast::AssignElementPtr &&lvalue)
      -> ast::AssignStmtPtr;

  [[nodiscard]] auto
  parseExpr(std::optional<ast::AssignElementPtr> elem = std::nullopt)
      -> ast::ExprPtr;
  [[nodiscard]] auto
  parseFactor(std::optional<ast::AssignElementPtr> elem = std::nullopt)
      -> ast::ExprPtr;
  [[nodiscard]] auto
  parseCmpExpr(std::optional<ast::AssignElementPtr> elem = std::nullopt)
      -> ast::ExprPtr;
  [[nodiscard]] auto
  parseAddExpr(std::optional<ast::AssignElementPtr> elem = std::nullopt)
      -> ast::ExprPtr;
  [[nodiscard]] auto
  parseMulExpr(std::optional<ast::AssignElementPtr> elem = std::nullopt)
      -> ast::ExprPtr;
  [[nodiscard]] auto
  parseElement(std::optional<ast::AssignElementPtr> elem = std::nullopt)
      -> ast::ExprPtr;

private:
  std::shared_ptr<err::ErrReporter> reporter; // error reporter

  std::function<std::expected<lex::token::Token, err::LexErr>()>
      nextTokenFunc; // 获取下一个 token

  lex::token::Token current;                  // 当前看到的 token
  std::optional<lex::token::Token> lookahead; // 往后看一个 token
};

} // namespace parser::base
