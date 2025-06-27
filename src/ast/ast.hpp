#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "util/position.hpp"

namespace par::ast {

// 结点类型
enum class NodeType : std::uint8_t {
  Prog, Arg,

  Decl, Stmt, Expr, VarType,
  VarDeclBody, AssignElement,

  FuncDecl, FuncHeaderDecl,

  BlockStmt, ExprStmt, RetStmt, VarDeclStmt,
  AssignStmt, VarDeclAssignStmt, ElseClause,
  IfStmt, WhileStmt, ForStmt, LoopStmt,
  BreakStmt, ContinueStmt, NullStmt,

  Number, Factor, ComparExpr, ArithExpr,
  CallExpr, ParenthesisExpr, FuncExprBlockStmt,
  IfExpr, ArrayElements, TupleElements,

  Integer, Array, Tuple,

  Variable, Dereference, ArrayAccess, TupleAccess
};

// 所有 AST 结点的基类
struct Node {
  util::Position pos{0, 0};

  Node() = default;
  virtual ~Node() = default;

  void setPos(std::size_t row, std::size_t col);
  void setPos(util::Position pos);

  [[nodiscard]]
  auto getPos() const -> util::Position;

  [[nodiscard]]
  virtual auto type() const -> NodeType = 0;
};
using NodePtr = std::shared_ptr<Node>;

// Declaration
struct Decl : virtual Node {
  ~Decl() override = default;

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using DeclPtr = std::shared_ptr<Decl>;

// Program
struct Prog : virtual Node {
  std::vector<DeclPtr> decls; // declarations

