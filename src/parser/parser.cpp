#include "parser.hpp"

#include <cassert>

#include "panic.hpp"
#include "err_report.hpp"
#include "symbol_table.hpp"

using namespace lex::token;

namespace par::base {

Parser::Parser(lex::base::Lexer& lexer, sym::SymbolTable& stable,
  sem::SemanticChecker& schecker, err::ErrReporter& ereporter
) : lexer(lexer), stable(stable), schecker(schecker), ereporter(ereporter)
{
  advance(); // 初始化，使 current 指向第一个 token
}

std::optional<Token>
Parser::nextToken()
{
  if (auto token = lexer.nextToken(); token.has_value()) {
    return token.value();
  }
  // 如果识别到未知 token，则发生了词法分析错误，且需要立即终止
  ereporter.terminateProg();
}

/**
 * @brief 向前扫描一个 token
 */
void
Parser::advance()
{
  if (latoken.has_value()) {
    ctoken = latoken.value();
    latoken.reset(); // 清除 latoken 中的值
  } else {
    ctoken = nextToken().value();
  }
}

/**
 * @brief  匹配当前 token，并向前扫描一个 token
 * @param  type 需匹配的 token 类型
 * @return 是否成功匹配
 */
bool
Parser::match(TokenType type)
{
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

/**
 * @brief  检查当前看到的 token 是否存在，如果存在判断其类型是否为给定值
 * @param  type 指定的 token 类型
 * @return 是否通过检查
 */
bool
Parser::check(TokenType type) const
{
  return ctoken.type == type;
}

/**
 * @brief  向前检查一个 token，判断其类型是否为给定值
 * @param  type 指定的 token 类型
 * @return 是否通过检查
 */
bool
Parser::checkAhead(TokenType type)
{
  if (!latoken.has_value()) {
    latoken = nextToken();
  }
  return latoken.has_value() && latoken.value().type == type;
}

/**
 * @brief 匹配期望的 token，如果未匹配成功则抛出 runtime error
 * @param type 期望的 token 类型
 * @param msg  错误信息
 */
void
Parser::expect(TokenType type, const std::string &msg)
{
  if (!match(type)) {
    ereporter.report(
      err::ParErrType::UNEXPECT_TOKEN,
      msg,
      ctoken.pos.row,
      ctoken.pos.col,
      ctoken.value
    );
  }
}

ast::ProgPtr
Parser::parseProgram()
{
  // Prog -> (FuncDecl)*
  std::vector<ast::DeclPtr> decls;
  while (check(TokenType::FN)) {
    decls.push_back(parseFuncDecl());
  }

  return std::make_shared<ast::Prog>(decls);
}

ast::FuncDeclPtr
Parser::parseFuncDecl()
{
  // FuncDecl -> FuncHeaderDecl BlockStmt
  auto header = parseFuncHeaderDecl();
  auto body   = parseBlockStmt();

  auto func = std::make_shared<ast::FuncDecl>(header, body);
  schecker.check(*func);

  return func;
}

ast::FuncHeaderDeclPtr
Parser::parseFuncHeaderDecl()
{
  // FuncHeaderDecl -> fn <ID> ( (arg)* ) (-> VarType)?
  expect(TokenType::FN, "Expected 'fn'");

  std::string    name = ctoken.value;
  util::Position pos  = ctoken.pos;
  expect(TokenType::ID, "Expected '<ID>'");

  expect(TokenType::LPAREN, "Expected '('");

  // (arg)*
  std::vector<ast::ArgPtr> argv{};
  while (!check(TokenType::RPAREN)) {
    argv.push_back(parseArg());
    if (!check(TokenType::COMMA)) {
      break;
    }
    advance();
  }

  expect(TokenType::RPAREN, "Expected ')'");

  // (-> VarType)?
  std::optional<type::TypePtr> rtype = std::nullopt;
  if (check(TokenType::ARROW)) {
    advance();
    rtype = parseType();
  }

  auto fhdecl = std::make_shared<ast::FuncHeaderDecl>(pos, name, argv, rtype);
  schecker.check(*fhdecl);
  return fhdecl;
}

ast::ArgPtr
Parser::parseArg()
{
  bool mut = false;
  if (check(TokenType::MUT)) {
    mut = true;
    advance();
  }

  std::string    name = ctoken.value;
  util::Position pos  = ctoken.pos;
  expect(TokenType::ID, "Expected '<ID>'");
  expect(TokenType::COLON, "Expected ':'");

  auto type = parseType();

  return std::make_shared<ast::Arg>(pos, mut, name, type);
}

type::TypePtr
Parser::parseType()
{
  if (check(TokenType::LBRACK)) {
    advance();
    auto etype = parseType();

    expect(TokenType::SEMICOLON, "Expected ';'");

    int cnt = std::stoi(ctoken.value);
    expect(TokenType::INT, "Expected <NUM>");
    expect(TokenType::RBRACK, "Expected ']'");

    return std::make_shared<type::ArrayType>(cnt, etype);
  }

  if (check(TokenType::LPAREN)) {
    advance();

    bool is_tuple;
    std::vector<type::TypePtr> etypes;
    while (!check(TokenType::RPAREN)) {
      etypes.push_back(parseType());
      if (!check(TokenType::COMMA)) {
        break;
      }
      advance();
      is_tuple = true;
    }

    expect(TokenType::RPAREN, "Expected ')'");
    if (0 == etypes.size()) {
      //DEBUG Error Reporter
      throw std::runtime_error{"Incorrect variable type."};
    }
    if (is_tuple) {
      return std::make_shared<type::TupleType>(etypes);
    }
    return etypes[0];
  }

  if (check(TokenType::I32)) {
    advance();
    return std::make_shared<type::IntType>();
  }

  //TODO error report
  util::unreachable("par::Parse::parseType()");
}

ast::BlockStmtPtr
Parser::parseBlockStmt()
{
  // BlockStmt -> { (Stmt)* }; FuncExprBlockStmt -> { (Stmt)* Expr }
  expect(TokenType::LBRACE, "Expected '{'");

  ast::ExprPtr expr;
  std::vector<ast::StmtPtr> stmts{};

  bool is_func_expr = false;
  while (!check(TokenType::RBRACE)) {
    ast::StmtOrExprPtr node = parseStmtOrExpr();
    if (node->stmt.has_value()) {
      stmts.push_back(std::move(node->stmt.value()));
    } else {
      expr = std::move(node->expr.value());
      is_func_expr = true;
      break;
    }
  }

  expect(TokenType::RBRACE, "Expected '}'");
  if (is_func_expr) {
    auto febstmt = std::make_shared<ast::FuncExprBlockStmt>(stmts, expr);
    schecker.check(*febstmt);
    return febstmt;
  }

  auto bstmt = std::make_shared<ast::BlockStmt>(stmts);
  schecker.check(*bstmt);
  return bstmt;
}

ast::StmtOrExprPtr
Parser::parseStmtOrExpr()
{
  ast::StmtOrExprPtr node;
  if (check(TokenType::LET)) {
    node->stmt = parseVarDeclStmt();
  } else if (check(TokenType::RETURN)) {
    node->stmt = parseRetStmt();
  } else if (check(TokenType::ID)) {
    if (checkAhead(TokenType::LPAREN)) {
      node->expr = parseCallExpr();
    }
    /*
     * x, x[idx], x.idx
     * 都即可以作为赋值语句的左值，又可以作为表达式的一个操作数
     */
    auto elem = parseAssignElem();
    if (check(TokenType::ASSIGN)) {
      node->stmt = parseAssignStmt(std::move(elem));
    } else {
      node->expr = parseExpr(elem);
    }
  } else if (check(TokenType::INT) || check(TokenType::LPAREN)) {
    node->expr = parseExpr();
  } else if (check(TokenType::IF)) {
    node->stmt = parseIfStmt();
  } else if (check(TokenType::WHILE)) {
    node->stmt = parseWhileStmt();
  } else if (check(TokenType::FOR)) {
    node->stmt = parseForStmt();
  } else if (check(TokenType::LOOP)) {
    node->stmt = parseLoopStmt();
  } else if (check(TokenType::BREAK)) {
    node->stmt = parseBreakStmt();
  } else if (check(TokenType::CONTINUE)) {
    advance();
    expect(TokenType::SEMICOLON, "Expected ';' after Continue");
    node->stmt = std::make_shared<ast::ContinueStmt>();
  } else if (check(TokenType::SEMICOLON)) {
    advance();
    node->stmt = std::make_shared<ast::NullStmt>();
  }

  if (check(TokenType::COLON) && node->expr.has_value()) {
    node->stmt = std::make_shared<ast::ExprStmt>(node->expr.value());
    node->expr.reset();
  }

  return node;
}

ast::RetStmtPtr
Parser::parseRetStmt()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::RETURN, "Expected 'return'");

