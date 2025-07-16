#pragma once

#include <vector>

#include "panic.hpp"
#include "symbol.hpp"
#include "ir_quad.hpp"
#include "position.hpp"
#include "type_factory.hpp"

namespace ast {

class OOPVisitor;

// 所有 AST 结点的基类
struct Node {
  util::Position pos;

  std::vector<ir::IRQuadPtr> ircode; // 存放结点对应的四元式序列

  virtual ~Node() = default;
  virtual void accept(OOPVisitor &visitor) = 0;
};
using NodePtr = std::shared_ptr<Node>;

template<typename Derived>
struct Visitable {
  template<typename VisitorT>
  void accept(VisitorT &visitor) {
    visitor.visit(static_cast<Derived&>(*this));
  }
};

// Declaration
struct Decl : virtual Node {
  ~Decl() override = default;
  void accept(OOPVisitor &visitor) override = 0;
};
using DeclPtr = std::shared_ptr<Decl>;

// Program
struct Prog : Node, Visitable<Prog> {
  std::vector<DeclPtr> decls; // declarations

  Prog(std::vector<DeclPtr> decls) : decls(std::move(decls)) {}
  ~Prog() override = default;
  void accept(OOPVisitor &visitor) override final;
};
using ProgPtr = std::shared_ptr<Prog>;

struct MetaNode : Node {
  void accept(OOPVisitor &) override final {
    UNREACHABLE("MetaNode cannot be visited");
  }
};

// Type - 封装了 type::Type
struct Type : MetaNode {
  type::TypePtr type;

  Type() : type(type::TypeFactory::UNKNOWN_TYPE) {}
  explicit Type(type::TypePtr t) : type(std::move(t)) {}
  ~Type() override = default;

  Type& operator=(const Type &other) {
    if (this == &other) {
      return *this;
    }
    this->type = other.type;
    return *this;
  }
  bool operator==(const Type& other) const {
    return type::typeEquals(this->type, other.type);
  }
  [[nodiscard]] std::string str() const {
    return type->str();
  }
};

// Argument
struct Arg : Node, Visitable<Arg> {
  bool          mut;
  std::string   name;
  Type          type;

  Arg(bool mut, std::string name, const Type &type
  ) : mut(mut), name(std::move(name)),
      type(type) {}
  ~Arg() override = default;

  void accept(OOPVisitor& visitor) override final;
};
using ArgPtr = std::shared_ptr<Arg>;

// Function header declaration
struct FuncHeaderDecl : Decl, Visitable<FuncHeaderDecl> {
  std::string         name; // function name
  std::vector<ArgPtr> argv; // argument vector
  Type                type; // return value type

  FuncHeaderDecl(std::string name,
    std::vector<ArgPtr> argv, const Type &type
  ) : name(std::move(name)), argv(std::move(argv)),
      type(type) {};
  ~FuncHeaderDecl() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using FuncHeaderDeclPtr = std::shared_ptr<FuncHeaderDecl>;

// Statement
struct Stmt : virtual Node {
  enum class Kind : std::uint8_t {
    EMPTY, // 空语句
    DECL,  // 声明语句（当前只有变量声明语句）
    EXPR,  // 表达式语句
  } kind;
  bool unreachable; // 是否可以执行到
  bool is_last = false;
  Type type;

  Stmt(Kind kind) : kind(kind) {}
  ~Stmt() override = default;
  void accept(OOPVisitor &visitor) override = 0;
};
using StmtPtr = std::shared_ptr<Stmt>;

// Empty Statement
struct EmptyStmt : Stmt, Visitable<Prog> {
  EmptyStmt() : Stmt(Kind::EMPTY) {}
  ~EmptyStmt() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using EmptyStmtPtr = std::shared_ptr<EmptyStmt>;

struct Expr;
using ExprPtr = std::shared_ptr<Expr>;

// Variable Declaration Statement
struct VarDeclStmt : Stmt, Visitable<VarDeclStmt> {
  bool        mut;
  std::string name;
  Type        vartype; // variable type
  std::optional<ExprPtr> value;

  VarDeclStmt(bool mut, std::string name,
    const Type &vartype, std::optional<ExprPtr> expr
  ) : Stmt(Kind::DECL), mut(mut), name(std::move(name)),
      vartype(vartype), value(std::move(expr)) {}
  ~VarDeclStmt() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using VarDeclStmtPtr = std::shared_ptr<VarDeclStmt>;

// Expression
struct Expr : virtual Node {
  sym::ValuePtr symbol; // 表达式计算结果存储位置

  bool res_mut; // 表达式计算结果是否可变
  bool used_as_stmt;
  bool is_ctlflow = false;
  bool is_var = false; // 是否是一个变量
  Type type; // value type

