/// @class SemanticIRBuilder
/// @brief Builds the semantic intermediate representation (IR) for AST nodes, performing semantic checks and IR generation.
///
/// This class coordinates semantic analysis and IR building for AST nodes. It uses a symbol table and error reporter,
/// and maintains a semantic context. For each node, it first performs semantic checking, and if no errors are reported,
/// proceeds to IR generation. The class is intended to be used within the `par` namespace for parsing and semantic IR building.
///
/// @note The class template method `build` dispatches to the appropriate visitor methods for semantic checking and IR building,
/// depending on the node type.
///
/// @see sym::SymbolTable
/// @see err::ErrReporter
/// @see sem::SemanticChecker
/// @see ir::IRBuilder
#pragma once

#include <memory>

#include "err_report.hpp"
#include "ir_builder.hpp"
#include "symbol_table.hpp"
#include "semantic_checker.hpp"
#include "semantic_context.hpp"

/// @brief The par namespace contains components related to parsing and semantic IR building.
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