  std::optional<ast::ExprPtr> retval = std::nullopt;
  if (!check(TokenType::SEMICOLON)) {
    retval = parseCmpExpr();
  }

  expect(TokenType::SEMICOLON, "Expected ';'");

  auto rstmt = std::make_shared<ast::RetStmt>(pos, retval);
  schecker.check(*rstmt);
  return rstmt;
}

ast::VarDeclStmtPtr
Parser::parseVarDeclStmt()
{
  // VarDeclStmt -> let (mut)? <ID> (: VarType)? (= Expr);
  expect(TokenType::LET, "Expected 'let'");

  bool mut = false;
  if (check(TokenType::MUT)) {
    mut = true;
    advance();
  }

  util::Position pos  = ctoken.pos;
  std::string    name = ctoken.value;
  expect(TokenType::ID, "Expected '<ID>'");

  std::optional<type::TypePtr> type = std::nullopt;
  if (check(TokenType::COLON)) {
    advance();
    type = parseType();
  }

  std::optional<ast::ExprPtr> expr = std::nullopt;
  if (check(TokenType::ASSIGN)) {
    advance();
    expr = parseExpr();
  }

  expect(TokenType::SEMICOLON, "Expected ';'");

  auto vdstmt = std::make_shared<ast::VarDeclStmt>(pos, mut, name, type, expr);
  schecker.check(*vdstmt);
  return vdstmt;
}

