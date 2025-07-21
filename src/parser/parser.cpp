/**
 * @file parser.cpp
 * @brief Implementation of the Parser class.
 *
 * This file contains the implementation of the Parser class,
 * which is responsible for syntactic analysis (parsing)
 * of the input token stream produced by the lexer. The parser
 * constructs an Abstract Syntax Tree (AST) representing
 * the structure of the source program.
 *
 * The parser supports parsing of:
 *   - Programs consisting of function declarations
 *   - Function headers and bodies
 *   - Variable declarations (including mutability and type annotations)
 *   - Expressions (arithmetic, assignment, control flow, etc.)
 *   - Statements (empty, variable declaration, expression statements)
 *   - Control flow constructs (if, else, while, for, loop, break, continue, return)
 *   - Composite types (arrays, tuples)
 *   - Array and tuple element access
 *
 * The parser uses recursive descent parsing techniques
 * and provides error reporting for unexpected tokens and syntactic errors.
 *
 * Key classes and concepts:
 *   - Token: Represents a lexical token from the lexer.
 *   - TokenType: Enum for different types of tokens.
 *   - ast::*: Namespace containing AST node types.
 *   - builder: Responsible for building and annotating AST nodes.
 *   - reporter: Used for error reporting during parsing.
 *   - util::Position: Represents source code positions for error messages.
 *
 * The parser maintains lookahead tokens and supports context-sensitive parsing
 * for constructs such as function arguments, block expressions, and control flow.
 */
#include "panic.hpp"
#include "parser.hpp"

using namespace lex;

