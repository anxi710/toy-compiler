#include "parser.hpp"

#include <cassert>

#include "error/err_report.hpp"

using namespace lex::token;

namespace par::base
{

/* constructor */

Parser::Parser(std::function<std::expected<Token, err::LexErr>()> nextTokenFunc)
  : nextTokenFunc(std::move(nextTokenFunc))
{
  advance(); // 初始化，使 current 指向第一个 token
}

/* constructor */

/* member function definition */

/**
 * @brief 向前扫描一个 token
 */
void
Parser::advance()
{
  if (lookahead.has_value()) {
    current = lookahead.value();
    lookahead.reset(); // 清除 lookahead 中的值
  } else {
    if (auto token = nextTokenFunc(); token.has_value()) {
      current = token.value();
    } else {
      // 如果识别到未知 token，则发生了词法分析错误，且需要立即终止
      reporter->report(token.error(), true);
    }
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
  return current.getType() == type;
}

/**
 * @brief  向前检查一个 token，判断其类型是否为给定值
 * @param  type 指定的 token 类型
 * @return 是否通过检查
 */
bool
Parser::checkAhead(TokenType type)
{
  if (!lookahead.has_value()) {
    if (auto token = nextTokenFunc(); token.has_value()) {
      lookahead = token.value(); // 获取下一个 token
    } else {
      // 如果识别到未知 token，则发生了词法分析错误，且需要立即终止
      reporter->report(token.error(), true);
    }
  }
  return lookahead.has_value() && lookahead->getType() == type;
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
    reporter->report(
      err::ParErrType::UnexpectToken,
      msg,
      current.getPos().row,
      current.getPos().col,
      current.getValue()
    );
  }
}

/**
 * @brief  对指定程序进行语法解析
 * @return ast::ProgPtr - AST Program 结点指针 (AST 根结点)
 */
ast::ProgPtr
Parser::parseProgram()
{
  std::vector<ast::DeclPtr> decls; // declarations;

  // Prog -> (FuncDecl)*
  while (check(TokenType::FN)) {
    decls.push_back(parseFuncDecl());
  }

  return std::make_shared<ast::Prog>(std::move(decls));
}

/**
 * @brief  解析函数声明
 * @return ast::FuncDeclPtr - AST Function Declaration 结点指针
 */
ast::FuncDeclPtr
Parser::parseFuncDecl()
{
  // FuncDecl -> FuncHeaderDecl BlockStmt

  util::Position pos = current.getPos();
  auto header = parseFuncHeaderDecl();
  auto body = parseBlockStmt();

  auto p_fdecl = std::make_shared<ast::FuncDecl>(std::move(header), std::move(body));
  p_fdecl->setPos(pos);
  return p_fdecl;
}

/**
 * @brief  解析函数头声明
 * @return ast::FuncHeaderDeclPtr - AST Function Header Declaration 结点指针
 */
ast::FuncHeaderDeclPtr
Parser::parseFuncHeaderDecl()
{
  // FuncHeaderDecl -> fn <ID> ( (arg)* ) (-> VarType)?
  util::Position pos = current.getPos();
  expect(TokenType::FN, "此处期望有一个 'fn'");

  std::string name = current.getValue(); // function name
  expect(TokenType::ID, "此处期望有一个 '<ID>' 作为函数名");

  expect(TokenType::LPAREN, "此处期望有一个 '('");

  std::vector<ast::ArgPtr> argv{};
  while (!check(TokenType::RPAREN)) {
    argv.push_back(parseArg());
    if (!check(TokenType::COMMA)) {
      break;
    }
    advance();
  }

  expect(TokenType::RPAREN, "Expected ')'");

  if (check(TokenType::ARROW)) {
    expect(TokenType::ARROW, "Expected '->'");
    auto type = parseVarType();
    return std::make_shared<ast::FuncHeaderDecl>(
      std::move(name), std::move(argv), std::move(type)
    );
  }

  auto p_fhdecl = std::make_shared<ast::FuncHeaderDecl>(
      std::move(name), std::move(argv), std::nullopt);
  p_fhdecl->setPos(pos);
  return p_fhdecl;
}

/**
 * @brief  解析语句块
 * @return ast::BlockStmtPtr - AST Block Statement 结点指针
 */
ast::BlockStmtPtr
Parser::parseBlockStmt()
{
  // BlockStmt -> { (Stmt)* }; FuncExprBlockStmt -> { (Stmt)* Expr }
  util::Position pos = current.getPos();
  expect(TokenType::LBRACE, "Expected '{' for block");

  std::vector<ast::StmtPtr> stmts{};
  ast::ExprPtr expr;
  bool flag_func_expr = false;
  while (!check(TokenType::RBRACE)) {
    ast::NodePtr node = parseStmtOrExpr();
    if (auto stmt = std::dynamic_pointer_cast<ast::Stmt>(node)) {
      stmts.push_back(stmt);
      continue;
    }
    expr = std::dynamic_pointer_cast<ast::Expr>(node);
    if (nullptr != expr) {
      if (check(TokenType::SEMICOLON)) {
        advance();
        stmts.push_back(std::make_shared<ast::ExprStmt>(std::move(expr)));
        continue;
      }
      flag_func_expr = true;
      break;
    }
  }

  expect(TokenType::RBRACE, "Expected '}' for block");
  if (flag_func_expr) {
    return std::make_shared<ast::FuncExprBlockStmt>(std::move(stmts), std::move(expr));
  }

  auto p_bstmt = std::make_shared<ast::BlockStmt>(std::move(stmts));
  p_bstmt->setPos(pos);
  return p_bstmt;
}

/**
 * @brief  解析语句或表达式
 * @return ast::NodePtr - Stmt 或 Expr 结点指针
 */
ast::NodePtr
Parser::parseStmtOrExpr()
{
  ast::StmtPtr stmt{};
  if (check(TokenType::LET)) {
    stmt = parseVarDeclStmt();
  } else if (check(TokenType::RETURN)) {
    stmt = parseRetStmt();
  } else if (check(TokenType::ID) || check(TokenType::OP_MUL)) {
    if (check(TokenType::ID) && checkAhead(TokenType::LPAREN)) {
      return parseCallExpr();
    }
    /*
     * x, *x, x[idx], x.idx
     * 都即可以作为赋值语句的左值，又可以作为表达式的一个操作数
     */
    auto elem = parseAssignElement();
    if (check(TokenType::ASSIGN)) {
      stmt = parseAssignStmt(std::move(elem));
    } else {
      return parseExpr(elem);
    }
  } else if (check(TokenType::INT) || check(TokenType::LPAREN)) {
    return parseExpr();
  } else if (check(TokenType::IF)) {
    stmt = parseIfStmt();
  } else if (check(TokenType::WHILE)) {
    stmt = parseWhileStmt();
  } else if (check(TokenType::FOR)) {
    stmt = parseForStmt();
  } else if (check(TokenType::LOOP)) {
    stmt = parseLoopStmt();
  } else if (check(TokenType::BREAK)) {
    stmt = parseBreakStmt();
  } else if (check(TokenType::CONTINUE)) {
    stmt = std::make_shared<ast::ContinueStmt>();
    stmt->setPos(current.getPos());
    advance();
    expect(TokenType::SEMICOLON, "Expected ';' after Continue");
  } else if (check(TokenType::SEMICOLON)) {
    stmt = std::make_shared<ast::NullStmt>();
    stmt->setPos(current.getPos());
    advance();
  }

  return stmt;
}

/**
 * @brief  解析返回语句
 * @return ast::RetStmtPtr - AST Return Statement 结点指针
 */
ast::RetStmtPtr
Parser::parseRetStmt()
{
  util::Position pos = current.getPos();
  expect(TokenType::RETURN, "Expected 'return'");

  if (check(TokenType::SEMICOLON))
  {
    expect(TokenType::SEMICOLON, "Expected ';'");
    return std::make_shared<ast::RetStmt>(std::nullopt);
  }

  ast::ExprPtr ret = Parser::parseCmpExpr();
  expect(TokenType::SEMICOLON, "Expected ';'");

  auto p_rstmt = std::make_shared<ast::RetStmt>(std::move(ret));
  p_rstmt->setPos(pos);
  return p_rstmt;
}

/**
 * @brief  解析参数
 * @return ast::ArgPtr - AST Argument 结点指针
 */
ast::ArgPtr
Parser::parseArg()
{
  util::Position pos = current.getPos();

  bool mut = false;
  if (check(TokenType::MUT))
  {
    mut = true;
    advance();
  }
  std::string name = current.getValue();
  expect(TokenType::ID, "Expected '<ID>'");
  auto var = std::make_shared<ast::VarDeclBody>(mut, name);

  expect(TokenType::COLON, "Expected ':'");
  auto type = parseVarType();

  auto p_arg = std::make_shared<ast::Arg>(std::move(var), std::move(type));
  p_arg->setPos(pos);
  return p_arg;
}

/**
 * @brief  解析变量声明语句或变量声明赋值语句
 * @return ast::VarDeclStmtPtr - AST Variable Declaration Statement 结点指针
 */
ast::VarDeclStmtPtr
Parser::parseVarDeclStmt()
{
  expect(TokenType::LET, "Expected 'let'");

  bool mut = false;
  if (check(TokenType::MUT)) {
    mut = true;
    advance();
  }
  util::Position pos = current.getPos();

  auto identifier = std::make_shared<ast::VarDeclBody>(mut, current.getValue());
  expect(TokenType::ID, "Expected '<ID>'");

  ast::VarTypePtr type;
  bool has_type = false;
  if (check(TokenType::COLON)) {
    has_type = true;
    advance();
    type = parseVarType();
  }

  ast::ExprPtr expr{};
  bool flag_assign = false;
  if (check(TokenType::ASSIGN)) {
    flag_assign = true;
    advance();
    expr = parseExpr();
  }

  expect(TokenType::SEMICOLON, "Expected ';'");

  if (flag_assign) {
    return std::make_shared<ast::VarDeclAssignStmt>(
      std::move(identifier),
      (has_type ? std::optional<ast::VarTypePtr>{type} : std::nullopt),
      std::move(expr)
    );
  }

  auto p_vdstmt = std::make_shared<ast::VarDeclStmt>(
    std::move(identifier),
    (has_type ? std::optional<ast::VarTypePtr>{type} : std::nullopt)
  );
  p_vdstmt->setPos(pos);
  return p_vdstmt;
}

/**
 * @brief  解析赋值语句
 * @return ast::AssignStmtPtr - AST Assignment Statement 结点指针
 */
ast::AssignStmtPtr
Parser::parseAssignStmt(ast::AssignElementPtr &&lvalue)
{
  util::Position pos = current.getPos();
  expect(TokenType::ASSIGN, "Expected '='");
  ast::ExprPtr expr = Parser::parseExpr();

  expect(TokenType::SEMICOLON, "Expected ';'");

  auto p_astmt =
      std::make_shared<ast::AssignStmt>(std::move(lvalue), std::move(expr));
  p_astmt->setPos(pos);
  return p_astmt;
}

/**
 * @brief 解析可赋值元素
 * @return ast::AssignElementPtr - AST Assign Element 节点指针
 */
ast::AssignElementPtr
Parser::parseAssignElement()
{
  util::Position pos = current.getPos();
  if (check(TokenType::OP_MUL)) {
    advance();
    std::string var = current.getValue();
    expect(TokenType::ID, "Expected '<ID>'");

    auto p_deref = std::make_shared<ast::Dereference>(std::move(var));
    p_deref->setPos(pos);
    return p_deref;
  }

  std::string var = current.getValue();
  expect(TokenType::ID, "Expected '<ID>'");
  if (check(TokenType::LBRACK)) {
    advance();
    auto expr = parseExpr();
    expect(TokenType::RBRACK, "Expected ']'");

    auto p_aacc =
        std::make_shared<ast::ArrayAccess>(std::move(var), std::move(expr));
    p_aacc->setPos(pos);
    return p_aacc;
  }

  if (check(TokenType::DOT)) {
    advance();
    int value = std::stoi(current.getValue());
    expect(TokenType::INT, "Expected <NUM> for Tuple");

    auto p_tacc = std::make_shared<ast::TupleAccess>(std::move(var), value);
    p_tacc->setPos(pos);
    return p_tacc;
  }

  auto p_var = std::make_shared<ast::Variable>(std::move(var));
  p_var->setPos(pos);
  return p_var;
}

/**
 * @brief  解析表达式，用递归下降解析分层处理运算优先级
 * @return ast::ExprPtr - 顶层比较表达式
 */
ast::ExprPtr
Parser::parseExpr(std::optional<ast::AssignElementPtr> elem)
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

/**
 * @brief  将 token type 转换为 comparison operator
 * @param  t token type
 * @return comparison operator
 */
static ast::ComparOperator
tokenType2ComparOper(TokenType t)
{
  using CmpOper = ast::ComparOperator;
  static std::unordered_map<TokenType, CmpOper> map{
    {TokenType::OP_EQ,  CmpOper::Equal},
    {TokenType::OP_NEQ, CmpOper::Nequal},
    {TokenType::OP_GE,  CmpOper::Gequal},
    {TokenType::OP_GT,  CmpOper::Great},
    {TokenType::OP_LT,  CmpOper::Less}
  };

  auto res = map.find(t);
  if (res == map.end()) {
    throw std::runtime_error{"Incorrect token type."};
  }
  return res->second;
}

/**
 * @brief  将 token type 转换为 arithmetic operator
 * @param  t token type
 * @return arithmetic operator
 */
static ast::ArithOperator
tokenType2ArithOper(TokenType t)
{
  static std::unordered_map<TokenType, ast::ArithOperator> map {
    {TokenType::OP_PLUS,  ast::ArithOperator::Add},
    {TokenType::OP_MINUS, ast::ArithOperator::Sub},
    {TokenType::OP_MUL,   ast::ArithOperator::Mul},
    {TokenType::OP_DIV,   ast::ArithOperator::Div}};

  auto res = map.find(t);
  if (res == map.end()) {
    throw std::runtime_error{"Incorrect token type."};
  }
  return res->second;
}

/**
 * @brief  解析比较表达式（最顶层的表达式）
 * @return ast::ArithmeticExprPtr - AST Expression
 * 结点指针（若无，则为下一层的加法表达式）
 */
ast::ExprPtr
Parser::parseCmpExpr(std::optional<ast::AssignElementPtr> elem)
{
  ast::ExprPtr left = Parser::parseAddExpr(std::move(elem));
  while (
    check(TokenType::OP_LT) || check(TokenType::OP_LE) ||
    check(TokenType::OP_GT) || check(TokenType::OP_GE) ||
    check(TokenType::OP_EQ) || check(TokenType::OP_NEQ)
  ) {
    util::Position pos = current.getPos();

    TokenType op = current.getType();
    advance();

    ast::ExprPtr right = Parser::parseAddExpr();

    auto p_cmp = std::make_shared<ast::ComparExpr>(
      std::move(left),
      tokenType2ComparOper(op),
      std::move(right));
    p_cmp->setPos(pos);
    left = p_cmp;
  } // end while

  return left;
}

/**
 * @brief  解析加法表达式
 * @return ast::ArithmeticExprPtr - AST Expression
 * 结点指针（若无，则为下一层的乘法表达式）
 */
ast::ExprPtr
Parser::parseAddExpr(std::optional<ast::AssignElementPtr> elem)
{
  ast::ExprPtr left = Parser::parseMulExpr(std::move(elem));
  while (check(TokenType::OP_PLUS) || check(TokenType::OP_MINUS)) {
    util::Position pos = current.getPos();

    TokenType op = current.getType();
    advance();

    ast::ExprPtr right = Parser::parseMulExpr();

    auto p_ari = std::make_shared<ast::ArithExpr>(
      std::move(left),
      tokenType2ArithOper(op),
      std::move(right)
    );
    p_ari->setPos(pos);
    left = p_ari;
  } // end while

  return left;
}

/**
 * @brief  解析乘法表达式（即为Item）
 * @return ast::ArithmeticExprPtr - AST Expression
 * 结点指针（若无，则为下一层的因子）
 */
ast::ExprPtr
Parser::parseMulExpr(std::optional<ast::AssignElementPtr> elem)
{
  ast::ExprPtr left = parseFactor(std::move(elem));
  while (check(TokenType::OP_MUL) || check(TokenType::OP_DIV)) {
    util::Position pos = current.getPos();

    TokenType op = current.getType();
    advance();

    ast::ExprPtr right = Parser::parseFactor();

    auto p_ari = std::make_shared<ast::ArithExpr>(
      std::move(left),
      tokenType2ArithOper(op),
      std::move(right)
    );
    p_ari->setPos(pos);
    left = p_ari;
  } // end while

  return left;
}

/**
 * @brief  解析因子
 * @return ast::FactorPtr - AST Factor 结点指针
 */
ast::ExprPtr
Parser::parseFactor(std::optional<ast::AssignElementPtr> elem)
{
  util::Position pos = current.getPos();
  // ArrayElements
  if (check(TokenType::LBRACK)) {
    advance();
    std::vector<ast::ExprPtr> elements{};
    while (!check(TokenType::RBRACK)) {
      elements.push_back(parseExpr());
      if (!check(TokenType::COMMA)) {
        break;
      }
      advance();
    }
    advance();

    auto p_aelem = std::make_shared<ast::ArrayElements>(elements);
    p_aelem->setPos(pos);
    return p_aelem;
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
      auto p_par = std::make_shared<ast::ParenthesisExpr>(std::move(elems[0]));
      p_par->setPos(pos);
      return p_par;
    }
    auto p_telem = std::make_shared<ast::TupleElements>(elems);
    p_telem->setPos(pos);
    return p_telem;
  }

