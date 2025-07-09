#pragma once

#include "visitor.hpp"
#include "semantic_context.hpp"

namespace ir {

class IRBuilder : public ast::BaseVisitor {
public:
  IRBuilder(sem::SemanticContext &ctx) : ctx(ctx) {}

private:
  sem::SemanticContext &ctx;
};

} // namespace ir