namespace par {

/**
 * @brief  获取下一个 token
 * @return next token
 */
Token
Parser::nextToken()
{
  // 检查了是否有值，所以可以直接返回一个 Token，
  // 而不是 std::optional<Token>
  if (auto token = lexer.nextToken(); token.has_value()) {
    return token.value();
  }
  // 如果识别到未知 token，则发生了词法分析错误，且需要立即终止
  err::terminate(reporter);
}

/**
 * @brief 向前扫描一个 token
 */
void
Parser::advance()
{
  // look ahead 是通过调用 next token
  // 获取下一个 token 实现的，因此需要检查 la 是否有值
  // 若有值直接调用 nextToken() 获取到的是下一个的下一个
  if (la.has_value()) {
    cur = la.value();
    la.reset(); // 清除 la 中的值
  } else {
    cur = nextToken();
  }
}

/**
 * @brief  按指定的 type 匹配一个 token
 * @param  type TokenType（指定的 token 类型）
 * @return 是否匹配成功
 */
bool
Parser::match(TokenType type)
{
  if (check(type)) { // 检查当前 token 是否是指定类型
    advance();
    return true;
  }
  return false;
}

/**
 * @brief  检查当前 token 是否为指定类型
 * @param  type TokenType（指定的 token 类型）
 * @return 是否是指定类型
 */
bool
Parser::check(TokenType type) const
{
  return cur.type == type;
}

/**
 * @brief  向前检查一个 token 是否为指定类型
 * @param  type TokenType（指定的 token 类型）
 * @return 是否是指定类型
 */
bool
Parser::checkAhead(TokenType type)
{
  // 向前看一个 token，如果 la 已经有值则无需获取！
  if (!la.has_value()) {
    la = nextToken();
  }
  return la.has_value() && la.value().type == type;
}

/**
 * @brief 按指定类型消耗一个 token
 * @param type TokenType（指定的 token 类型）
 * @param msg  错误信息（如果当前 token 不是指定的类型，则报错）
 */
void
Parser::consume(TokenType type, const std::string &msg)
{
  if (!match(type)) { // 并不会杀死进程
    reporter.report(
      err::ParErrType::UNEXPECT_TOKEN,
      msg,
      cur.pos,
      cur.value
    );
  }
}

/**
 * @brief  解析输入程序
 * @return 解析到的 ast::Prog 结点指针
 */
ast::ProgPtr
Parser::parseProgram()
{
  // Prog -> (FuncDecl)*

  // 循环解析函数声明
  std::vector<ast::DeclPtr> decls;
  while (check(TokenType::FN)) {
    decls.push_back(parseFuncDecl());
  }

  // 检查 Prog 的语义、拼接各函数的中间代码
  auto prog = std::make_shared<ast::Prog>(decls);
  builder.build(*prog);
  return prog;
}

/**
 * @brief  解析标识符
 * @return 解析到的标识符名和声明位置
 */
std::pair<std::string, util::Position>
Parser::parseID()
{
  std::string id = cur.value;
  util::Position declpos = cur.pos;
  consume(TokenType::ID, "Expect '<ID>'");
  return {id, declpos};
}

/**
 * @brief  解析变量声明内部
 * @return 解析到的 mutable & variable name
 */
std::tuple<bool, std::string, util::Position>
Parser::parseInnerVarDecl()
{
  // InnerVarDecl -> (mut)? <ID>

  // (mut)?
  bool varmutable = false;
  if (check(TokenType::MUT)) {
    varmutable = true;
    advance();
  }

  // <ID>
  auto [id, declpos] = parseID(); // 参数解包

  return {varmutable, id, declpos};
}

/**
 * @brief  解析函数声明
 * @return 解析到的 ast::FuncDecl 结点指针
 */
ast::FuncDeclPtr
Parser::parseFuncDecl()
{
  // FuncDecl -> FuncHeaderDecl BlockStmt

  util::Position declpos = cur.pos;
  auto header = parseFuncHeaderDecl();
  auto body   = parseStmtBlockExpr();

  auto funcdecl = std::make_shared<ast::FuncDecl>(header, body);
  funcdecl->pos = declpos;

  builder.build(*funcdecl);
  builder.ctx->exitScope();
  return funcdecl;
}

/**
 * @brief  解析函数头声明
 * @return 解析到的 ast::FuncHeaderDecl 结点指针
 */
ast::FuncHeaderDeclPtr
Parser::parseFuncHeaderDecl()
{
  // FuncHeaderDecl -> fn <ID> ( (arg)? (, arg)* ) (-> Type)?

  consume(TokenType::FN, "Expect 'fn'");

  auto [funcname, declpos] = parseID();

  builder.ctx->enterFunc(funcname, declpos); // 必须先进入作用域！！！

  consume(TokenType::LPAREN, "Expect '('");

  // (arg)? (, arg)*
  std::vector<ast::ArgPtr> argv{};
  while (!check(TokenType::RPAREN)) {
    argv.push_back(parseArg());

    if (check(TokenType::RPAREN)) {
      break;
    }

    util::Position comma_pos = cur.pos;
    consume(TokenType::COMMA, "Expect ','");
    if (check(TokenType::RPAREN)) {
      // (arg, ..., ) => 多了一个 ,
      reporter.report(
        err::ParErrType::UNEXPECT_TOKEN,
        "考虑删除这个 ','",
        comma_pos,
        ","
      );
      break;
    }
  }
  advance(); // consume ) !

  // (-> Type)?
  ast::Type rettype{type::TypeFactory::UNIT_TYPE};
  if (check(TokenType::ARROW)) {
    advance();
    rettype = parseType();
  }

  auto funcheaderdecl =
    std::make_shared<ast::FuncHeaderDecl>(funcname, argv, rettype);
  funcheaderdecl->pos = declpos;
  builder.build(*funcheaderdecl);
  return funcheaderdecl;
}

/**
 * @brief  解析参数
 * @return 解析到的 ast::Arg 结点指针
 */
ast::ArgPtr
Parser::parseArg()
{
  // arg -> (mut)? <ID> : Type
  auto [varmutable, id, declpos] = parseInnerVarDecl();

  consume(TokenType::COLON, "Expect ':'");

  auto vartype = parseType();

  auto arg = std::make_shared<ast::Arg>(varmutable, id, vartype);
  arg->pos = declpos;
  builder.build(*arg);
  return arg;
}

/**
 * @brief  解析类型
 * @return 解析到的 ast::Type 结点
 */
ast::Type
Parser::parseType()
{
  // Type -> i32
  //       | [ Type ; <NUM> ]
  //       | ( Type (, Type)* )

  ast::Type type;
  type.pos = cur.pos; // 避免在类型系统中引入 util::Position

  // i32
  if (check(TokenType::I32)) {
    advance();
    type.type = type::TypeFactory::INT_TYPE;
    return type;
  }

  // [ Type ; <NUM> ]
  if (check(TokenType::LBRACK)) {
    advance();
    auto elemtype = parseType().type;

    consume(TokenType::SEMICOLON, "Expect ';'");

    int elemcnt = 0;
    if (check(TokenType::INT)) {
      elemcnt = std::stoi(cur.value);
    }
    consume(TokenType::INT, "Expect <NUM>");
    consume(TokenType::RBRACK, "Expect ']'");

    type.type = builder.ctx->produceArrType(elemcnt, elemtype);
    return type;
  }

  // ( Type (, Type)* )
  if (check(TokenType::LPAREN)) {
    advance();

    bool is_tuple = false;
    std::vector<type::TypePtr> elemtypes;
    while (!check(TokenType::RPAREN)) {
      elemtypes.push_back(parseType().type);

      util::Position comma_pos = cur.pos;
      if (!check(TokenType::COMMA)) {
        break;
      }
      advance();
      is_tuple = true;

      if (elemtypes.size() > 1 && check(TokenType::RPAREN)) {
        reporter.report(
          err::ParErrType::UNEXPECT_TOKEN,
          "考虑删除这个 ','",
          comma_pos,
          ","
        );
      }
    }

    consume(TokenType::RPAREN, "Expect ')'");

    if (0 == elemtypes.size()) {
      // ()
      type.type = type::TypeFactory::UNIT_TYPE;
      return type;
    }
    if (is_tuple) {
      // ( Type , (Type (, Type)*)? )
      type.type = builder.ctx->produceTupType(elemtypes);
      return type;
    }
    // ( Type )
    type.type = elemtypes[0];
    return type;
  }

  //TODO: error report
  UNREACHABLE("unreachable point");
}

/**
 * @brief  解析语句块表达式
 * @return 解析到的 ast::StmtBlockExpr 结点指针
 */
ast::StmtBlockExprPtr
Parser::parseStmtBlockExpr()
{
  // StmtBlockExpr -> { (Stmt)* }

  util::Position declpos = cur.pos;
  consume(TokenType::LBRACE, "Expect '{'");

  // (stmt)*
  std::vector<ast::StmtPtr> stmts;
  while (!check(TokenType::RBRACE)) {
    stmts.push_back(parseStmt());
  }

  consume(TokenType::RBRACE, "Expect '}'");

  auto stmt_block = std::make_shared<ast::StmtBlockExpr>(stmts);
  stmt_block->pos = declpos;
  builder.build(*stmt_block);
  return stmt_block;
}

/**
 * @brief  解析语句
 * @return 解析到的 ast::Stmt 结点指针
 */
ast::StmtPtr
Parser::parseStmt()
{
  // Stmt -> EmptyStmt
  //       | VarDeclStmt
  //       | ExprStmt

  util::Position declpos = cur.pos;
  if (check(TokenType::SEMICOLON)) {
    // EmptyStmt -> ;
    advance();
    auto emptystmt = std::make_shared<ast::EmptyStmt>();
    emptystmt->pos = declpos;
    builder.build(*emptystmt);
    return emptystmt;
  }

  if (check(TokenType::LET)) {
    // VarDeclStmt -> let ...
    return parseVarDeclStmt();
  }

  // other case
  return parseExprStmt();
}

/**
 * @brief  解析变量声明语句
 * @return 解析到的 ast::VarDeclStmt 结点指针
 */
ast::VarDeclStmtPtr
Parser::parseVarDeclStmt()
{
  // VarDeclStmt -> let (mut)? <ID> (: Type)? (= Expr)? ;

  consume(TokenType::LET, "Expect 'let'");

  // (mut)? <ID>
  auto [varmutable, id, declpos] = parseInnerVarDecl();

  // (: Type)?
  ast::Type vartype{type::TypeFactory::UNKNOWN_TYPE};
  if (check(TokenType::COLON)) {
    advance();
    vartype = parseType();
  }

  // (= Expr)?
  std::optional<ast::ExprPtr> initval = std::nullopt;
  if (check(TokenType::ASSIGN)) {
    advance();
    initval = parseExpr();
  }

  consume(TokenType::SEMICOLON, "Expect ';'");

  auto vardeclstmt = std::make_shared<ast::VarDeclStmt>(
    varmutable, id, vartype, initval
  );
  vardeclstmt->pos = declpos;
  builder.build(*vardeclstmt);
  return vardeclstmt;
}

/**
 * @brief  解析表达式语句
 * @return 解析到的 ast::ExprStmt 结点指针
 */
ast::ExprStmtPtr
Parser::parseExprStmt()
{
  // ExprStmt -> Expr (;)?

  // 表达式语句贪婪匹配一个 ;
  // 若有则匹配，若没有则不匹配
  util::Position declpos = cur.pos;
  auto expr = parseExpr();
  auto expr_stmt = std::make_shared<ast::ExprStmt>(expr);

  // 一个表达式只有在两种情况下被用作表达式且需要设置
  // 1. 后面有一个 semicolon
  // 2. 位于语句块的最后
  // 其余情况参考上下文
  if (check(TokenType::SEMICOLON)) {
    expr->used_as_stmt = false;
    advance();
  } else {
    if (check(TokenType::RBRACE)) {
      expr_stmt->is_last = true;
      expr->used_as_stmt = false;
    } else {
      if (expr->is_ctlflow) {
        expr->used_as_stmt = true;
      } else {
        //TODO: expect ';'!!!
        std::println(stderr, "parseExprStmt(): Expect ';'");
        expr->used_as_stmt = false;
      }
    }
  }

  expr_stmt->pos = declpos;
  builder.build(*expr_stmt);
  return expr_stmt;
}

/**
 * @brief  解析表达式
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseExpr()
{
  // Expr -> RetExpr
  //       | BreakExpr
  //       | ContinueExpr
  //       | IfExpr
  //       | LoopExpr
  //       | WhileLoopExpr
  //       | ForLoopExpr
  //       | StmtBlockExpr
  //       | AssignExpr
  //       | CmpExpr

  switch (cur.type) {
    case TokenType::RETURN:   return parseRetExpr();
    case TokenType::BREAK:    return parseBreakExpr();
    case TokenType::CONTINUE: return parseContinueExpr();
    case TokenType::IF:       return parseIfExpr();
    case TokenType::WHILE:    return parseWhileLoopExpr();
    case TokenType::FOR:      return parseForLoopExpr();
    case TokenType::LOOP:     return parseLoopExpr();
    case TokenType::LBRACE: {
      // StmtBlocKExpr -> { (Stmt)* }
      builder.ctx->enterBlockExpr();
      auto expr = parseStmtBlockExpr();
      builder.ctx->exitScope();
      return expr;
    }
    case TokenType::ID: {
      if (checkAhead(TokenType::LPAREN)) {
        // CallExpr -> <ID> ( ...
        return parseCallExpr();
      }
      /*
       * x, x[idx], x.idx
       * 都即可以作为赋值语句的左值，又可以作为表达式的一个操作数
       */
      util::Position declpos = cur.pos;
      auto val = parseValue(); // x 会被简单的识别为一个 Variable！