ast::AssignStmtPtr
Parser::parseAssignStmt(ast::AssignElemPtr &&lvalue)
{
  util::Position pos = ctoken.pos;
  expect(TokenType::ASSIGN, "Expected '='");
  ast::ExprPtr expr = Parser::parseExpr();

  expect(TokenType::SEMICOLON, "Expected ';'");

  auto astmt = std::make_shared<ast::AssignStmt>(pos, lvalue, expr);
  schecker.check(*astmt);
  return astmt;
}

ast::AssignElemPtr
Parser::parseAssignElem()
{
  util::Position pos = ctoken.pos;

  std::string name = ctoken.value;
  expect(TokenType::ID, "Expected '<ID>'");
  if (check(TokenType::LBRACK)) {
    advance();
    auto expr = parseExpr();
    expect(TokenType::RBRACK, "Expected ']'");

    auto aacc = std::make_shared<ast::ArrayAccess>(pos, name, expr);
    schecker.check(*aacc);
    return aacc;
  }

  if (check(TokenType::DOT)) {
    advance();
    int value = std::stoi(ctoken.value);
    expect(TokenType::INT, "Expected <NUM> for Tuple");

    auto tacc = std::make_shared<ast::TupleAccess>(pos, name, value);
    schecker.check(*tacc);
    return tacc;
  }

  auto var = std::make_shared<ast::Variable>(pos, name);
  schecker.check(*var);
  return var;
}

/**
 * @brief  解析表达式，用递归下降解析分层处理运算优先级
 * @return ast::ExprPtr - 顶层比较表达式
 */
ast::ExprPtr
Parser::parseExpr(std::optional<ast::AssignElemPtr> elem)
{
  if (check(TokenType::LBRACE)) {
    return parseFuncExprBlockStmt();
  }
  if (check(TokenType::IF)) {
    return parseIfExpr();
  }
  if (check(TokenType::LOOP)) {
    return parseLoopStmt();
  }
  return Parser::parseCmpExpr(std::move(elem));
}

static ast::CmpOper
tokenType2CmpOper(TokenType type)
{
  switch (type) {
    case TokenType::EQ:  return ast::CmpOper::EQ;
    case TokenType::NEQ: return ast::CmpOper::NEQ;
    case TokenType::GEQ: return ast::CmpOper::GEQ;
    case TokenType::LEQ: return ast::CmpOper::LEQ;
    case TokenType::GT:  return ast::CmpOper::GT;
    case TokenType::LT:  return ast::CmpOper::LT;
    default:
      util::unreachable("par::tokenType2CmpOper()");
  }
}

static ast::AriOper
tokenType2AriOper(TokenType type)
{
  switch (type) {
    case TokenType::PLUS:  return ast::AriOper::ADD;
    case TokenType::MINUS: return ast::AriOper::SUB;
    case TokenType::MUL:   return ast::AriOper::MUL;
    case TokenType::DIV:   return ast::AriOper::DIV;
    default:
      util::unreachable("par::tokenType2AriOper");
  }
}

ast::ExprPtr
Parser::parseCmpExpr(std::optional<ast::AssignElemPtr> elem)
{
  ast::ExprPtr lhs = parseAddExpr(std::move(elem));
  while (
    check(TokenType::LT) || check(TokenType::LEQ) || check(TokenType::GT) ||
    check(TokenType::GEQ) || check(TokenType::EQ) || check(TokenType::NEQ)
  ) {
    util::Position pos = ctoken.pos;

    TokenType op = ctoken.type;
    advance();

    ast::ExprPtr rhs = parseAddExpr();

    auto cexpr = std::make_shared<ast::CmpExpr>(
      pos, lhs, tokenType2CmpOper(op), rhs
    );
    schecker.check(*cexpr);
    lhs = cexpr;
  } // end while

  return lhs;
}

