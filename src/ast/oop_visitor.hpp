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

#define OOP_VISIT_NODES(_) \
  _(Prog) \
  _(Type) \
  _(Arg) \
  _(StmtBlockExpr) \
  _(FuncHeaderDecl) \
  _(FuncDecl) \
  _(ExprStmt) \
  _(EmptyExpr) \
  _(BracketExpr) \
  _(AssignElem) \
  _(Variable) \
  _(ArrAcc) \
  _(TupAcc) \
  _(Number) \
  _(ArrElems) \
  _(TupElems) \
  _(RetExpr) \
  _(VarDeclStmt) \
  _(AssignExpr) \
  _(CmpExpr) \
  _(AriExpr) \
  _(CallExpr) \
  _(ElseClause) \
  _(IfExpr) \
  _(WhileLoopExpr) \
  _(RangeExpr) \
  _(IterableVal) \
  _(ForLoopExpr) \
  _(LoopExpr) \
  _(BreakExpr) \
  _(ContinueExpr) \
  _(EmptyStmt)

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
