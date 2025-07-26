#pragma once

#include "symbol.hpp"
#include "ir_quad.hpp"

namespace ir {

class QuadFactory {
public:
  static IRQuadPtr makeFunc(std::string name);

  static IRQuadPtr makeAssign(sym::ValuePtr src, sym::ValuePtr dst);

  static IRQuadPtr makeRet(std::string funcname);
  static IRQuadPtr makeRet(sym::ValuePtr retval, std::string funcname);

  static IRQuadPtr makeGoto(std::string target);

  static IRQuadPtr makeAcc(IROp op, sym::ValuePtr base,
    sym::ValuePtr idx, sym::ValuePtr dst);

  static IRQuadPtr makeElems(IROp op, std::vector<Operand> elems,
    sym::ValuePtr dst);

  static IRQuadPtr makeOperation(IROp op, sym::ValuePtr arg1,
    sym::ValuePtr arg2, sym::ValuePtr dst);

  static IRQuadPtr makeCall(std::string callee,
    std::vector<Operand> params, sym::ValuePtr dst);

  static IRQuadPtr makeBeqz(sym::ValuePtr cond, std::string label);

  static IRQuadPtr makeBnez(sym::ValuePtr cond, std::string label);

  static IRQuadPtr makeBge(sym::ValuePtr arg1, sym::ValuePtr arg2,
    std::string label);

  static IRQuadPtr makeLabel(std::string label);
};

} // namespace ir

#include "quad_factory.inl"
