#pragma once

#include "panic.hpp"
#include "visitor.hpp"
#include "semantic_context.hpp"

namespace err {

class ErrReporter;

} // namespace err

namespace sem {

class VarRefChecker : public ast::BaseVisitor {
public:
  VarRefChecker(const SemanticContext &ctx) : ctx(ctx) {}
  ~VarRefChecker() override = default;

public:
  void visit(ast::Variable &var) override {
    auto opt_var = ctx.lookupVal(var.name);
    if (opt_var.has_value()) {
      init = opt_var.value()->init;
    } else {
      init = true;
    }
  }

public:
  bool init;

private:
  const SemanticContext &ctx;
};

} // namespace sem