      ast::AssignElemPtr assign_elem = nullptr;
      if (check(TokenType::LBRACK) || check(TokenType::DOT)) {
        assign_elem = parseAssignElem(val);
      }

      if (check(TokenType::ASSIGN)) {
        // AssignExpr -> AssignElem = Expr
        if (assign_elem == nullptr) {
          // 如果没有包装为一个赋值元素，则先包装
          assign_elem = std::make_shared<ast::AssignElem>(val);
          assign_elem->kind = ast::AssignElem::Kind::VARIABLE;
          assign_elem->pos = declpos;
          builder.build(*assign_elem);
        }
        return parseAssignExpr(std::move(assign_elem));
      }

      return parseCmpExpr(assign_elem == nullptr ? val : assign_elem);
    }
    case TokenType::INT:
    case TokenType::LPAREN:
    case TokenType::LBRACK:
      return parseCmpExpr();
    default:
      // TODO: unexpected token!
      UNREACHABLE("parseExpr()");
  }
}

/**
 * @brief  解析 return 表达式
 * @return 解析到的 ast::RetExpr 结点指针
 */
ast::RetExprPtr
Parser::parseRetExpr()
{
  // RetExpr -> return (Expr)?

  util::Position declpos = cur.pos;
  consume(TokenType::RETURN, "Expect 'return'");

  std::optional<ast::ExprPtr> retval = std::nullopt;
  if (!check(TokenType::SEMICOLON)) {
    retval = parseExpr();
  }

  auto retexpr = std::make_shared<ast::RetExpr>(retval);
  retexpr->pos = declpos;
  builder.build(*retexpr);
  return retexpr;
}

