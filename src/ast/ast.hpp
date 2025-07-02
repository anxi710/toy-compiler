#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "type.hpp"
#include "position.hpp"

namespace visit {

class NodeVisitor;

} // namespace visit

namespace ast {

// 所有 AST 结点的基类
struct Node {
  util::Position pos{0, 0};

  Node() = default;
  Node(util::Position pos) : pos(pos) {}
  virtual ~Node() = default;
  virtual void accept(visit::NodeVisitor &visitor) = 0;
};
using NodePtr = std::shared_ptr<Node>;

// Declaration
struct Decl : virtual Node {
  Decl() = default;
  Decl(util::Position pos) : Node(pos) {}
  ~Decl() override = default;
  void accept(visit::NodeVisitor &visitor) override = 0;
};
using DeclPtr = std::shared_ptr<Decl>;

// Program
struct Prog : virtual Node {
  std::vector<DeclPtr> decls; // declarations

  Prog(std::vector<DeclPtr> decls) : decls(std::move(decls)) {}
  ~Prog() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using ProgPtr = std::shared_ptr<Prog>;

// Argument
struct Arg : virtual Node {
  bool          mut;
  std::string   name;
  type::TypePtr type;

  Arg(util::Position pos, bool mut,
    std::string name, type::TypePtr type
  ) : Node(pos), mut(mut), name(std::move(name)),
      type(std::move(type)) {}
  ~Arg() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using ArgPtr = std::shared_ptr<Arg>;

// Function header declaration
struct FuncHeaderDecl : Decl {
  std::string name; // function name
  std::vector<ArgPtr> argv; // argument vector
  std::optional<type::TypePtr> retval_type; // return value type

  FuncHeaderDecl(util::Position pos, std::string name,
    std::vector<ArgPtr> argv, std::optional<type::TypePtr> retval_type
  ) : Decl(pos), name(std::move(name)), argv(std::move(argv)),
      retval_type(std::move(retval_type)) {};
  ~FuncHeaderDecl() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using FuncHeaderDeclPtr = std::shared_ptr<FuncHeaderDecl>;

// Statement
struct Stmt : virtual Node {
  Stmt() = default;
  Stmt(util::Position pos) : Node(pos) {}
  ~Stmt() override = default;
  void accept(visit::NodeVisitor &visitor) override = 0;
};
using StmtPtr = std::shared_ptr<Stmt>;

// Block statement
struct BlockStmt : Stmt {
  std::vector<StmtPtr> stmts; // statements

  BlockStmt(std::vector<StmtPtr> stmts)
    : stmts(std::move(stmts)) {}
  ~BlockStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using BlockStmtPtr = std::shared_ptr<BlockStmt>;

// Function Declaration
struct FuncDecl : Decl {
  FuncHeaderDeclPtr header; // function header
  BlockStmtPtr      body;   // function body

  FuncDecl(FuncHeaderDeclPtr fhdecl, BlockStmtPtr bstmt)
    : header(std::move(fhdecl)), body(std::move(bstmt)) {}
  ~FuncDecl() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using FuncDeclPtr = std::shared_ptr<FuncDecl>;

// Expression
struct Expr : virtual Node {
  type::TypePtr type; // value type

  Expr() = default;
  Expr(util::Position pos) : Node(pos) {}
  ~Expr() override = default;
  void accept(visit::NodeVisitor& visitor) override = 0;
};
using ExprPtr = std::shared_ptr<Expr>;

struct StmtOrExpr {
  std::optional<StmtPtr> stmt = std::nullopt;
  std::optional<ExprPtr> expr = std::nullopt;
};
using StmtOrExprPtr = std::shared_ptr<StmtOrExpr>;

// Expression Statement
struct ExprStmt : Stmt {
  ExprPtr expr;

  ExprStmt(ExprPtr expr) : expr(std::move(expr)) {}
  ~ExprStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using ExprStmtPtr = std::shared_ptr<ExprStmt>;

// 括号表达式
struct BracketExpr : Expr {
  ExprPtr expr; // ( expr ) - 括号中的表达式

  BracketExpr(util::Position pos, ExprPtr expr)
    : Expr(pos), expr(std::move(expr)) {}
  ~BracketExpr() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using BracketExprPtr = std::shared_ptr<BracketExpr>;

// Assign Element
struct AssignElem : Expr {
  AssignElem(util::Position pos) : Expr(pos) {}
  ~AssignElem() override = default;
  void accept(visit::NodeVisitor& visitor) override = 0;
};
using AssignElemPtr = std::shared_ptr<AssignElem>;

struct Variable : AssignElem {
  std::string name; // 变量名