  Prog() = default;
  explicit Prog(const std::vector<DeclPtr> &ds) : decls(ds) {}
  explicit Prog(std::vector<DeclPtr> &&ds) : decls(std::move(ds)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ProgPtr = std::shared_ptr<Prog>;

// 变量声明内部
struct VarDeclBody : virtual Node {
  bool        mut = true; // mutable or not
  std::string name;       // variable name

  VarDeclBody() = default;
  explicit VarDeclBody(bool mut, const std::string &n) : mut(mut), name(n) {}
  explicit VarDeclBody(bool mut, std::string &&n) : mut(mut), name(n) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using VarDeclBodyPtr = std::shared_ptr<VarDeclBody>;

// 类型引用修饰符
enum class RefType : std::uint8_t {
  Normal,    // 普通变量
  Immutable, // 不可变引用
  Mutable    // 可变引用
};

// Variable Type
struct VarType : virtual Node {
  RefType ref_type = RefType::Normal;

  VarType() = default;
  explicit VarType(RefType rt) : ref_type(rt) {}
  ~VarType() override = default;

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using VarTypePtr = std::shared_ptr<VarType>;

struct Integer : VarType {
  Integer() = default;
  explicit Integer(RefType rt) : VarType(rt) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using IntegerPtr = std::shared_ptr<Integer>;

struct Array : VarType
{
  int cnt = 0;          // 数组中元素个数
  VarTypePtr elem_type; // 数组中元素的类型

  Array() = default;
  explicit Array(int cnt, const VarTypePtr &et, RefType rt = RefType::Normal)
    : VarType(rt), cnt(cnt), elem_type(et) {}
  explicit Array(int cnt, VarTypePtr &&et, RefType rt = RefType::Normal)
    : VarType(rt), cnt(cnt), elem_type(std::move(et)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ArrayPtr = std::shared_ptr<Array>;

struct Tuple : VarType {
  int                     cnt = 0;    // 元素个数
  std::vector<VarTypePtr> elem_types; // 每个元素的类型

  Tuple() = default;
  explicit Tuple(const std::vector<VarTypePtr> &et, RefType rt = RefType::Normal)
    : VarType(rt), cnt(static_cast<int>(et.size())), elem_types(et) {}
  explicit Tuple(std::vector<VarTypePtr> &&et, RefType rt = RefType::Normal)
    : VarType(rt), cnt(static_cast<int>(et.size())), elem_types(std::move(et)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using TuplePtr = std::shared_ptr<Tuple>;

// Argument
struct Arg : virtual Node {
  VarDeclBodyPtr variable; // variable
  VarTypePtr var_type;     // variable type

  Arg() = default;
  explicit Arg(const VarDeclBodyPtr &var, const VarTypePtr &vt)
    : variable(var), var_type(vt) {}
  explicit Arg(VarDeclBodyPtr &&var, VarTypePtr &&vt)
    : variable(std::move(var)), var_type(std::move(vt)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ArgPtr = std::shared_ptr<Arg>;

// Statement
struct Stmt : virtual Node {
  ~Stmt() override = default;

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using StmtPtr = std::shared_ptr<Stmt>;

// Block statement
struct BlockStmt : Stmt {
  std::vector<StmtPtr> stmts; // statements

  BlockStmt() = default;
  explicit BlockStmt(const std::vector<StmtPtr> &s) : stmts(s) {}
  explicit BlockStmt(std::vector<StmtPtr> &&s) : stmts(std::move(s)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using BlockStmtPtr = std::shared_ptr<BlockStmt>;

// Function header declaration
struct FuncHeaderDecl : Decl {
  std::string name;                      // function name
  std::vector<ArgPtr> argv;              // argument vector
  std::optional<VarTypePtr> retval_type; // return value type

  FuncHeaderDecl() = default;
  explicit FuncHeaderDecl(const std::string &n, const std::vector<ArgPtr> &av,
                          const std::optional<VarTypePtr> &rt)
    : name(n), argv(av), retval_type(rt) {};
  explicit FuncHeaderDecl(std::string &&n, std::vector<ArgPtr> &&av,
                          std::optional<VarTypePtr> &&rt)
    : name(std::move(n)), argv(std::move(av)), retval_type(std::move(rt)) {}

  [[nodiscard]] auto type() const -> NodeType override;
};
using FuncHeaderDeclPtr = std::shared_ptr<FuncHeaderDecl>;

// Function Declaration
struct FuncDecl : Decl {
  FuncHeaderDeclPtr header; // function header
  BlockStmtPtr body;        // function body

  FuncDecl() = default;
  explicit FuncDecl(const FuncHeaderDeclPtr &h, const BlockStmtPtr &b)
    : header(h), body(b) {}
  explicit FuncDecl(FuncHeaderDeclPtr &&h, BlockStmtPtr &&b)
    : header(std::move(h)), body(std::move(b)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using FuncDeclPtr = std::shared_ptr<FuncDecl>;

// Expression
struct Expr : virtual Node {
  ~Expr() override = default;
  [[nodiscard]] auto type() const -> NodeType override;
};
using ExprPtr = std::shared_ptr<Expr>;

// Expression Statement
struct ExprStmt : Stmt {
  ExprPtr expr;

  ExprStmt() = default;
  explicit ExprStmt(const ExprPtr &e) : expr(e) {}
  explicit ExprStmt(ExprPtr &&e) : expr(std::move(e)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ExprStmtPtr = std::shared_ptr<ExprStmt>;

// 括号表达式
struct ParenthesisExpr : Expr {
  ExprPtr expr; // ( expr ) - 括号中的表达式

  ParenthesisExpr() = delete;
  explicit ParenthesisExpr(const ExprPtr &e) : expr(e) {}
  explicit ParenthesisExpr(ExprPtr &&e) : expr(std::move(e)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ParenthesisExprPtr = std::shared_ptr<ParenthesisExpr>;

// Assign Element
struct AssignElement : Expr {
  enum class Kind : std::uint8_t {
    Variable,    // 普通变量 x
    Dereference, // 解引用 *x
    ArrayAccess, // 数组访问 a[i]
    TupleAccess  // 元组访问 t.0
  };
  Kind kind = Kind::Variable;

  AssignElement() = default;
  AssignElement(Kind k) : kind(k) {}
  ~AssignElement() override = default;

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using AssignElementPtr = std::shared_ptr<AssignElement>;

struct Variable : AssignElement {
  std::string name; // 变量名

  Variable() = default;
  explicit Variable(const std::string &n)
    : AssignElement(Kind::Variable), name(n) {}
  explicit Variable(std::string &&n)
    : AssignElement(Kind::Variable), name(std::move(n)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using VariablePtr = std::shared_ptr<Variable>;

struct Dereference : AssignElement {
  std::string target; // 被解引用的变量 x

  Dereference() = default;
  explicit Dereference(const std::string &t) : target(t) {}
  explicit Dereference(std::string &&t)
    : AssignElement(Kind::Dereference), target(std::move(t)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using DereferencePtr = std::shared_ptr<Dereference>;

struct ArrayAccess : AssignElement {
  std::string array; // 数组名
  ExprPtr index;     // 索引值

  ArrayAccess() = default;
  explicit ArrayAccess(const std::string &a, const ExprPtr &idx)
    : AssignElement(Kind::ArrayAccess), array(a), index(idx) {}
  explicit ArrayAccess(std::string &&a, ExprPtr &&idx)
    : AssignElement(Kind::ArrayAccess), array(std::move(a)), index(std::move(idx)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ArrayAccessPtr = std::shared_ptr<ArrayAccess>;

struct TupleAccess : AssignElement {
  std::string tuple; // 元组名
  int index;         // 索引值

  TupleAccess() = default;
  explicit TupleAccess(const std::string &t, int idx)
    : AssignElement(Kind::TupleAccess), tuple(t), index(idx) {}
  explicit TupleAccess(std::string &&t, int idx)
    : AssignElement(Kind::TupleAccess), tuple(std::move(t)), index(idx) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using TupleAccessPtr = std::shared_ptr<TupleAccess>;

struct Number : Expr {
  int value; // 值

  Number() = default;
  Number(int value) : value(value) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using NumberPtr = std::shared_ptr<Number>;

struct Factor : Expr
{
  RefType ref_type;
  ExprPtr element;

  Factor() = default;
  explicit Factor(RefType rt, const ExprPtr &e) : ref_type(rt), element(e) {}
  explicit Factor(RefType rt, ExprPtr &&e) : ref_type(rt), element(std::move(e)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using FactorPtr = std::shared_ptr<Factor>;

struct ArrayElements : Expr
{
  std::vector<ExprPtr> elements;

  ArrayElements() = default;
  explicit ArrayElements(const std::vector<ExprPtr> &els) : elements(els) {}
  explicit ArrayElements(std::vector<ExprPtr> &&els) : elements(std::move(els)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ArrayElementsPtr = std::shared_ptr<ArrayElements>;

struct TupleElements : Expr {
  std::vector<ExprPtr> elements;

  TupleElements() = default;
  explicit TupleElements(const std::vector<ExprPtr> &els) : elements(els) {}
  explicit TupleElements(std::vector<ExprPtr> &&els) : elements(std::move(els)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using TupleElementsPtr = std::shared_ptr<TupleElements>;

// Return Statement
struct RetStmt : Stmt {
  std::optional<ExprPtr> ret_val; // return value (an expression)

  RetStmt() = default;
  explicit RetStmt(const std::optional<ExprPtr> &rv) : ret_val(rv) {}
  explicit RetStmt(std::optional<ExprPtr> &&rv) : ret_val(std::move(rv)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using RetStmtPtr = std::shared_ptr<RetStmt>;

// Variable Declaration Statement
struct VarDeclStmt : Stmt, Decl {
  VarDeclBodyPtr variable;            // variable
  std::optional<VarTypePtr> var_type; // variable type

  VarDeclStmt() = default;
  explicit VarDeclStmt(const VarDeclBodyPtr &var, const std::optional<VarTypePtr> &vt)
    : variable(var), var_type(vt) {}
  explicit VarDeclStmt(VarDeclBodyPtr &&var, std::optional<VarTypePtr> &&vt)
    : variable(std::move(var)), var_type(std::move(vt)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using VarDeclStmtPtr = std::shared_ptr<VarDeclStmt>;

// Assign Statement
struct AssignStmt : Stmt {
  AssignElementPtr lvalue; // four kind
  ExprPtr expr;            // expression

  AssignStmt() = default;
  explicit AssignStmt(const AssignElementPtr &lv, const ExprPtr &e)
    : lvalue(lv), expr(e) {}
  explicit AssignStmt(AssignElementPtr &&lv, ExprPtr &&e)
    : lvalue(std::move(lv)), expr(std::move(e)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using AssignStmtPtr = std::shared_ptr<AssignStmt>;

// Variable Declaration Assign Statement
struct VarDeclAssignStmt : VarDeclStmt {
  ExprPtr expr;

  template <typename T1, typename T2, typename T3>
  VarDeclAssignStmt(T1 &&var, T2 &&type, T3 &&expr)
    : VarDeclStmt(std::forward<T1>(var), std::forward<T2>(type)),
      expr(std::forward<T3>(expr))
  {
  }

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using VarDeclAssignStmtPtr = std::shared_ptr<VarDeclAssignStmt>;

// Comparison Operator
enum class ComparOperator : std::uint8_t {
  Equal,  // equal to
  Nequal, // not equal to
  Gequal, // great than or equal to
  Lequal, // less than or equal to
  Great,  // great than
  Less    // less than
};

// Arithmetic Operator
enum class ArithOperator : std::uint8_t {
  Add,
  Sub,
  Mul,
  Div
};

// Comparison Expression
struct ComparExpr : Expr {
  ExprPtr lhs;       // 左部
  ComparOperator op; // operator
  ExprPtr rhs;       // 右部

  ComparExpr() = default;
  explicit ComparExpr(const ExprPtr &l, ComparOperator op, const ExprPtr &r)
    : lhs(l), op(op), rhs(r) {}
  explicit ComparExpr(ExprPtr &&l, ComparOperator op, ExprPtr &&r)
    : lhs(std::move(l)), op(op), rhs(std::move(r)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ComparExprPtr = std::shared_ptr<ComparExpr>;

// Arithmetic Expression
struct ArithExpr : Expr {
  ExprPtr lhs;      // 左操作数
  ArithOperator op; // operator
  ExprPtr rhs;      // 右操作数

  ArithExpr() = default;
  explicit ArithExpr(const ExprPtr &l, ArithOperator op, const ExprPtr &r)
    : lhs(l), op(op), rhs(r) {}
  explicit ArithExpr(ExprPtr &&l, ArithOperator op, ExprPtr &&r)
    : lhs(std::move(l)), op(op), rhs(std::move(r)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ArithExprPtr = std::shared_ptr<ArithExpr>;

// Call Expression
struct CallExpr : Expr {
  std::string callee;        // 被调用函数名
  std::vector<ExprPtr> argv; // argument vector

  CallExpr() = default;
  explicit CallExpr(const std::string &ce, const std::vector<ExprPtr> &av)
    : callee(ce), argv(av) {}
  explicit CallExpr(std::string &&ce, std::vector<ExprPtr> &&argv)
    : callee(std::move(ce)), argv(std::move(argv)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using CallExprPtr = std::shared_ptr<CallExpr>;

// else 子句
struct ElseClause : Stmt {
  std::optional<ExprPtr> expr; // else (if expr)?
  BlockStmtPtr block;

  template <typename T1, typename T2>
  ElseClause(T1 &&expr, T2 &&block)
    : expr(std::forward<T1>(expr)), block(std::forward<T2>(block)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ElseClausePtr = std::shared_ptr<ElseClause>;

// if statement
struct IfStmt : Stmt {
  ExprPtr expr;
  BlockStmtPtr if_branch;
  std::vector<ElseClausePtr> else_clauses; // else clauses

  template <typename T1, typename T2, typename T3>
  IfStmt(T1 &&e, T2 &&ib, T3 &&clauses)
    : expr(std::forward<T1>(e)), if_branch(std::forward<T2>(ib)),
      else_clauses(std::forward<T3>(clauses))
  {
  }

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using IfStmtPtr = std::shared_ptr<IfStmt>;

// while statement
struct WhileStmt : Stmt {
  ExprPtr expr;
  BlockStmtPtr block;

  template <typename T1, typename T2>
  WhileStmt(T1 &&expr, T2 &&block)
    : expr(std::forward<T1>(expr)), block(std::forward<T2>(block)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using WhileStmtPtr = std::shared_ptr<WhileStmt>;

// for statement
struct ForStmt : Stmt {
  VarDeclBodyPtr var;
  ExprPtr lexpr;
  ExprPtr rexpr;
  BlockStmtPtr block;

  template <typename T, typename T1, typename T2, typename T3>
  ForStmt(T &&var, T1 &&expr1, T2 &&expr2, T3 &&block)
    : var(std::forward<T>(var)), lexpr(std::forward<T1>(expr1)),
      rexpr(std::forward<T2>(expr2)), block(std::forward<T3>(block))
  {
  }

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ForStmtPtr = std::shared_ptr<ForStmt>;

// loop statement
struct LoopStmt : Stmt, Expr {
  BlockStmtPtr block;

  LoopStmt() = default;
  explicit LoopStmt(const BlockStmtPtr &b) : block(b) {}
  explicit LoopStmt(BlockStmtPtr &&b) : block(std::move(b)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using LoopStmtPtr = std::shared_ptr<LoopStmt>;

// break statement
struct BreakStmt : Stmt {
  std::optional<ExprPtr> expr;

  BreakStmt() = default;
  explicit BreakStmt(const std::optional<ExprPtr> &e) : expr(e) {}
  explicit BreakStmt(std::optional<ExprPtr> &&e) : expr(std::move(e)) {}
  ~BreakStmt() override = default;

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using BreakStmtPtr = std::shared_ptr<BreakStmt>;

// continue statement
struct ContinueStmt : Stmt {
  ~ContinueStmt() override = default;

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using ContinueStmtPtr = std::shared_ptr<ContinueStmt>;

// null statement => ;
struct NullStmt : Stmt {
  ~NullStmt() override = default;

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using NullStmtPtr = std::shared_ptr<NullStmt>;

// 函数表达式语句块
struct FuncExprBlockStmt : Expr, BlockStmt {
  ExprPtr expr; // the final expression

  template <typename T1, typename T2>
  FuncExprBlockStmt(T1 &&stmts, T2 &&expr)
    : BlockStmt(std::forward<T1>(stmts)), expr(std::forward<T2>(expr)) {}

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using FuncExprBlockStmtPtr = std::shared_ptr<FuncExprBlockStmt>;

struct IfExpr : Expr {
  ExprPtr condition;
  FuncExprBlockStmtPtr if_branch;
  FuncExprBlockStmtPtr else_branch;

  template <typename T1, typename T2, typename T3>
  IfExpr(T1 &&condition, T2 &&if_branch, T3 &&else_branch)
    : condition(std::forward<T1>(condition)),
      if_branch(std::forward<T2>(if_branch)),
      else_branch(std::forward<T3>(else_branch))
  {
  }

  [[nodiscard]]
  auto type() const -> NodeType override;
};
using IfExprPtr = std::shared_ptr<IfExpr>;

} // namespace parser::ast