ast::ExprPtr
Parser::parseAddExpr(std::optional<ast::AssignElemPtr> elem)
{
  ast::ExprPtr lhs = Parser::parseMulExpr(std::move(elem));
  while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
    util::Position pos = ctoken.pos;

    TokenType op = ctoken.type;
    advance();

    ast::ExprPtr right = parseMulExpr();

    auto aexpr = std::make_shared<ast::AriExpr>(
      pos, lhs, tokenType2AriOper(op), right
    );
    schecker.check(*aexpr);
    lhs = aexpr;
  } // end while

  return lhs;
}

ast::ExprPtr
Parser::parseMulExpr(std::optional<ast::AssignElemPtr> elem)
{
  ast::ExprPtr lhs = parseFactor(std::move(elem));
  while (check(TokenType::MUL) || check(TokenType::DIV)) {
    util::Position pos = ctoken.pos;

    TokenType op = ctoken.type;
    advance();

    ast::ExprPtr rhs = parseFactor();

    auto aexpr = std::make_shared<ast::AriExpr>(
      pos, lhs, tokenType2AriOper(op), rhs
    );
    schecker.check(*aexpr);
    lhs = aexpr;
  } // end while

  return lhs;
}

ast::ExprPtr
Parser::parseFactor(std::optional<ast::AssignElemPtr> elem)
{
  util::Position pos = ctoken.pos;
  // ArrayElements
  if (check(TokenType::LBRACK)) {
    advance();
    std::vector<ast::ExprPtr> elems{};
    while (!check(TokenType::RBRACK)) {
      elems.push_back(parseExpr());
      if (!check(TokenType::COMMA)) {
        break;
      }
      advance();
    }
    advance();

    auto aelems = std::make_shared<ast::ArrayElems>(pos, elems);
    schecker.check(*aelems);
    return aelems;
  }

  // TupleElements
  if (check(TokenType::LPAREN)) {
    advance();
    std::vector<ast::ExprPtr> elems{};

    while (!check(TokenType::RPAREN)) {
      elems.push_back(parseExpr());
      if (!check(TokenType::COMMA)) {
        break;
      }
      advance();
    }

    expect(TokenType::RPAREN, "Expected ')'");
    auto cnt = elems.size();

    if (0 == cnt) {
      throw std::runtime_error{"Unsupport the unit type."};
    }
    if (1 == cnt) {
      // 单个表达式没有逗号不是元组，而是普通括号表达式
      auto bexpr = std::make_shared<ast::BracketExpr>(pos, elems[0]);
      schecker.check(*bexpr);
      return bexpr;
    }

    auto telems = std::make_shared<ast::TupleElems>(pos, elems);
    schecker.check(*telems);
    return telems;
  }

  auto element = parseElement(std::move(elem));

  auto factor = std::make_shared<ast::Factor>(pos, element);
  schecker.check(*factor);
  return factor;
}

ast::ExprPtr
Parser::parseElement(std::optional<ast::AssignElemPtr> elem)
{
  if (elem.has_value()) {
    return elem.value();
  }

  util::Position pos = ctoken.pos;
  if (check(TokenType::LPAREN)) {
    advance();
    ast::ExprPtr expr = Parser::parseCmpExpr();
    expect(TokenType::RPAREN, "Expected ')'");
    auto bexpr = std::make_shared<ast::BracketExpr>(pos, expr);
    schecker.check(*bexpr);
    return bexpr;
  }

  if (check(TokenType::INT)) {
    int value = std::stoi(ctoken.value);
    advance();
    auto num = std::make_shared<ast::Number>(pos, value);
    schecker.check(*num);
    return num;
  }

  if (check(TokenType::ID)) {
    if (checkAhead(TokenType::LBRACK) || checkAhead(TokenType::DOT)) {
      return parseAssignElem();
    }
    if (checkAhead(TokenType::LPAREN)) {
      return parseCallExpr();
    }
    std::string name = ctoken.value;
    advance();
    auto var = std::make_shared<ast::Variable>(pos, name);
    schecker.check(*var);
    return var;
  }

  util::unreachable("par::base::Parser::parseElement()");
}

