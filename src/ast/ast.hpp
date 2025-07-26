/**
 * @file ast.hpp
 * @brief Defines the Abstract Syntax Tree (AST) node structures.
 *
 * This header contains the core AST node definitions, including statements, expressions,
 * declarations, types, and control flow constructs. It supports both OOP and CRTP visitor patterns
 * for traversing and processing the AST. Each node records its source position and may store
 * intermediate representation (IR) quads for code generation.
 *
 * Key Features:
 * - Node hierarchy for all language constructs (program, declarations, statements, expressions, etc.)
 * - Support for both OOP and CRTP visitor patterns via `accept` methods
 * - Type information encapsulated in `Type` nodes
 * - Rich set of expression and statement types, including control flow, function calls, assignments, etc.
 * - Use of smart pointers for memory management
 * - Source code position tracking for error reporting and diagnostics
 *
 * Namespaces:
 * - ast: Contains all AST node definitions and related types
 *
 * Dependencies:
 * - position.hpp: Source code position tracking
 * - type_factory.hpp: Type system and type construction utilities
 */
#pragma once

#include <memory>
#include <vector>

#include "position.hpp"
#include "type_factory.hpp"

namespace ir {

struct IRQuad;
using IRQuadPtr = std::shared_ptr<IRQuad>;

} // namespace ir

namespace sym {

struct Value;
using ValuePtr = std::shared_ptr<Value>;

} // namespace sym

namespace ast {

class OOPVisitor;

/// 所有 AST 结点的基类
struct Node {
  util::Position pos; // 在源代码中的位置
  std::vector<ir::IRQuadPtr> ircode; // 存放结点对应的四元式序列