  ~Expr() override = default;
  void accept(OOPVisitor& visitor) override = 0;
};

struct EmptyExpr : Expr, Visitable<EmptyExpr> {
  ~EmptyExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using EmptyExprPtr = std::shared_ptr<EmptyExpr>;

// Return Expression
struct RetExpr : Expr, Visitable<RetExpr> {
  std::optional<ExprPtr> retval; // return value (an expression)

  RetExpr(std::optional<ExprPtr> retval)
    : retval(std::move(retval)) {}
  ~RetExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using RetExprPtr = std::shared_ptr<RetExpr>;

// break expression
struct BreakExpr : Expr, Visitable<BreakExpr> {
  std::optional<ExprPtr> value;

  BreakExpr(std::optional<ExprPtr> expr)
    : value(std::move(expr)) {}
  ~BreakExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using BreakExprPtr = std::shared_ptr<BreakExpr>;

// continue expression
struct ContinueExpr : Expr, Visitable<ContinueExpr> {
  ~ContinueExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using ContinueExprPtr = std::shared_ptr<ContinueExpr>;

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
  ADD, // +
  SUB, // -
  MUL, // *
  DIV  // /
};

// Comparison Expression
struct CmpExpr : Expr, Visitable<CmpExpr> {
  ExprPtr lhs; // 左部
  CmpOper op;  // operator
  ExprPtr rhs; // 右部

  CmpExpr(ExprPtr lhs, CmpOper op, ExprPtr rhs)
    : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  ~CmpExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using CmpExprPtr = std::shared_ptr<CmpExpr>;

// Arithmetic Expression
struct AriExpr : Expr, Visitable<AriExpr> {
  ExprPtr lhs; // 左操作数
  AriOper op;  // operator
  ExprPtr rhs; // 右操作数

  AriExpr(ExprPtr lhs, AriOper op, ExprPtr rhs)
    : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  ~AriExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using AriExprPtr = std::shared_ptr<AriExpr>;

struct Number : Expr, Visitable<Number> {
  int value; // 值

  Number(int value) : value(value) {}
  ~Number() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using NumberPtr = std::shared_ptr<Number>;

struct Variable : Expr, Visitable<Variable> {
  std::string name;

  Variable(std::string name) : name(std::move(name)) {}
  ~Variable() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using VariablePtr = std::shared_ptr<Variable>;

// Assign Element
struct AssignElem : Expr, Visitable<AssignElem> {
  enum class Kind : std::uint8_t {
    VARIABLE,
    ARRACC,
    TUPACC
  } kind;

  ExprPtr value;

  AssignElem(ExprPtr value) : value(std::move(value)) {}
  ~AssignElem() override = default;
  void accept(OOPVisitor& visitor) override;
};
using AssignElemPtr = std::shared_ptr<AssignElem>;

// 数组访问
struct ArrAcc : AssignElem {
  ExprPtr idx; // 索引值

  ArrAcc(ExprPtr value, ExprPtr idx)
    : AssignElem(std::move(value)), idx(std::move(idx)) {}
  ~ArrAcc() override = default;

  void accept(OOPVisitor& visitor) override final;
  // 重写 accept，调用 visit(ArrAcc&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using ArrAccPtr = std::shared_ptr<ArrAcc>;

// 元组访问
struct TupAcc : AssignElem {
  int idx; // 索引值

  TupAcc(ExprPtr value, int idx)
    : AssignElem(std::move(value)), idx(idx) {}
  ~TupAcc() override = default;

  void accept(OOPVisitor& visitor) override final;
  // 重写 accept，调用 visit(ArrAcc&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using TupAccPtr = std::shared_ptr<TupAcc>;

// Expression Statement
struct ExprStmt : Stmt, Visitable<ExprStmt> {
  ExprPtr expr;

  ExprStmt(ExprPtr expr) : Stmt(Kind::EXPR), expr(std::move(expr)) {}
  ~ExprStmt() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using ExprStmtPtr = std::shared_ptr<ExprStmt>;

// Statement Block Expression
struct StmtBlockExpr : Expr, Visitable<StmtBlockExpr> {
  bool has_ret; // 是否含有返回语句
  std::vector<StmtPtr> stmts; // statements

  StmtBlockExpr(std::vector<StmtPtr> stmts)
    : stmts(std::move(stmts)) {}
  ~StmtBlockExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using StmtBlockExprPtr = std::shared_ptr<StmtBlockExpr>;

// Function Declaration
struct FuncDecl : Decl, Visitable<FuncDecl> {
  FuncHeaderDeclPtr header; // function header
  StmtBlockExprPtr  body;   // function body

  FuncDecl(FuncHeaderDeclPtr header, StmtBlockExprPtr body)
    : header(std::move(header)), body(std::move(body)) {}
  ~FuncDecl() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using FuncDeclPtr = std::shared_ptr<FuncDecl>;

// 括号表达式
struct BracketExpr : Expr, Visitable<BracketExpr> {
  std::optional<ExprPtr> expr; // ( expr )，允许空括号

