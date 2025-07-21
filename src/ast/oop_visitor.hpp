/**
 * @file oop_visitor.hpp
 * @brief Defines the OOPVisitor and BaseVisitor classes for the AST visitor pattern.
 *
 * This file contains the abstract base class `OOPVisitor` which declares pure virtual
 * visit methods for each AST node type. The `BaseVisitor` class provides default
 * (empty) implementations for all visit methods, allowing derived visitors to override
 * only the methods they need.
 *
 * Usage:
 * - Inherit from `BaseVisitor` to implement custom AST traversals or analyses.
 * - Override only the visit methods relevant to your use case.
 *
 * Classes:
 * - ast::OOPVisitor: Abstract visitor interface for all AST node types.
 * - ast::BaseVisitor: Concrete visitor with default no-op implementations.
 */
#pragma once

#define OOP_VISIT_NODES(DEFINE_NODE) \
  DEFINE_NODE(Prog) \
  DEFINE_NODE(Type) \
  DEFINE_NODE(Arg) \
  DEFINE_NODE(StmtBlockExpr) \
  DEFINE_NODE(FuncHeaderDecl) \
  DEFINE_NODE(FuncDecl) \
  DEFINE_NODE(ExprStmt) \
  DEFINE_NODE(EmptyExpr) \
  DEFINE_NODE(BracketExpr) \
  DEFINE_NODE(AssignElem) \
  DEFINE_NODE(Variable) \
  DEFINE_NODE(ArrAcc) \
  DEFINE_NODE(TupAcc) \
  DEFINE_NODE(Number) \
  DEFINE_NODE(ArrElems) \
  DEFINE_NODE(TupElems) \
  DEFINE_NODE(RetExpr) \
  DEFINE_NODE(VarDeclStmt) \
  DEFINE_NODE(AssignExpr) \
  DEFINE_NODE(CmpExpr) \
  DEFINE_NODE(AriExpr) \
  DEFINE_NODE(CallExpr) \
  DEFINE_NODE(ElseClause) \
  DEFINE_NODE(IfExpr) \
  DEFINE_NODE(WhileLoopExpr) \
  DEFINE_NODE(RangeExpr) \
  DEFINE_NODE(IterableVal) \
  DEFINE_NODE(ForLoopExpr) \
  DEFINE_NODE(LoopExpr) \
  DEFINE_NODE(BreakExpr) \
  DEFINE_NODE(ContinueExpr) \
  DEFINE_NODE(EmptyStmt)

namespace ast {

#define FORWARD_DECLARE(name) struct name;
  OOP_VISIT_NODES(FORWARD_DECLARE)
#undef FORWARD_DECLARE

class OOPVisitor {
public:
  virtual ~OOPVisitor() = default;

public:
#define VIRTUAL_DECLARE(name) virtual void visit(name&) = 0;
  OOP_VISIT_NODES(VIRTUAL_DECLARE)
#undef VIRTUAL_DECLARE
};

class BaseVisitor : public OOPVisitor {
public:
  ~BaseVisitor() override = default;

public:
#define DEFAULT_IMPL(name) void visit(name &node) override {};
  OOP_VISIT_NODES(DEFAULT_IMPL)
#undef DEFAULT_IMPL
};

} // namespace ast