  Variable(util::Position pos, std::string name)
    : AssignElem(pos), name(std::move(name)) {}
  ~Variable() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using VariablePtr = std::shared_ptr<Variable>;

struct ArrayAccess : AssignElem {
  std::string name; // 数组名
  ExprPtr     idx; // 索引值

  ArrayAccess(util::Position pos, std::string name, ExprPtr idx)
    : AssignElem(pos), name(std::move(name)), idx(std::move(idx)) {}
  ~ArrayAccess() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using ArrayAccessPtr = std::shared_ptr<ArrayAccess>;

struct TupleAccess : AssignElem {
  std::string name; // 元组名
  int         idx;  // 索引值

  TupleAccess(util::Position pos, std::string name, int idx)
    : AssignElem(pos), name(std::move(name)), idx(idx) {}
  ~TupleAccess() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using TupleAccessPtr = std::shared_ptr<TupleAccess>;

struct Number : Expr {
  int value; // 值

  Number(util::Position pos, int value)
    : Expr(pos), value(value) {}
  ~Number() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using NumberPtr = std::shared_ptr<Number>;

struct Factor : Expr {
  ExprPtr elem;

  Factor(util::Position pos, ExprPtr expr)
    : Expr(pos), elem(std::move(expr)) {}
  ~Factor() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using FactorPtr = std::shared_ptr<Factor>;

struct ArrayElems : Expr {
  std::vector<ExprPtr> elems;

  ArrayElems(util::Position pos, std::vector<ExprPtr> elems)
    : Expr(pos), elems(std::move(elems)) {}
  ~ArrayElems() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using ArrayElemsPtr = std::shared_ptr<ArrayElems>;

struct TupleElems : Expr {
  std::vector<ExprPtr> elems;

  TupleElems(util::Position pos, std::vector<ExprPtr> elems)
    : Expr(pos), elems(std::move(elems)) {}
  ~TupleElems() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using TupleElemsPtr = std::shared_ptr<TupleElems>;

// Return Statement
struct RetStmt : Stmt {
  std::optional<ExprPtr> retval; // return value (an expression)

  RetStmt(util::Position pos, std::optional<ExprPtr> retval)
    : Node(pos), retval(std::move(retval)) {}
  ~RetStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using RetStmtPtr = std::shared_ptr<RetStmt>;

// Variable Declaration Statement
struct VarDeclStmt : Stmt, Decl {
  bool mut;
  std::string name;
  std::optional<type::TypePtr> type; // variable type
  std::optional<ExprPtr> value;

  VarDeclStmt(util::Position pos, bool mut, std::string name,
    std::optional<type::TypePtr> type, std::optional<ExprPtr> expr
  ) : Node(pos), mut(mut), name(std::move(name)),
      type(std::move(type)), value(std::move(expr)) {}
  ~VarDeclStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using VarDeclStmtPtr = std::shared_ptr<VarDeclStmt>;

// Assign Statement
struct AssignStmt : Stmt {
  AssignElemPtr lvalue;
  ExprPtr       expr; // expression

  AssignStmt(util::Position pos, AssignElemPtr lvalue, ExprPtr expr)
    : Stmt(pos), lvalue(std::move(lvalue)), expr(std::move(expr)) {}
  ~AssignStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using AssignStmtPtr = std::shared_ptr<AssignStmt>;

// Comparison Operator
enum class CmpOper : std::uint8_t {
  EQ,  // equal to
  NEQ, // not equal to
  GEQ, // great than or equal to
  LEQ, // less than or equal to
  GT,  // great than
  LT   // less than
};

// Arithmetic Operator
enum class AriOper : std::uint8_t {
  ADD,
  SUB,
  MUL,
  DIV
};

// Comparison Expression
struct CmpExpr : Expr {
  ExprPtr lhs; // 左部
  CmpOper op;  // operator
  ExprPtr rhs; // 右部

  CmpExpr(util::Position pos, ExprPtr lhs, CmpOper op, ExprPtr rhs)
    : Expr(pos), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  ~CmpExpr() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using CmpExprPtr = std::shared_ptr<CmpExpr>;

// Arithmetic Expression
struct AriExpr : Expr {
  ExprPtr lhs; // 左操作数
  AriOper op;  // operator
  ExprPtr rhs; // 右操作数

  AriExpr(util::Position pos, ExprPtr lhs, AriOper op, ExprPtr rhs)
    : Expr(pos), lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  ~AriExpr() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using AriExprPtr = std::shared_ptr<AriExpr>;

// Call Expression
struct CallExpr : Expr {
  std::string          callee; // 被调用函数名
  std::vector<ExprPtr> argv;   // argument vector