ast::CallExprPtr
Parser::parseCallExpr()
{
  util::Position pos  = ctoken.pos;
  std::string    name = ctoken.value; // function name
  expect(TokenType::ID, "Expected function name");

  expect(TokenType::LPAREN, "Expected '('");

  std::vector<ast::ExprPtr> argv{};
  while (!check(TokenType::RPAREN)) {
    argv.push_back(parseCmpExpr());
    if (!check(TokenType::COMMA)) {
      break;
    }
    advance();
  }

  expect(TokenType::RPAREN, "Expected ')'");
  auto cexpr = std::make_shared<ast::CallExpr>(pos, name, argv);
  schecker.check(*cexpr);
  return cexpr;
}

ast::IfStmtPtr
Parser::parseIfStmt()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::IF, "Expected 'if'");
  auto cond = parseCmpExpr();

  //DEBUG 作用域
  stable.enterScope("if");

  auto body = parseBlockStmt();

  std::vector<ast::ElseClausePtr> elses{};
  while (check(TokenType::ELSE)) {
    bool end = !check(TokenType::IF);
    elses.push_back(parseElseClause());
    if (end) {
      break;
    }
  }

  stable.exitScope();

  auto istmt = std::make_shared<ast::IfStmt>(pos, cond, body, elses);
  schecker.check(*istmt);
  return istmt;
}

ast::ElseClausePtr
Parser::parseElseClause()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::ELSE, "Expected 'else'");

  std::optional<ast::ExprPtr> cond = std::nullopt;
  ast::BlockStmtPtr body;
  if (check(TokenType::IF)) {
    advance();
    cond = parseCmpExpr();
  }
  body = parseBlockStmt();

  auto eclause = std::make_shared<ast::ElseClause>(pos, cond, body);
  schecker.check(*eclause);
  return eclause;
}

ast::WhileStmtPtr
Parser::parseWhileStmt()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::WHILE, "Expected 'while'");
  auto expr = parseCmpExpr();

  //DEBUG
  stable.enterScope("while");

  auto block = parseBlockStmt();

  stable.exitScope();

  auto wstmt = std::make_shared<ast::WhileStmt>(pos, expr, block);
  schecker.check(*wstmt);
  return wstmt;
}

/**
 * @brief  解析 for 语句
 * @return ast::ForStmtPtr - AST For Statement 结点指针
 */
ast::ForStmtPtr
Parser::parseForStmt()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::FOR, "Expected 'for'");

  std::string name = ctoken.value;
  expect(TokenType::ID, "Expected '<ID>'");
  expect(TokenType::IN, "Expected 'in'");

  auto expr1 = parseCmpExpr();
  expect(TokenType::DOTS, "Expected '..'");

  auto expr2 = parseCmpExpr();
  auto block = parseBlockStmt();

  auto fstmt = std::make_shared<ast::ForStmt>(
      pos, name, expr1, expr2, block
  );
  schecker.check(*fstmt);
  return fstmt;
}

ast::LoopStmtPtr
Parser::parseLoopStmt()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::LOOP, "Expected 'loop'");

  auto block = parseBlockStmt();

  auto lstmt = std::make_shared<ast::LoopStmt>(pos, block);
  schecker.check(*lstmt);
  return lstmt;
}

ast::BreakStmtPtr
Parser::parseBreakStmt()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::BREAK, "Expected 'break'");

  std::optional<ast::ExprPtr> value = std::nullopt;
  if (!check(TokenType::SEMICOLON)) {
    value = parseExpr();
  }

  auto bstmt = std::make_shared<ast::BreakStmt>(pos, value);
  schecker.check(*bstmt);
  return bstmt;
}

ast::FuncExprBlockStmtPtr
Parser::parseFuncExprBlockStmt()
{
  auto bstmt = parseBlockStmt();
  auto febstmt = std::static_pointer_cast<ast::FuncExprBlockStmt>(bstmt);
  assert(febstmt);
  return febstmt;
}

/**
 * @brief  解析 if 表达式
 * @return ast::IfExprPtr - AST If Expression 结点指针
 */
ast::IfExprPtr
Parser::parseIfExpr()
{
  util::Position pos = ctoken.pos;
  expect(TokenType::IF, "Expected 'if' for If Expression");

  auto condition = parseExpr();
  auto if_branch = parseFuncExprBlockStmt();

  expect(TokenType::ELSE, "Expected 'else' for If expression");
  auto else_branch = parseFuncExprBlockStmt();

  auto iexpr = std::make_shared<ast::IfExpr>(
    pos, condition, if_branch, else_branch
  );
  schecker.check(*iexpr);
  return iexpr;
}

} // namespace parser::base