  ast::RefType ref_type{ast::RefType::Normal};
  if (check(TokenType::REF)) {
    advance();
    if (check(TokenType::MUT)) {
      advance();
      ref_type = ast::RefType::Mutable;
    } else {
      ref_type = ast::RefType::Immutable;
    }
  }

  auto element = parseElement(std::move(elem));

  auto p_factor = std::make_shared<ast::Factor>(ref_type, std::move(element));
  p_factor->setPos(pos);
  return p_factor;
}

/**
 * @brief  解析元素
 * @return ast::Number or ast::Variable or ast::ParenthesisExpr or
 *         ast::AssignElement or CallExpr
 */
ast::ExprPtr
Parser::parseElement(std::optional<ast::AssignElementPtr> elem)
{
  if (elem.has_value()) {
    return elem.value();
  }

  util::Position pos = current.getPos();
  if (check(TokenType::LPAREN)) {
    advance();
    ast::ExprPtr expr = Parser::parseCmpExpr();
    expect(TokenType::RPAREN, "Expected ')'");
    auto p_par = std::make_shared<ast::ParenthesisExpr>(std::move(expr));
    p_par->setPos(pos);
    return p_par;
  }
  if (check(TokenType::INT)) {
    int value = std::stoi(current.getValue());
    advance();
    auto p_num = std::make_shared<ast::Number>(value);
    p_num->setPos(pos);
    return p_num;
  }
  if (check(TokenType::ID)) {
    if (checkAhead(TokenType::LBRACK) || checkAhead(TokenType::DOT)) {
      return parseAssignElement();
    }
    if (checkAhead(TokenType::LPAREN)) {
      return parseCallExpr();
    }
    std::string name = current.getValue();
    advance();
    auto p_var = std::make_shared<ast::Variable>(std::move(name));
    p_var->setPos(pos);
    return p_var;
  }
  if (check(TokenType::OP_MUL) && checkAhead(TokenType::ID)) {
    return parseAssignElement();
  }
  throw std::runtime_error("Unexpected token in expression: " +
                           current.getValue());
}