  CallExpr(util::Position pos, std::string callee,
    std::vector<ExprPtr> argv
  ) : Expr(pos), callee(std::move(callee)), argv(std::move(argv)) {}
  ~CallExpr() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using CallExprPtr = std::shared_ptr<CallExpr>;

// else 子句
struct ElseClause : Stmt {
  std::optional<ExprPtr> cond; // else (if expr)?
  BlockStmtPtr           body;

  ElseClause(util::Position pos, std::optional<ExprPtr> expr,
    BlockStmtPtr bstmt
  ) : Stmt(pos), cond(std::move(expr)), body(std::move(bstmt)) {}
  ~ElseClause() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using ElseClausePtr = std::shared_ptr<ElseClause>;

// if statement
struct IfStmt : Stmt {
  ExprPtr                    cond;
  BlockStmtPtr               body;
  std::vector<ElseClausePtr> elses; // else clauses

  IfStmt(util::Position pos, ExprPtr expr, BlockStmtPtr bstmt,
    std::vector<ElseClausePtr> elses
  ) : Stmt(pos), cond(std::move(expr)), body(std::move(bstmt)),
      elses(std::move(elses)) {}
  ~IfStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using IfStmtPtr = std::shared_ptr<IfStmt>;

// while statement
struct WhileStmt : Stmt {
  ExprPtr      cond;
  BlockStmtPtr body;

  WhileStmt(util::Position pos, ExprPtr expr, BlockStmtPtr bstmt)
    : Stmt(pos), cond(std::move(expr)), body(std::move(bstmt)) {}
  ~WhileStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using WhileStmtPtr = std::shared_ptr<WhileStmt>;

// for statement
struct ForStmt : Stmt {
  std::string  name;
  ExprPtr      start;
  ExprPtr      end;
  BlockStmtPtr body;

  ForStmt(util::Position pos, std::string name, ExprPtr lexpr,
    ExprPtr rexpr, BlockStmtPtr bstmt
  ) : Stmt(pos), name(std::move(name)), start(std::move(lexpr)),
      end(std::move(rexpr)), body(std::move(bstmt)) {}
  ~ForStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using ForStmtPtr = std::shared_ptr<ForStmt>;

// loop statement
struct LoopStmt : Stmt, Expr {
  BlockStmtPtr body;

  LoopStmt(util::Position pos, BlockStmtPtr bstmt)
    : Node(pos), body(std::move(bstmt)) {}
  ~LoopStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using LoopStmtPtr = std::shared_ptr<LoopStmt>;

// break statement
struct BreakStmt : Stmt {
  std::optional<ExprPtr> value;

  BreakStmt(util::Position pos, std::optional<ExprPtr> expr)
    : Stmt(pos), value(std::move(expr)) {}
  ~BreakStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using BreakStmtPtr = std::shared_ptr<BreakStmt>;

// continue statement
struct ContinueStmt : Stmt {
  ~ContinueStmt() override = default;
  void accept(visit::NodeVisitor& visitor) override;
};
using ContinueStmtPtr = std::shared_ptr<ContinueStmt>;

// null statement => ;
struct NullStmt : Stmt {
  ~NullStmt() override = default;
  void accept(visit::NodeVisitor& visitor) override;
};
using NullStmtPtr = std::shared_ptr<NullStmt>;

// 函数表达式语句块
struct FuncExprBlockStmt : Expr, BlockStmt {
  ExprPtr value; // the final expression

  FuncExprBlockStmt(std::vector<StmtPtr> stmts, ExprPtr expr)
    : BlockStmt(std::move(stmts)), value(std::move(expr)) {}
  ~FuncExprBlockStmt() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using FuncExprBlockStmtPtr = std::shared_ptr<FuncExprBlockStmt>;

struct IfExpr : Expr {
  ExprPtr              cond;
  FuncExprBlockStmtPtr tbranch;  // true branch
  FuncExprBlockStmtPtr fbranch; // false branch

  template <typename T, typename U, typename V>
  IfExpr(util::Position pos, T &&cond, U &&tbranch, V &&fbranch)
    : Expr(pos), cond(std::forward<T>(cond)),
      tbranch(std::forward<U>(tbranch)),
      fbranch(std::forward<V>(fbranch)) {}
  ~IfExpr() override = default;

  void accept(visit::NodeVisitor& visitor) override;
};
using IfExprPtr = std::shared_ptr<IfExpr>;

} // namespace parser::ast
