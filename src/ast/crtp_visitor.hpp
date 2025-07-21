/**
 * @file crtp_visitor.hpp
 * @brief Defines the CRTPVisitor base class template for implementing
 *        the Curiously Recurring Template Pattern (CRTP) visitor pattern.
 *
 * This header provides:
 * - The HasVisit concept to detect if a visitor
 *   implements a specific visit(NodeT&) method.
 * - The CRTPVisitor base class template, which dispatches visit calls
 *   to the derived visitor implementation if available,
 *   or provides a default (optionally debug-logged) implementation otherwise.
 *
 * Usage:
 *   Inherit from CRTPVisitor<Derived> in your visitor class,
 *   and implement visit(NodeT&) for each node type you wish to handle.
 *
 * @note If DEBUG is defined, missing visit implementations will emit a warning to stderr.
 */
#pragma once

#ifdef DEBUG
#include <print>
#endif

namespace ast {

/**
 * @brief a concept to check whether a visitor implement
 *        a specific visit method (take a AST Node& as input, and output nothing)
 *
 * @tparam Visitor CRTP visitor
 * @tparam NodeT   AST Node struct
 */
template <typename Visitor, typename NodeT>
concept HasVisit = requires(Visitor v, NodeT &node) {
  { v.visit(node) } -> std::same_as<void>;
};

template <typename Derived>
class CRTPVisitor {
public:
  virtual ~CRTPVisitor() = default;

public:
  template <typename NodeT>
  void visit(NodeT &node) {
    if constexpr (HasVisit<Derived, NodeT>) {
      // 派发给 Derived 实现的 visit(NodeT&)
      static_cast<Derived*>(this)->visit(node);
    } else {
      // 默认空实现
#ifdef DEBUG
    std::println(stderr, "[Warning] visit() not implemented for {}", typeid(NodeT).name());
#endif
    }
  }
};

} // namespace ast