/**
 * @brief  解析函数调用
 * @return ast::CallExpr - AST Expression 结点指针
 */
ast::CallExprPtr
Parser::parseCallExpr()
{
  util::Position pos = current.getPos();

  std::string name = current.getValue(); // function name

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
  auto p_cexpr =
      std::make_shared<ast::CallExpr>(std::move(name), std::move(argv));
  p_cexpr->setPos(pos);
  return p_cexpr;
}

/**
 * @brief  解析 if 语句
 * @return ast::IfStmtPtr - AST If Statement 结点指针
 */
ast::IfStmtPtr
Parser::parseIfStmt()
{
  util::Position pos = current.getPos();

  expect(TokenType::IF, "Expected 'if'");
  auto expr = parseCmpExpr();
  auto if_branch = parseBlockStmt();

  std::vector<ast::ElseClausePtr> else_clauses{};
  while (check(TokenType::ELSE)) {
    advance();
    bool end = !check(TokenType::IF);
    else_clauses.push_back(parseElseClause());
    if (end) {
      break;
    }
  }

  auto p_istmt = std::make_shared<ast::IfStmt>(
      std::move(expr), std::move(if_branch), std::move(else_clauses));
  p_istmt->setPos(pos);
  return p_istmt;
}

/**
 * @brief  解析 else/else if 语句
 * @return ast::ElseClausePtr - AST Else Statement 结点指针
 */
