/**
 * @file parser.hpp
 * @brief Defines the Parser class for syntactic and semantic analysis of source code.
 *
 * The Parser class is responsible for parsing tokens produced by the lexer,
 * constructing the abstract syntax tree (AST), and building the semantic intermediate representation (IR).
 * It provides methods for parsing various language constructs such as programs, functions, statements, and expressions.
 *
 * Dependencies:
 * - ast.hpp: Abstract Syntax Tree node definitions.
 * - lexer.hpp: Tokenizer for source code.
 * - position.hpp: Source code position tracking.
 * - err_report.hpp: Error reporting utilities.
 * - semantic_ir_builder.hpp: Semantic IR construction.
 *
 * Namespace: par
 */
#pragma once

#include <optional>

#include "ast.hpp"
#include "lexer.hpp"
#include "position.hpp"
#include "err_report.hpp"
#include "semantic_ir_builder.hpp"

namespace par {

/// @class Parser
/// @brief Implements a recursive descent parser for the language.
///
/// The Parser consumes tokens from a Lexer, reports errors via ErrReporter,
/// and builds semantic IR using SemanticIRBuilder. It supports lookahead and
/// provides methods for parsing all major syntactic constructs.
///
/// Usage:
///   - Construct with references to a Lexer, SemanticIRBuilder, and ErrReporter.
///   - Call parseProgram() to parse the entire program.
///
/// Member Functions:
///   - parseProgram: Parse the top-level program.
///   - Various parse* methods: Parse specific constructs (functions, statements, expressions, etc.).
///   - Utility methods: Token management (advance, match, check, consume, etc.).
///
/// Member Variables:
///   - cur: The current token.
///   - la: Optional lookahead token.
///   - lexer: Reference to the token source.
///   - builder: Reference to the semantic IR builder.
///   - reporter: Reference to the error reporter.
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
  auto parseAssignExpr(ast::AssignElemPtr &&lval) -> ast::AssignExprPtr;
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
