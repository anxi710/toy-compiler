#pragma once

namespace ir {

inline IRQuadPtr
QuadFactory::makeFunc(std::string name)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::FUNC;
  quad->label = std::move(name);
  return quad;
}

inline IRQuadPtr
QuadFactory::makeAssign(sym::ValuePtr src, sym::ValuePtr dst)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::ASSIGN;
  quad->arg1 = Operand{std::move(src)};
  quad->dst = Operand{std::move(dst)};
  return quad;
}

inline IRQuadPtr
QuadFactory::makeRet(std::string funcname)
{
  return makeRet(nullptr, std::move(funcname));
}

inline IRQuadPtr
QuadFactory::makeRet(sym::ValuePtr retval, std::string funcname)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::RETURN;
  if (retval) {
    quad->arg1 = Operand{std::move(retval)};
  }
  quad->label = std::move(funcname);
  return quad;
}

inline IRQuadPtr
QuadFactory::makeGoto(std::string target)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::GOTO;
  quad->label = std::move(target);
  return quad;
}

inline IRQuadPtr
QuadFactory::makeAcc(IROp op, sym::ValuePtr base,
  sym::ValuePtr idx, sym::ValuePtr dst)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = op;
  quad->arg1 = Operand{std::move(base)};
  quad->arg2 = Operand{std::move(idx)};
  quad->dst = Operand{std::move(dst)};
  return quad;
}

inline IRQuadPtr
QuadFactory::makeElems(IROp op, std::vector<Operand> elems,
  sym::ValuePtr dst)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = op;
  quad->elems = std::move(elems);
  quad->dst = Operand{std::move(dst)};
  return quad;
}

inline IRQuadPtr
QuadFactory::makeOperation(IROp op, sym::ValuePtr arg1,
  sym::ValuePtr arg2, sym::ValuePtr dst)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = op;
  quad->arg1 = Operand{std::move(arg1)};
  quad->arg2 = Operand{std::move(arg2)};
  quad->dst = Operand{std::move(dst)};
  return quad;
}

inline IRQuadPtr
QuadFactory::makeCall(std::string callee,
  std::vector<Operand> params, sym::ValuePtr dst)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::CALL;
  quad->elems = std::move(params);
  quad->dst = Operand{std::move(dst)};
  quad->label = std::move(callee);
  return quad;
}

inline IRQuadPtr
QuadFactory::makeBeqz(sym::ValuePtr cond, std::string label)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::BEQZ;
  quad->arg1 = Operand{std::move(cond)};
  quad->label = std::move(label);
  return quad;
}

inline IRQuadPtr
QuadFactory::makeBnez(sym::ValuePtr cond, std::string label)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::BNEZ;
  quad->arg1 = Operand{std::move(cond)};
  quad->label = std::move(label);
  return quad;
}

inline IRQuadPtr
QuadFactory::makeBge(sym::ValuePtr arg1, sym::ValuePtr arg2,
  std::string label)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::BGE;
  quad->arg1 = Operand{std::move(arg1)};
  quad->arg2 = Operand{std::move(arg2)};
  quad->label = std::move(label);
  return quad;
}

inline IRQuadPtr
QuadFactory::makeLabel(std::string label)
{
  auto quad = std::make_shared<IRQuad>();
  quad->op = IROp::LABEL;
  quad->label = std::move(label);
  return quad;
}

} // namespace ir
