#pragma once

#include "symbol.hpp"
#include "ir_quad.hpp"

namespace ir {

class QuadFactory {
public:
  static IRQuadPtr makeFunc(std::string name) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::FUNC;
    quad->label = std::move(name);
    return quad;
  }

  static IRQuadPtr makeAssign(sym::ValuePtr src, sym::ValuePtr dst) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::ASSIGN;
    quad->arg1 = Operand{src};
    quad->dst = Operand{dst};
    return quad;
  }

  static IRQuadPtr makeRet(std::string funcname) {
    return makeRet(nullptr, std::move(funcname));
  }

  static IRQuadPtr makeRet(sym::ValuePtr retval, std::string funcname) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::RETURN;
    if (retval) {
      quad->arg1 = Operand{retval};
    }
    quad->label = std::move(funcname);
    return quad;
  }

  static IRQuadPtr makeGoto(std::string target) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::GOTO;
    quad->label = std::move(target);
    return quad;
  }

  static IRQuadPtr makeAcc(IROp op, sym::ValuePtr base,
    sym::ValuePtr idx, sym::ValuePtr dst)
  {
    auto quad = std::make_shared<IRQuad>();
    quad->op = op;
    quad->arg1 = Operand{base};
    quad->arg2 = Operand{idx};
    quad->dst = Operand{dst};
    return quad;
  }

  static IRQuadPtr makeElems(IROp op,
    std::vector<Operand> elems, sym::ValuePtr dst)
  {
    auto quad = std::make_shared<IRQuad>();
    quad->op = op;
    quad->elems = std::move(elems);
    quad->dst = Operand{dst};
    return quad;
  }

  static IRQuadPtr makeOperation(IROp op, sym::ValuePtr arg1,
    sym::ValuePtr arg2, sym::ValuePtr dst)
  {
    auto quad = std::make_shared<IRQuad>();
    quad->op = op;
    quad->arg1 = Operand{arg1};
    quad->arg2 = Operand{arg2};
    quad->dst = Operand{dst};
    return quad;
  }

  static IRQuadPtr makeCall(std::string callee, sym::ValuePtr dst) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::CALL;
    quad->dst = Operand{dst};
    quad->label = std::move(callee);
    return quad;
  }

  static IRQuadPtr makeParam(sym::ValuePtr param) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::PARAM;
    quad->arg1 = Operand{param};
    return quad;
  }

  static IRQuadPtr makeBnez(sym::ValuePtr cond, std::string label) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::BNEZ;
    quad->arg1 = Operand{cond};
    quad->label = label;
    return quad;
  }

  static IRQuadPtr makeBlt(sym::ValuePtr arg1, sym::ValuePtr arg2, std::string label) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::BLT;
    quad->arg1 = Operand{arg1};
    quad->arg2 = Operand{arg2};
    quad->label = label;
    return quad;
  }

  static IRQuadPtr makeLabel(std::string label) {
    auto quad = std::make_shared<IRQuad>();
    quad->op = IROp::LABEL;
    quad->label = std::move(label);
    return quad;
  }
};

} // namespace ir
