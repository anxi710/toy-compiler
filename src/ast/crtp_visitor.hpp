#pragma once

#include "ast.hpp"

#ifdef DEBUG
#include <print>
#endif

namespace ast {

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