/**
 * @brief  解析 break 表达式
 * @return 解析到的 ast::BreakExpr 结点指针
 */
ast::BreakExprPtr
Parser::parseBreakExpr()
{
  // BreakExpr -> break (Expr)?

  util::Position declpos = cur.pos;
  consume(TokenType::BREAK, "Expect 'break'");

  std::optional<ast::ExprPtr> retval = std::nullopt;
  if (!check(TokenType::RBRACE) && !check(TokenType::SEMICOLON)) {
    retval = parseExpr();
  }

  auto breakexpr = std::make_shared<ast::BreakExpr>(retval);
  breakexpr->pos = declpos;
  builder.build(*breakexpr);
  return breakexpr;
}

/**
 * @brief  解析 continue 表达式
 * @return 解析到的 ast::ContinueExpr 结点指针
 */
ast::ContinueExprPtr
Parser::parseContinueExpr()
{
  // ContinueExpr -> continue

  util::Position declpos = cur.pos;
  consume(TokenType::CONTINUE, "Expect 'continue'");

  auto contexpr = std::make_shared<ast::ContinueExpr>();
  contexpr->pos = declpos;
  builder.build(*contexpr);
  return contexpr;
}

/**
 * @brief  解析赋值表达式
 * @return 解析到的 ast::AssignExpr 结点指针
 */
ast::AssignExprPtr
Parser::parseAssignExpr(ast::AssignElemPtr &&lval)
{
  // AssignElem = Expr

  util::Position declpos = cur.pos;
  consume(TokenType::ASSIGN, "Expect '='");

  auto rval = parseExpr();

  auto assign_expr = std::make_shared<ast::AssignExpr>(lval, rval);
  assign_expr->pos = declpos;
  builder.build(*assign_expr);
  return assign_expr;
}

/**
 * @brief  解析赋值元素
 * @return 解析到的 ast::AssignElem 结点指针
 */
ast::AssignElemPtr
Parser::parseAssignElem(std::optional<ast::ExprPtr> val)
{
  // AssignElem -> Variable
  //             | ArrAcc
  //             | TupAcc

  // NOTE: 不应该解析到变量作为赋值元素的情况
  //       这一情况在 parseExpr 解析了！
  if (check(TokenType::ID) && checkAhead(TokenType::ASSIGN)) {
    UNREACHABLE("Variables should not be parsed as assignment elements here");
  }

  ast::ExprPtr value = val.has_value() ? val.value() : parseValue();

  // ArrAcc -> Value [ Expr ]
  //         | ArrAcc [ Expr ]
  //         | TupAcc [ Expr ]
  //
  // TupAcc -> Value . <NUM>
  //         | TupAcc . <NUM>
  //         | ArrAcc . <NUM>
  //
  // 通过检查下一个 token 是 [ 还是 . 调用相应的 parse 函数
  // 可赋值元素必须要先识别一个 ArrAcc 或 TupAcc
  ast::AssignElemPtr aelem;

  if (check(TokenType::LBRACK)) {
    aelem = parseArrAcc(value);
  } else if (check(TokenType::DOT)) {
    aelem = parseTupAcc(value);
  } else {
    UNREACHABLE("unreachable point");
  }

  while (check(TokenType::LBRACK) || check(TokenType::DOT)) {
    if (check(TokenType::LBRACK)) {
      aelem = parseArrAcc(aelem);
    } else if (check(TokenType::DOT)) {
      aelem = parseTupAcc(aelem);
    } else {
      UNREACHABLE("unreachable point");
    }
  }

  return aelem;
}

