#pragma once

#include "visitor.hpp"

namespace visit {

class SpecialStmtScanner : public BaseVisitor {
public:
  ~SpecialStmtScanner() override = default;

public:
  void visit(ast::BlockStmt &bstmt) override;

public:
  bool has_ret = false;
  bool has_break = false;
  bool has_continue = false;
};

} // namespace visit