ast::ElseClausePtr
Parser::parseElseClause()
{
  util::Position pos = current.getPos();
  if (check(TokenType::IF)) {
    advance();
    auto expr = parseCmpExpr();
    auto block = parseBlockStmt();
    auto p_eclause =
        std::make_shared<ast::ElseClause>(std::move(expr), std::move(block));
    p_eclause->setPos(pos);
    return p_eclause;
  }
  auto block = parseBlockStmt();
  auto p_eclause =
      std::make_shared<ast::ElseClause>(std::nullopt, std::move(block));
  p_eclause->setPos(pos);
  return p_eclause;
}

/**
 * @brief  解析 while 语句
 * @return ast::WhileStmtPtr - AST While Statement 结点指针
 */
ast::WhileStmtPtr
Parser::parseWhileStmt()
{
  util::Position pos = current.getPos();
  expect(TokenType::WHILE, "Expected 'while'");

  auto expr = parseCmpExpr();
  auto block = parseBlockStmt();

  auto p_wstmt =
      std::make_shared<ast::WhileStmt>(std::move(expr), std::move(block));
  p_wstmt->setPos(pos);
  return p_wstmt;
}

/**
 * @brief  解析 for 语句
 * @return ast::ForStmtPtr - AST For Statement 结点指针
 */