/**
 * @brief  解析数组访问
 * @return 解析到的 ast::ArrAcc 结点指针
 */
ast::ArrAccPtr
Parser::parseArrAcc(ast::ExprPtr val)
{
  // ArrAcc -> Value [ Expr ]

  util::Position declpos = cur.pos;
  consume(TokenType::LBRACK, "Expect '['");
  auto idx = parseExpr();
  consume(TokenType::RBRACK, "Expect ']'");

  auto arr_acc = std::make_shared<ast::ArrAcc>(val, idx);
  arr_acc->kind = ast::AssignElem::Kind::ARRACC;
  arr_acc->pos = declpos;
  builder.build(*arr_acc);
  return arr_acc;
}

/**
 * @brief  解析元组访问
 * @return 解析到的 ast::TupAcc 结点指针
 */
ast::TupAccPtr
Parser::parseTupAcc(ast::ExprPtr val)
{
  // TupAcc -> Value . Number

  util::Position pos = cur.pos;
  consume(TokenType::DOT, "Expect '.'");

  // Number -> <NUM>
  ast::NumberPtr idx = nullptr;
  util::Position idxpos = cur.pos;
  if (check(TokenType::INT)) {
    idx = std::make_shared<ast::Number>(std::stoi(cur.value));
    idx->pos = idxpos;
    builder.build(*idx);
  } else {
    // TODO: Error!
  }
  consume(TokenType::INT, "Expect <NUM>");

  auto tacc = std::make_shared<ast::TupAcc>(val, idx);
  tacc->kind = ast::AssignElem::Kind::TUPACC;
  tacc->pos = pos;
  builder.build(*tacc);
  return tacc;
}

/**
 * @brief  解析 Value
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseValue()
{
  // Value -> BracketExpr
  //        | CallExpr
  //        | Variable

  util::Position pos = cur.pos;

  // BracketExpr -> ( Expr )
  if (check(TokenType::LPAREN)) {
    advance();
    auto expr = parseExpr();
    consume(TokenType::RPAREN, "Expect ')'");
    auto bexpr = std::make_shared<ast::BracketExpr>(expr);
    bexpr->pos = pos;
    builder.build(*bexpr);
    return bexpr;
  }

  // CallExpr -> <ID> ( ...
  if (check(TokenType::ID) && checkAhead(TokenType::LPAREN)) {
    return parseCallExpr();
  }

  // Variable -> <ID>
  std::string name = cur.value;
  consume(TokenType::ID, "Expect '<ID>");
  auto var = std::make_shared<ast::Variable>(name);
  var->pos = pos;
  builder.build(*var);
  return var;
}

/**
 * @brief  token type 转 compare operator
 * @param  type 待转换的 token type
 * @return 转换得到的 compare operator
 */
static ast::CmpOper
tokenType2CmpOper(TokenType type)
{
  // 使用 switch 加速分发过程
  switch (type) {
    case TokenType::EQ:  return ast::CmpOper::EQ;
    case TokenType::NEQ: return ast::CmpOper::NEQ;
    case TokenType::GEQ: return ast::CmpOper::GEQ;
    case TokenType::LEQ: return ast::CmpOper::LEQ;
    case TokenType::GT:  return ast::CmpOper::GT;
    case TokenType::LT:  return ast::CmpOper::LT;
    default:
  }

  UNREACHABLE("unreachable point");
}

/**
 * @brief  token type 转 arithmetic operator
 * @param  type 待转换的 token type
 * @return 转换得到的 arithmetic operator
 */
static ast::AriOper
tokenType2AriOper(TokenType type)
{
  // 使用 switch 加速分发过程
  switch (type) {
    case TokenType::PLUS:  return ast::AriOper::ADD;
    case TokenType::MINUS: return ast::AriOper::SUB;
    case TokenType::MUL:   return ast::AriOper::MUL;
    case TokenType::DIV:   return ast::AriOper::DIV;
    default:
  }

  UNREACHABLE("unreachable point");
}