  virtual ~Node() = default;
  virtual void accept(OOPVisitor &visitor) = 0; // 传统双分发 visitor 接口
};
using NodePtr = std::shared_ptr<Node>;

// CRTP 模式 visitor 接口
// 需要使用 CRTP 模式进行访问的 AST 结点都需要继承该基类
template<typename Derived>
struct CRTPVisitable {
  template<typename VisitorT>
  void accept(VisitorT &visitor) {
    // 编译时确定实际调用的 visit 方法，减少运行时开销
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
struct Prog : Node, CRTPVisitable<Prog> {
  std::vector<DeclPtr> decls; // declarations

  Prog(std::vector<DeclPtr> decls) : decls(std::move(decls)) {}
  ~Prog() override = default;
  void accept(OOPVisitor &visitor) final;
};
using ProgPtr = std::shared_ptr<Prog>;

// 显式弃用 visit 访问，所有不应该被 visitor 访问的结点都需要继承该基类
struct MetaNode : Node {
  void accept(OOPVisitor &visitor) final {
    UNREACHABLE("MetaNode cannot be visited");
  }
};

// Type - 封装了 type::Type
struct Type : MetaNode {
  type::TypePtr type;

  Type() : type(type::TypeFactory::UNKNOWN_TYPE) {}
  explicit Type(type::TypePtr t) : type(std::move(t)) {}
  ~Type() override = default;

  bool operator==(const Type& other) const {
    return type::typeEquals(this->type, other.type);
  }
  [[nodiscard]] std::string str() const { return type->str(); }
};

// Argument
struct Arg : Node, CRTPVisitable<Arg> {
  bool        mut;  // mutable or not
  std::string name; // argument name
  Type        type; // argument type

  Arg(bool mut, std::string name, const Type &type)
    : mut(mut), name(std::move(name)), type(type) {}
  ~Arg() override = default;

  void accept(OOPVisitor& visitor) final;
};
using ArgPtr = std::shared_ptr<Arg>;

// Function header declaration
struct FuncHeaderDecl : Decl, CRTPVisitable<FuncHeaderDecl> {
  std::string         name; // function name
  std::vector<ArgPtr> argv; // argument vector
  Type                type; // return value type

  FuncHeaderDecl(std::string name,
    std::vector<ArgPtr> argv, const Type &type
  ) : name(std::move(name)), argv(std::move(argv)),
      type(type) {};
  ~FuncHeaderDecl() override = default;
  void accept(OOPVisitor& visitor) final;
};
using FuncHeaderDeclPtr = std::shared_ptr<FuncHeaderDecl>;

// Statement
struct Stmt : virtual Node {
  enum class Kind : std::uint8_t {
    EMPTY, // 空语句
    DECL,  // 声明语句（当前只有变量声明语句）
    EXPR,  // 表达式语句
  } kind;
  bool unreachable = false; // 是否可以执行到
  bool is_last = false;
  Type type;

  Stmt(Kind kind) : kind(kind) {}
  ~Stmt() override = default;
  void accept(OOPVisitor &visitor) override = 0;
};
using StmtPtr = std::shared_ptr<Stmt>;

// Empty Statement
struct EmptyStmt : Stmt, CRTPVisitable<EmptyStmt> {
  EmptyStmt() : Stmt(Kind::EMPTY) {}
  ~EmptyStmt() override = default;
  void accept(OOPVisitor& visitor) final;
};
using EmptyStmtPtr = std::shared_ptr<EmptyStmt>;

struct Expr;
using ExprPtr = std::shared_ptr<Expr>;

// Variable Declaration Statement
struct VarDeclStmt : Stmt, CRTPVisitable<VarDeclStmt> {
  bool        mut;     // mutable or not (the declared variable)
  std::string name;    // the declared variable name
  Type        vartype; // the declared variable type
  std::optional<ExprPtr> rval; // r-value for variable initialization

  VarDeclStmt(bool mut, std::string name,
    const Type &vartype, std::optional<ExprPtr> expr
  ) : Stmt(Kind::DECL), mut(mut), name(std::move(name)),
      vartype(vartype), rval(std::move(expr)) {}
  ~VarDeclStmt() override = default;
  void accept(OOPVisitor& visitor) final;
};
using VarDeclStmtPtr = std::shared_ptr<VarDeclStmt>;

// Expression
struct Expr : virtual Node {
  sym::ValuePtr symbol; // 表达式计算结果存储位置

  bool res_mut = false; // 表达式计算结果是否可变
  bool used_as_stmt; // used as a statement or not
  bool is_ctlflow = false; // is a control flow expression
                           // (if expression, loop expression etc.)
  bool is_var = false; // 是否是一个变量
  Type type; // value type

  ~Expr() override = default;
  void accept(OOPVisitor& visitor) override = 0;
};

struct EmptyExpr : Expr, CRTPVisitable<EmptyExpr> {
  ~EmptyExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using EmptyExprPtr = std::shared_ptr<EmptyExpr>;

// Return Expression
struct RetExpr : Expr, CRTPVisitable<RetExpr> {
  std::optional<ExprPtr> retval; // return value (an expression)

  RetExpr(std::optional<ExprPtr> retval)
    : retval(std::move(retval)) {}
  ~RetExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using RetExprPtr = std::shared_ptr<RetExpr>;

// break expression
struct BreakExpr : Expr, CRTPVisitable<BreakExpr> {
  // break expression can return
  // a value if it in a loop context
  std::optional<ExprPtr> value;

  // the destination of the break return value
  // corresponding to the value of the loop expression
  std::optional<sym::ValuePtr> dst;

  BreakExpr(std::optional<ExprPtr> expr)
    : value(std::move(expr)) {}
  ~BreakExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using BreakExprPtr = std::shared_ptr<BreakExpr>;

// continue expression
struct ContinueExpr : Expr, CRTPVisitable<ContinueExpr> {
  ~ContinueExpr() override = default;
  void accept(OOPVisitor& visitor) final;
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
struct CmpExpr : Expr, CRTPVisitable<CmpExpr> {
  ExprPtr lhs; // 左部
  CmpOper op;  // operator
  ExprPtr rhs; // 右部

  CmpExpr(ExprPtr lhs, CmpOper op, ExprPtr rhs)
    : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  ~CmpExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using CmpExprPtr = std::shared_ptr<CmpExpr>;

// Arithmetic Expression
struct AriExpr : Expr, CRTPVisitable<AriExpr> {
  ExprPtr lhs; // 左操作数
  AriOper op;  // operator
  ExprPtr rhs; // 右操作数

  AriExpr(ExprPtr lhs, AriOper op, ExprPtr rhs)
    : lhs(std::move(lhs)), op(op), rhs(std::move(rhs)) {}
  ~AriExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using AriExprPtr = std::shared_ptr<AriExpr>;

struct Number : Expr, CRTPVisitable<Number> {
  int value; // 值

  Number(int value) : value(value) {}
  ~Number() override = default;
  void accept(OOPVisitor& visitor) final;
};
using NumberPtr = std::shared_ptr<Number>;

struct Variable : Expr, CRTPVisitable<Variable> {
  std::string name; // variable name

  Variable(std::string name) : name(std::move(name)) {}
  ~Variable() override = default;
  void accept(OOPVisitor& visitor) final;
};
using VariablePtr = std::shared_ptr<Variable>;

// Assign Element
struct AssignElem : Expr, CRTPVisitable<AssignElem> {
  enum class Kind : std::uint8_t {
    VARIABLE, // 变量
    ARRACC,   // 数组访问
    TUPACC    // 元组访问
  } kind;

  ExprPtr base;

  AssignElem(ExprPtr expr) : base(std::move(expr)) {}
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

  void accept(OOPVisitor& visitor) final;
  // 重写 accept，调用 visit(TupAcc&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using ArrAccPtr = std::shared_ptr<ArrAcc>;

// 元组访问
struct TupAcc : AssignElem {
  NumberPtr idx; // 索引值

  TupAcc(ExprPtr value, NumberPtr idx)
    : AssignElem(std::move(value)), idx(std::move(idx)) {}
  ~TupAcc() override = default;

  void accept(OOPVisitor& visitor) final;
  // 重写 accept，调用 visit(ArrAcc&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using TupAccPtr = std::shared_ptr<TupAcc>;

// Expression Statement
struct ExprStmt : Stmt, CRTPVisitable<ExprStmt> {
  ExprPtr expr;

  ExprStmt(ExprPtr expr) : Stmt(Kind::EXPR), expr(std::move(expr)) {}
  ~ExprStmt() override = default;
  void accept(OOPVisitor& visitor) final;
};
using ExprStmtPtr = std::shared_ptr<ExprStmt>;

// Statement Block Expression
struct StmtBlockExpr : Expr, CRTPVisitable<StmtBlockExpr> {
  bool has_ret = false; // 是否含有返回语句
  std::vector<StmtPtr> stmts; // statements

  StmtBlockExpr(std::vector<StmtPtr> stmts)
    : stmts(std::move(stmts)) {}
  ~StmtBlockExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using StmtBlockExprPtr = std::shared_ptr<StmtBlockExpr>;

// Function Declaration
struct FuncDecl : Decl, CRTPVisitable<FuncDecl> {
  FuncHeaderDeclPtr header; // function header
  StmtBlockExprPtr  body;   // function body

  FuncDecl(FuncHeaderDeclPtr header, StmtBlockExprPtr body)
    : header(std::move(header)), body(std::move(body)) {}
  ~FuncDecl() override = default;
  void accept(OOPVisitor& visitor) final;
};
using FuncDeclPtr = std::shared_ptr<FuncDecl>;

// 括号表达式
struct BracketExpr : Expr, CRTPVisitable<BracketExpr> {
  std::optional<ExprPtr> expr; // ( expr )，允许空括号

  BracketExpr(std::optional<ExprPtr> expr)
    : expr(std::move(expr)) {}
  ~BracketExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using BracketExprPtr = std::shared_ptr<BracketExpr>;

// Array Elements => e.g., [1, 2, 3]
struct ArrElems : Expr, CRTPVisitable<ArrElems> {
  std::vector<ExprPtr> elems;

  ArrElems(std::vector<ExprPtr> elems)
    : elems(std::move(elems)) {}
  ~ArrElems() override = default;
  void accept(OOPVisitor& visitor) final;
};
using ArrElemsPtr = std::shared_ptr<ArrElems>;

// Tuple Elements => e.g. (1, 2)
struct TupElems : Expr, CRTPVisitable<TupElems> {
  std::vector<ExprPtr> elems;

  TupElems(std::vector<ExprPtr> elems)
    : elems(std::move(elems)) {}
  ~TupElems() override = default;
  void accept(OOPVisitor& visitor) final;
};
using TupElemsPtr = std::shared_ptr<TupElems>;

// Assign Expression
struct AssignExpr : Expr, CRTPVisitable<AssignExpr> {
  AssignElemPtr lval;
  ExprPtr       rval; // expression

  AssignExpr(AssignElemPtr lval, ExprPtr rval)
    : lval(std::move(lval)), rval(std::move(rval)) {}
  ~AssignExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using AssignExprPtr = std::shared_ptr<AssignExpr>;

// Call Expression
struct CallExpr : Expr, CRTPVisitable<CallExpr> {
  std::string          callee; // 被调用函数名
  std::vector<ExprPtr> argv;   // argument vector

  CallExpr(std::string callee, std::vector<ExprPtr> argv
  ) : callee(std::move(callee)), argv(std::move(argv)) {}
  ~CallExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using CallExprPtr = std::shared_ptr<CallExpr>;

// else 子句
struct ElseClause : Node, CRTPVisitable<ElseClause> {
  sym::ValuePtr          symbol;
  std::optional<ExprPtr> cond; // else (if expr)?
  StmtBlockExprPtr       body;

  ElseClause(std::optional<ExprPtr> cond, StmtBlockExprPtr body
  ) : cond(std::move(cond)), body(std::move(body)) {}
  ~ElseClause() override = default;
  void accept(OOPVisitor& visitor) final;
};
using ElseClausePtr = std::shared_ptr<ElseClause>;

// If Expression
struct IfExpr : Expr, CRTPVisitable<IfExpr> {
  ExprPtr                    cond;
  StmtBlockExprPtr           body;
  std::vector<ElseClausePtr> elses; // else clauses

  IfExpr(ExprPtr cond, StmtBlockExprPtr body,
    std::vector<ElseClausePtr> elses
  ) : cond(std::move(cond)), body(std::move(body)),
      elses(std::move(elses)) {}
  ~IfExpr() override = default;
  void accept(OOPVisitor& visitor) final;
};
using IfExprPtr = std::shared_ptr<IfExpr>;

// loop expression
struct LoopExpr : Expr, CRTPVisitable<LoopExpr> {
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

  void accept(OOPVisitor& visitor) final;
  // CRTP visitor: 调用 visit(WhileLoopExpr&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using WhileLoopExprPtr = std::shared_ptr<WhileLoopExpr>;

// 可迭代的值
// 可以是一个 Array 类型的变量或中间值
struct IterableVal : Expr, CRTPVisitable<IterableVal> {
  ExprPtr value;

  IterableVal(ExprPtr value) : value(std::move(value)) {}
  ~IterableVal() override = default;
  void accept(OOPVisitor &visitor) final;
};
using IterableValPtr = std::shared_ptr<IterableVal>;

struct RangeExpr : Expr, CRTPVisitable<RangeExpr> {
  // 左闭右开区间
  ExprPtr start;
  ExprPtr end;

  RangeExpr(ExprPtr start, ExprPtr end)
    : start(std::move(start)), end(std::move(end)) {}
  ~RangeExpr() override = default;
  void accept(OOPVisitor &visitor) final;
};
using RangeExprPtr = std::shared_ptr<RangeExpr>;

// for loop expression
struct ForLoopExpr : LoopExpr {
  bool        mut;
  std::string pattern;
  ExprPtr     iterexpr;

  ForLoopExpr(bool mut, std::string pattern, ExprPtr iterexpr,
    StmtBlockExprPtr body) : LoopExpr(std::move(body)), mut(mut),
      pattern(std::move(pattern)), iterexpr(std::move(iterexpr)) {}
  ~ForLoopExpr() override = default;

  void accept(OOPVisitor &visitor) final;
  // 重写 accept，调用 visit(ForLoopExpr&)
  template <typename Visitor>
  void accept(Visitor &visitor) {
    visitor.visit(*this);
  }
};
using ForLoopExprPtr = std::shared_ptr<ForLoopExpr>;

} // namespace ast