ast::ForStmtPtr
Parser::parseForStmt()
{
  util::Position pos = current.getPos();
  expect(TokenType::FOR, "Expected 'for'");

  bool mut = false;
  if (check(TokenType::MUT)) {
    mut = true;
    advance();
  }
  expect(TokenType::ID, "Expected '<ID>'");
  ast::VarDeclBodyPtr var =
      std::make_shared<ast::VarDeclBody>(mut, current.getValue());

  expect(TokenType::IN, "Expected 'in'");

  auto expr1 = parseCmpExpr();
  expect(TokenType::DOTS, "Expected '..'");

  auto expr2 = parseCmpExpr();
  auto block = parseBlockStmt();

  auto p_fstmt = std::make_shared<ast::ForStmt>(
      std::move(var), std::move(expr1), std::move(expr2), std::move(block));
  p_fstmt->setPos(pos);
  return p_fstmt;
}

/**
 * @brief  解析 loop 语句
 * @return ast::LoopStmtPtr - AST Loop Statement 结点指针
 */
ast::LoopStmtPtr
Parser::parseLoopStmt()
{
  util::Position pos = current.getPos();
  expect(TokenType::LOOP, "Expected 'loop'");
  auto block = parseBlockStmt();

  auto p_lstmt = std::make_shared<ast::LoopStmt>(std::move(block));
  p_lstmt->setPos(pos);
  return p_lstmt;
}