/**
 * @brief  解析 compare 表达式
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseCmpExpr(std::optional<ast::ExprPtr> expr)
{
  // CmpExpr -> (CmpExpr CmpOper)* AddExpr

  ast::ExprPtr lhs = parseAddExpr(std::move(expr));
  while (check(TokenType::LT) || check(TokenType::LEQ)
      || check(TokenType::GT) || check(TokenType::GEQ)
      || check(TokenType::EQ) || check(TokenType::NEQ)
  ) {
    util::Position pos = cur.pos;

    TokenType op = cur.type;
    advance();

    ast::ExprPtr rhs = parseAddExpr();

    auto cexpr = std::make_shared<ast::CmpExpr>(
      lhs, tokenType2CmpOper(op), rhs // 注意结点挂载位置！！！
    );
    cexpr->pos = pos;
    builder.build(*cexpr);
    lhs = cexpr;
  }

  return lhs;
}

/**
 * @brief  解析 add & sub expression
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseAddExpr(std::optional<ast::ExprPtr> expr)
{
  // AddExpr -> (AddExpr [+ | -])* MulExpr

  ast::ExprPtr lhs = Parser::parseMulExpr(std::move(expr));
  while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
    util::Position pos = cur.pos;

    TokenType op = cur.type;
    advance();

    ast::ExprPtr rhs = parseMulExpr();

    auto aexpr = std::make_shared<ast::AriExpr>(
      lhs, tokenType2AriOper(op), rhs // 注意结点挂载位置！！！
    );
    aexpr->pos = pos;
    builder.build(*aexpr);
    lhs = aexpr;
  }

  return lhs;
}

/**
 * @brief  解析 mul & div expression
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseMulExpr(std::optional<ast::ExprPtr> expr)
{
  // MulExpr -> (MulExpr [* | /])* Factor

  ast::ExprPtr lhs = parseFactor(std::move(expr));
  while (check(TokenType::MUL) || check(TokenType::DIV)) {
    util::Position pos = cur.pos;

    TokenType op = cur.type;
    advance();

    ast::ExprPtr rhs = parseFactor();

    auto aexpr = std::make_shared<ast::AriExpr>(
      lhs, tokenType2AriOper(op), rhs // 注意结点挂载位置！！！
    );
    aexpr->pos = pos;
    builder.build(*aexpr);
    lhs = aexpr;
  } // end while

  return lhs;
}

/**
 * @brief  解析因子
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseFactor(std::optional<ast::ExprPtr> expr)
{
  // Factor -> ArrayElems
  //         | TupleElems
  //         | Elements
  // NOTE: 对上述产生式进行了一定的修正，
  //       补充了 ArrayElems 和 TupleElems 的元素访问

  // ArrayElems -> [ (Expr)? (, Expr)* ]
  if (check(TokenType::LBRACK)) {
    auto aelems = parseArrElems();

    // NOTE: 可能存在 [1, 2, 3][1] 的情况，因此需要递归的解析对 ArrayElems 的访问！
    if (check(TokenType::LBRACK) || check(TokenType::DOT)) {
      return parseAssignElem(aelems);
    }

    return aelems;
  }

  // TupleElems -> ( (Expr , TupleElem)? )
  // TupleElem -> epsilon | Expr (, Expr)*
  if (check(TokenType::LPAREN)) {
    auto telems = parseTupElems();

    // NOTE: 可能存在 (1, 2, 3).0 的情况，因此需要递归的解析对 TupleElems 的访问！
    if (check(TokenType::LBRACK) || check(TokenType::DOT)) {
      return parseAssignElem(telems);
    }

    return telems;
  }

  return parseElement(std::move(expr));
}

/**
 * @brief  解析数组元素
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseArrElems()
{
  // ArrayElems -> [ Expr (, Expr)* ]

  util::Position pos = cur.pos;
  consume(TokenType::LBRACK, "Expect '['");

  std::vector<ast::ExprPtr> elems{};
  if (!check(TokenType::RBRACK)) {
    elems.push_back(parseExpr());
  }
  while (!check(TokenType::RBRACK)) {
    util::Position comma_pos = cur.pos;
    consume(TokenType::COMMA, "Expect ','");
    if (check(TokenType::RBRACK)) {
      reporter.report(
        err::ParErrType::UNEXPECT_TOKEN,
        "考虑删除这个 ','",
        comma_pos,
        ","
      );
      break;
    }
    elems.push_back(parseExpr());
  } // end while
  advance();

  auto aelems = std::make_shared<ast::ArrElems>(elems);
  aelems->pos = pos;
  builder.build(*aelems);

  return aelems;
}

/**
 * @brief  解析元组元素
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseTupElems()
{
  // TupleElems -> ( (Expr , TupleElem)? )
  // TupleElem -> epsilon | Expr (, Expr)*

  util::Position pos = cur.pos;
  consume(TokenType::LPAREN, "Expect '('");

  bool is_tuple_elem = false;
  std::vector<ast::ExprPtr> elems{};

  while (!check(TokenType::RPAREN)) {
    elems.push_back(parseExpr());

    util::Position comma_pos = cur.pos;
    if (!check(TokenType::COMMA)) {
      break;
    }
    advance();

    is_tuple_elem = true;

    if (elems.size() > 1 && check(TokenType::RPAREN)) {
      reporter.report(
        err::ParErrType::UNEXPECT_TOKEN,
        "考虑删除这个 ','",
        comma_pos,
        ","
      );
    } // end if
  } // end while

  consume(TokenType::RPAREN, "Expect ')'");
  auto cnt = elems.size();

  if (0 == cnt) {
    auto bexpr = std::make_shared<ast::BracketExpr>(std::nullopt);
    bexpr->pos = pos;
    builder.build(*bexpr);

    if (check(TokenType::LBRACK) || check(TokenType::DOT)) {
      return parseAssignElem(bexpr);
    }
    return bexpr;
  } // end if

  if (!is_tuple_elem) {
    // 单个表达式没有逗号不是元组，而是普通括号表达式
    auto bexpr = std::make_shared<ast::BracketExpr>(elems[0]);
    bexpr->pos = pos;
    builder.build(*bexpr);

    if (check(TokenType::LBRACK) || check(TokenType::DOT)) {
      return parseAssignElem(bexpr);
    }

    return bexpr;
  } // end if

  auto telems = std::make_shared<ast::TupElems>(elems);
  telems->pos = pos;
  builder.build(*telems);
  return telems;
}

/**
 * @brief  解析元素
 * @return 解析到的 ast::Expr 结点指针
 */
