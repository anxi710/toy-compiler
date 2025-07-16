#pragma once

#include <memory>

#include "ir_builder.hpp"
#include "symbol_table.hpp"
#include "semantic_checker.hpp"

namespace err {

class ErrReporter;

} // namespace err

namespace par {

class SemanticIRBuilder {
public:
  SemanticIRBuilder(sym::SymbolTable &symtab, err::ErrReporter &reporter)
    : ctx(std::make_unique<sem::SemanticContext>(symtab)), reporter(reporter),
      sema(*ctx, reporter), ir(*ctx) {}
  ~SemanticIRBuilder() = default;

public:
  template <typename NodeT>
  void build(NodeT &node) {
    if constexpr (ast::HasVisit<sem::SemanticChecker, NodeT>) {
      sema.visit(node);
    }

    if constexpr (ast::HasVisit<ir::IRBuilder, NodeT>) {
      if (!reporter.hasErrs()) {
        ir.visit(node);
      }
    }
  }

public:
  std::unique_ptr<sem::SemanticContext> ctx;

private:
  err::ErrReporter     &reporter;
  sem::SemanticChecker  sema;
  ir::IRBuilder         ir;
};

} // namespace par