/**
 * @brief  解析变量类型
 * @return ast::VarTypePtr - AST Variable Type 结点指针
 */
ast::VarTypePtr
Parser::parseVarType()
{
  util::Position pos = current.getPos();
  ast::RefType ref_type{ast::RefType::Normal};
  if (check(TokenType::REF)) {
    advance();
    if (check(TokenType::MUT)) {
      advance();
      ref_type = ast::RefType::Mutable;
    } else {
      ref_type = ast::RefType::Immutable;
    }
  }

  if (check(TokenType::LBRACK)) {
    advance();
    ast::VarTypePtr elem_type = parseVarType();
    expect(TokenType::SEMICOLON, "Expected ';' for Array");
    int cnt = std::stoi(current.getValue());
    expect(TokenType::INT, "Expected <NUM> for Array");
    expect(TokenType::RBRACK, "Expected ']' for Array");
    auto p_arr = std::make_shared<ast::Array>(cnt, elem_type, ref_type);
    p_arr->setPos(pos);
    return p_arr;
  }

  if (check(TokenType::LPAREN)) {
    advance();

    std::vector<ast::VarTypePtr> elem_types;
    while (!check(TokenType::RPAREN)) {
      elem_types.push_back(parseVarType());
      if (!check(TokenType::COMMA)) {
        break;
      }
      advance();
    }

    expect(TokenType::RPAREN, "Expected ')'");
    auto cnt = elem_types.size();
    if (0 == cnt) {
      throw std::runtime_error{"Incorrect variable type."};
    }
    if (1 == cnt) {
      return elem_types[0];
    }
    auto p_tup = std::make_shared<ast::Tuple>(std::move(elem_types), ref_type);
    p_tup->setPos(pos);
    return p_tup;
  }

  if (check(TokenType::I32)) {
    advance();
    auto p_int = std::make_shared<ast::Integer>(ref_type);
    p_int->setPos(pos);
    return p_int;
  }

  throw std::runtime_error{"Incorrect variable type"};
}