ast::ExprPtr
Parser::parseElement(std::optional<ast::ExprPtr> expr)
{
  // Element -> Number
  //          | Value
  //          | AssignElem

  if (expr.has_value()) {
    return expr.value();
  }

  // Number -> <NUM>
  util::Position pos = cur.pos;
  std::string value = cur.value;
  if (check(TokenType::INT)) {
    advance();
    auto num = std::make_shared<ast::Number>(std::stoi(value));
    num->pos = pos;
    builder.build(*num);
    return num;
  }

  // Value -> BracketExpr | CallExpr | Variable
  // AssignElem -> Variable | ArrAcc | TupAcc
  //   ArrAcc -> Value [ Expr ]
  //   TupAcc -> Value . <NUM>
  auto val = parseValue(); // 在表达式中 Variable 被识别为一个 Value 而不是一个 AssignElem
  if (check(TokenType::LBRACK) || check(TokenType::DOT)) {
    return parseAssignElem(val);
  }

  return val;
}

ast::CallExprPtr
Parser::parseCallExpr()
{
  // CallExpr -> <ID> ( ArgList )

  auto [name, pos] = parseID();

  consume(TokenType::LPAREN, "Expect '('");

  // ArgList -> Expr (, Expr)* | epsilon
  std::vector<ast::ExprPtr> argv{};
  while (!check(TokenType::RPAREN)) {
    argv.push_back(parseExpr());

    if (check(TokenType::RPAREN)) {
      break;
    }

    util::Position comma_pos = cur.pos;
    consume(TokenType::COMMA, "Expect ','");

    if (check(TokenType::RPAREN)) {
      reporter.report(
        err::ParErrType::UNEXPECT_TOKEN,
        "考虑删除这个 ','",
        comma_pos,
        ","
      );
    }
  }
  consume(TokenType::RPAREN, "Expect ')'");

  auto cexpr = std::make_shared<ast::CallExpr>(name, argv);
  cexpr->pos = pos;
  builder.build(*cexpr);
  return cexpr;
}

ast::IfExprPtr
Parser::parseIfExpr()
{
  // IfExpr -> if Expr StmtBlockExpr ElseClause

  util::Position pos = cur.pos;
  consume(TokenType::IF, "Expect 'if'");

  ast::ExprPtr cond = parseExpr();
  ast::StmtBlockExprPtr body;
  sym::TempPtr temp_val = nullptr;
  if (!check(TokenType::LBRACE)) {
    // TODO: 缺少一个 body!
    std::println(stderr, "缺少语句块，如果这是语句块，考虑在前面添加一个判断条件");

    std::vector<ast::StmtPtr> stmts{};
    body = std::make_shared<ast::StmtBlockExpr>(stmts);
  } else {
    builder.ctx->enterIf();
    body = parseStmtBlockExpr();

    if (body->type.type != type::TypeFactory::UNIT_TYPE) {
      temp_val = builder.ctx->produceTemp(pos, body->type.type);
      builder.ctx->setCurCtxSymbol(temp_val);
    }

    builder.ctx->exitSymtabScope();
  }

  // ElseClause -> else if Expr StmtBlockExpr ElseClause
  //             | else StmtBlockExpr
  //             | epsilon
  std::vector<ast::ElseClausePtr> elses{};
  while (check(TokenType::ELSE)) {
    bool end = !checkAhead(TokenType::IF);
    elses.push_back(parseElseClause());
    if (end) {
      break;
    }
  }

  auto iexpr = std::make_shared<ast::IfExpr>(cond, body, elses);
  iexpr->is_ctlflow = true;
  if (temp_val != nullptr) {
    iexpr->symbol = temp_val;
  }
  iexpr->pos = pos;
  builder.build(*iexpr);

  builder.ctx->exitCtxScope();

  return iexpr;
}