  BracketExpr(std::optional<ExprPtr> expr)
    : expr(std::move(expr)) {}
  ~BracketExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using BracketExprPtr = std::shared_ptr<BracketExpr>;

// Array Elements => e.g., [1, 2, 3]
struct ArrElems : Expr, Visitable<ArrElems> {
  std::vector<ExprPtr> elems;

  ArrElems(std::vector<ExprPtr> elems)
    : elems(std::move(elems)) {}
  ~ArrElems() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using ArrElemsPtr = std::shared_ptr<ArrElems>;

// Tuple Elements => e.g. (1, 2)
struct TupElems : Expr, Visitable<TupElems> {
  std::vector<ExprPtr> elems;

  TupElems(std::vector<ExprPtr> elems)
    : elems(std::move(elems)) {}
  ~TupElems() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using TupElemsPtr = std::shared_ptr<TupElems>;

// Assign Expression
struct AssignExpr : Expr, Visitable<AssignExpr> {
  AssignElemPtr lval;
  ExprPtr       rval; // expression

  AssignExpr(AssignElemPtr lval, ExprPtr rval)
    : lval(std::move(lval)), rval(std::move(rval)) {}
  ~AssignExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using AssignExprPtr = std::shared_ptr<AssignExpr>;

// Call Expression
struct CallExpr : Expr, Visitable<CallExpr> {
  std::string          callee; // 被调用函数名
  std::vector<ExprPtr> argv;   // argument vector

  CallExpr(std::string callee, std::vector<ExprPtr> argv
  ) : callee(std::move(callee)), argv(std::move(argv)) {}
  ~CallExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using CallExprPtr = std::shared_ptr<CallExpr>;

// else 子句
struct ElseClause : Node, Visitable<ElseClause> {
  std::optional<ExprPtr> cond; // else (if expr)?
  StmtBlockExprPtr       body;

  ElseClause(std::optional<ExprPtr> cond, StmtBlockExprPtr body
  ) : cond(std::move(cond)), body(std::move(body)) {}
  ~ElseClause() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using ElseClausePtr = std::shared_ptr<ElseClause>;

// If Expression
struct IfExpr : Expr, Visitable<IfExpr> {
  ExprPtr                    cond;
  StmtBlockExprPtr           body;
  std::vector<ElseClausePtr> elses; // else clauses

  IfExpr(ExprPtr cond, StmtBlockExprPtr body,
    std::vector<ElseClausePtr> elses
  ) : cond(std::move(cond)), body(std::move(body)),
      elses(std::move(elses)) {}
  ~IfExpr() override = default;
  void accept(OOPVisitor& visitor) override final;
};
using IfExprPtr = std::shared_ptr<IfExpr>;

// loop expression
struct LoopExpr : Expr, Visitable<LoopExpr> {
  StmtBlockExprPtr body;

  LoopExpr(StmtBlockExprPtr body)
    : body(std::move(body)) {}
  ~LoopExpr() override = default;
  void accept(OOPVisitor& visitor) override;
};
using LoopExprPtr = std::shared_ptr<LoopExpr>;

// while loop expression
struct WhileLoopExpr : LoopExpr {
  ExprPtr cond;

  WhileLoopExpr(ExprPtr cond, StmtBlockExprPtr body)
    : LoopExpr(std::move(body)), cond(std::move(cond)) {}
  ~WhileLoopExpr() override = default;

  void accept(OOPVisitor& visitor) override final;
  // 重写 accept，调用 visit(WhileLoopExpr&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using WhileLoopExprPtr = std::shared_ptr<WhileLoopExpr>;

struct IterableVal : Expr, Visitable<IterableVal> {
  ExprPtr value;

  IterableVal(ExprPtr value) : value(std::move(value)) {}
  ~IterableVal() override = default;
  void accept(OOPVisitor &visitor) override final;
};
using IterableValPtr = std::shared_ptr<IterableVal>;

struct Interval : Expr, Visitable<Interval> {
  // 左闭右开区间
  ExprPtr start;
  ExprPtr end;

  Interval(ExprPtr start, ExprPtr end)
    : start(std::move(start)), end(std::move(end)) {}
  ~Interval() override = default;
  void accept(OOPVisitor &visitor) override final;
};
using IntervalPtr = std::shared_ptr<Interval>;

// for loop expression
struct ForLoopExpr : LoopExpr {
  bool mut;
  std::string name;
  ExprPtr iter;

  ForLoopExpr(bool mut, std::string name,
    ExprPtr iter, StmtBlockExprPtr body
  ) : LoopExpr(std::move(body)), mut(mut),
      name(std::move(name)), iter(std::move(iter)) {}
  ~ForLoopExpr() override = default;

  void accept(OOPVisitor &visitor) override final;
  // 重写 accept，调用 visit(ForLoopExpr&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using ForLoopExprPtr = std::shared_ptr<ForLoopExpr>;

} // namespace ast