/**
 * @brief  解析函数表达式语句块
 * @return ast::FuncExprBlockStmtPtr - AST Function Expression Block Statements
 * 结点指针
 */
ast::FuncExprBlockStmtPtr
Parser::parseFuncExprBlockStmt()
{
  util::Position pos = current.getPos();
  expect(
    TokenType::LBRACE,
    "Expected '{' for function expression block statements"
  );

  std::vector<ast::StmtPtr> stmts{};
  ast::ExprPtr expr{};
  while (!check(TokenType::RBRACE)) {
    ast::NodePtr node = parseStmtOrExpr();
    if (auto stmt = std::dynamic_pointer_cast<ast::Stmt>(node)) {
      stmts.push_back(stmt);
      continue;
    }
    expr = std::dynamic_pointer_cast<ast::Expr>(node);
    if (nullptr != expr) {
      if (check(TokenType::SEMICOLON)) {
        advance();
        stmts.push_back(std::make_shared<ast::ExprStmt>(std::move(expr)));
        continue;
      }
      break;
    }
  }

  expect(TokenType::RBRACE, "Expected '}' for block");

  auto p_febstmt = std::make_shared<ast::FuncExprBlockStmt>(
    std::move(stmts),
    std::move(expr)
  );
  p_febstmt->setPos(pos);
  return p_febstmt;
}

/**
 * @brief  解析 if 表达式
 * @return ast::IfExprPtr - AST If Expression 结点指针
 */
ast::IfExprPtr
Parser::parseIfExpr()
{
  util::Position pos = current.getPos();
  expect(TokenType::IF, "Expected 'if' for If Expression");

  auto condition = parseExpr();
  auto if_branch = parseFuncExprBlockStmt();

  expect(TokenType::ELSE, "Expected 'else' for If expression");
  auto else_branch = parseFuncExprBlockStmt();

  auto p_iexpr = std::make_shared<ast::IfExpr>(
    std::move(condition),
    std::move(if_branch),
    std::move(else_branch)
  );
  p_iexpr->setPos(pos);
  return p_iexpr;
}

/**
 * @brief  解析 Break 表达式
 * @return ast::BreakStmtPtr - AST Break Statement 结点指针
 */
ast::BreakStmtPtr
Parser::parseBreakStmt()
{
  util::Position pos = current.getPos();
  expect(TokenType::BREAK, "Expected 'break' for break statement");

  if (!check(TokenType::SEMICOLON)) {
    auto expr = parseExpr();
    auto p_bstmt = std::make_shared<ast::BreakStmt>(std::move(expr));
    p_bstmt->setPos(pos);
    return p_bstmt;
  }

  auto p_bstmt = std::make_shared<ast::BreakStmt>();
  p_bstmt->setPos(pos);
  return p_bstmt;
}

/* member function definition */

} // namespace parser::base