ast::ElseClausePtr
Parser::parseElseClause()
{
  // ElseClause -> else (if Expr)? StmtBlockExpr

  util::Position pos = cur.pos;
  consume(TokenType::ELSE, "Expect 'else'");

  // (if Expr)?
  std::optional<ast::ExprPtr> cond = std::nullopt;
  ast::StmtBlockExprPtr body;
  if (check(TokenType::IF)) {
    advance();
    cond = parseExpr();

    if (!check(TokenType::LBRACE)) {
      // TODO: 缺少一个 body!
      std::println(stderr, "缺少语句块，如果这是语句块，考虑在前面添加一个判断条件");

      std::vector<ast::StmtPtr> stmts{};
      body = std::make_shared<ast::StmtBlockExpr>(stmts);
    } else {
      builder.ctx->enterElse();
      body = parseStmtBlockExpr();
      builder.ctx->exitSymtabScope();
    }
  } else {
    // StmtBlockExpr
    builder.ctx->enterElse();
    body = parseStmtBlockExpr();
    builder.ctx->exitSymtabScope();
  }

  auto else_clause = std::make_shared<ast::ElseClause>(cond, body);
  else_clause->pos = pos;
  builder.build(*else_clause);
  builder.ctx->exitCtxScope();
  return else_clause;
}

ast::WhileLoopExprPtr
Parser::parseWhileLoopExpr()
{
  // WhileLoopExpr -> while Expr StmtBlockExpr

  util::Position pos = cur.pos;
  consume(TokenType::WHILE, "Expect 'while'");

  ast::ExprPtr cond = parseExpr();

  builder.ctx->enterWhile();

  ast::StmtBlockExprPtr body;
  if (!check(TokenType::LBRACE)) {
    // TODO: 缺少一个 body!
    std::println(stderr, "缺少语句块，如果这是语句块，考虑在前面添加一个判断条件");

    std::vector<ast::StmtPtr> stmts{};
    body = std::make_shared<ast::StmtBlockExpr>(stmts);
  } else {
    body = parseStmtBlockExpr();
  }

  auto while_loop = std::make_shared<ast::WhileLoopExpr>(cond, body);
  while_loop->is_ctlflow = true;
  while_loop->pos = pos;
  builder.build(*while_loop);
  builder.ctx->exitScope();
  return while_loop;
}

ast::ForLoopExprPtr
Parser::parseForLoopExpr()
{
  // ForLoopExpr -> for (mut)? <ID> in Iterable StmtBlockExpr

  util::Position declpos = cur.pos;
  consume(TokenType::FOR, "Expect 'for'");

  auto [mut, name, varpos] = parseInnerVarDecl();

  builder.ctx->enterFor();
  // 这里简单的将迭代器的类型认为是 i32
  // 实际类型应该由可迭代对象的元素的类型确定
  auto var = builder.ctx->declareVar(
    name, mut, true, type::TypeFactory::INT_TYPE, varpos
  );
  builder.ctx->setCurCtxSymbol(var);

  consume(TokenType::IN, "Expect 'in'");

  ast::ExprPtr iterexpr = parseIterable();
  ast::StmtBlockExprPtr body;
  if (!check(TokenType::LBRACE)) {
    // TODO: 缺少一个 body!
    std::println(stderr, "缺少语句块，如果这是语句块，考虑在前面添加一个可迭代对象");

    std::vector<ast::StmtPtr> stmts{};
    body = std::make_shared<ast::StmtBlockExpr>(stmts);
  } else {
    body = parseStmtBlockExpr();
  }

  auto for_loop =
    std::make_shared<ast::ForLoopExpr>(mut, name, iterexpr, body);
  for_loop->is_ctlflow = true;
  for_loop->pos = declpos;

  builder.build(*for_loop);
  builder.ctx->exitScope();
  return for_loop;
}

ast::ExprPtr
Parser::parseIterable()
{
  // Iterable -> RangeExpr
  //           | IterableVal

  util::Position declpos = cur.pos;

  // RangeExpr -> Expr .. Expr
  // IterableVal -> Expr
  auto expr1 = parseExpr();
  if (check(TokenType::DOTS)) {
    advance();
    auto expr2 = parseExpr();
    auto range_expr = std::make_shared<ast::RangeExpr>(expr1, expr2);
    range_expr->pos = declpos;
    builder.build(*range_expr);
    return range_expr;
  }

  auto iterval = std::make_shared<ast::IterableVal>(expr1);
  iterval->pos = declpos;
  builder.build(*iterval);
  return iterval;
}

ast::LoopExprPtr
Parser::parseLoopExpr()
{
  // LoopExpr -> loop StmtBlockExpr

  util::Position declpos = cur.pos;
  consume(TokenType::LOOP, "Expect 'loop'");

  builder.ctx->enterLoop();
  auto body = parseStmtBlockExpr();

  auto loopexpr = std::make_shared<ast::LoopExpr>(body);
  loopexpr->is_ctlflow = true;
  loopexpr->pos = declpos;
  builder.build(*loopexpr);
  builder.ctx->exitScope();
  return loopexpr;
}

} // namespace par
