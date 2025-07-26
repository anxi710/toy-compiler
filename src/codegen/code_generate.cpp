#include <print>
#include <ranges>

#include "ast.hpp"
#include "panic.hpp"
#include "asm_dbg.hpp"
#include "ir_quad.hpp"
#include "symbol_table.hpp"
#include "code_generate.hpp"

namespace cg {

CodeGenerator::CodeGenerator(std::ofstream &out,
  sym::SymbolTable &symtab) : out(out), symtab(symtab)
{
  stackalloc = std::make_unique<StackAllocator>(out);
  regalloc   = std::make_unique<RegAllocator>(out, *stackalloc);
  memalloc   = std::make_unique<MemAllocator>(out, *regalloc, *stackalloc);
}

void
CodeGenerator::generate(const ast::Prog &prog)
{
  std::println(out, "  .text");
  std::println(out, "  .align 2\n");

  for (const auto &funcdecl : prog.decls) {
    generateFunc(funcdecl->ircode);
  }
}

void
CodeGenerator::generateFunc(const std::vector<ir::IRQuadPtr> &funccode)
{
  for (const auto &code : funccode) {
    DBG(out, "  # {}", code->str());

    switch (code->op) {
      case ir::IROp::ADD: case ir::IROp::SUB:
      case ir::IROp::MUL: case ir::IROp::DIV:
      case ir::IROp::EQ:  case ir::IROp::NEQ:
      case ir::IROp::GT:  case ir::IROp::GEQ:
      case ir::IROp::LT:  case ir::IROp::LEQ:
        emitBinary(code);
        break;
      case ir::IROp::ASSIGN: emitAssign(code); break;
      case ir::IROp::GOTO:   emitGoto(code);   break;
      case ir::IROp::BEQZ:   emitBeqz(code);   break;
      case ir::IROp::BNEZ:   emitBnez(code);   break;
      case ir::IROp::BGE:    emitBge(code);    break;
      case ir::IROp::LABEL:  emitLabel(code);  break;
      case ir::IROp::CALL:   emitCall(code);   break;
      case ir::IROp::FUNC:   emitFunc(code);   break;
      case ir::IROp::RETURN: emitRet(code);    break;
      default:
        UNREACHABLE(
          std::format("unsupport ir operator {}", ir::irop2str(code->op))
        );
    }

    DBG(out, "");
  }
}

void
CodeGenerator::emitFunc(const ir::IRQuadPtr &code)
{
  std::println(out, ".global {}", code->label);
  std::println(out, "{}:", code->label);
  stackalloc->reset();
  stackalloc->enterFunc();
  regalloc->reset();
  memalloc->reset();

  auto opt_func = symtab.lookupFunc(code->label);
  CHECK(opt_func.has_value(), "can't find function symbol");
  const auto &func = opt_func.value();
  memalloc->allocArgv(func->argv);
}

void
CodeGenerator::emitRet(const ir::IRQuadPtr &code)
{
  if (code->arg1.value != nullptr) {
    DBG(out, "  # prepare return value");
    auto retval = code->arg1.value;
    if (retval->isConst()) {
      std::println(out, "  li a0, {}", retval->str());
    } else {
      auto opt_sym = memalloc->lookup(retval);
      ASSERT_MSG(opt_sym.has_value(), "can't find this symbol");
      const auto &symbol = opt_sym.value();
      if (symbol->on_stack) {
        std::println(out,
          "  lw a0, {}(sp)",
          stackalloc->offsetFromSP(symbol->stackloc)
        );
      } else {
        std::println(out,
          "  mv a0, {}",
          symbol->regloc
        );
      }
    }
  }

  stackalloc->retFunc();

  std::println(out, "  ret");
}

void
CodeGenerator::emitAssign(const ir::IRQuadPtr &code)
{
  if (code->arg1.value->isConst()) {
    auto dst = memalloc->alloc(code->dst.value, true);
    std::println(out, "  li {}, {}", dst, code->arg1.value->str());
    return;
  }

  auto src = memalloc->alloc(code->arg1.value, false);
  auto dst = memalloc->alloc(code->dst.value, true);

  std::println(out, "  mv {}, {}", dst, src);

  // DBG(out,
  //   "  # {} reuses {}'s register",
  //   code->dst.value->str(),
  //   code->arg1.value->str()
  // );
  // memalloc->reuseReg(src, code->dst.value);
}

void
CodeGenerator::emitGoto(const ir::IRQuadPtr &code)
{
  std::println(out, "  j {}", code->label);
}

void
CodeGenerator::emitBeqz(const ir::IRQuadPtr &code)
{
  auto cond = memalloc->alloc(code->arg1.value, false);
  std::println(out, "  beq {}, x0, {}", cond, code->label);
}

void
CodeGenerator::emitBnez(const ir::IRQuadPtr &code)
{
  auto cond = memalloc->alloc(code->arg1.value, false);
  std::println(out, "  bne {}, x0, {}", cond, code->label);
}

void
CodeGenerator::emitBge(const ir::IRQuadPtr &code)
{
  auto lhs = memalloc->alloc(code->arg1.value, false);
  auto rhs = memalloc->alloc(code->arg2.value, false);

  std::println(out, "  bge {}, {}, {}", lhs, rhs, code->label);
}

void
CodeGenerator::emitLabel(const ir::IRQuadPtr &code)
{
  std::println(out, "{}:", code->label);
}

void
CodeGenerator::emitCall(const ir::IRQuadPtr &code)
{
  regalloc->spillCaller();

  auto params = code->elems
    | std::views::transform([](const auto &elem) {
        return elem.value;
      })
    | std::ranges::to<std::vector>();
  memalloc->prepareParam(params);

  std::println(out, "  call {}", code->label);
  memalloc->reuseReg(Register::A0, code->dst.value);
}

/**
 * NOTE: 由于 RISC-V 中只有 SLT, SLTI, SLTU, SLTIU 这四条比较指令
 *       因此对于其他的比较运算而言，我们需要基于这四条指令来生成
 *       Tips: 这里假设 a, b 都在寄存器中！
 *
 *       1. EQ (==)
 *          a == b
 *       => xor   t0,  a, b (t0 == 0 if a == b else t0 != 0)
 *          sltiu t0, t0, 1 (t0 < 1 if and only if t0 == 0)
 *
 *       2. NEQ (!=)
 *          a != b
 *       => xor  t0,  a,  b (t0 != 0 if a != b else t0 == 0)
 *          sltu t0, x0, t0 (x0 = 0 -> t1 = 0 < t0 -> t0 != 0)
 *
 *       3. GT (>)
 *          a > b
 *       => slt t0, b, a (t0 = b < a -> t0 = a > b)
 *
 *       4. GEQ (>=)
 *          a >= b
 *       => slt  t0,  a, b (t0 == 0 if b < a else t0 == 1)
 *          xori t0, t0, 1 (if t0 == 0 then t1 == 1 -> b < a -> a >= b)
 *
 *       5. LT (<)
 *          a < b
 *       => slt t0, a, b
 *
 *       6. LEQ (<=)
 *          a <= b
 *       => slt  t0, b,  a
 *          xori t0, t0, 1
 */

static int
getConstantVal(const sym::ValuePtr &val)
{
  auto constant = std::static_pointer_cast<sym::Constant>(val);
  CHECK(constant != nullptr, "val isn't a const value");

  if (std::holds_alternative<int>(constant->val)) {
    return std::get<int>(constant->val);
  }
  return std::get<bool>(constant->val) ? 1 : 0;
}

static int
calculateConst(ir::IROp op, const ir::Operand &arg1, const ir::Operand &arg2)
{
  auto lhs = getConstantVal(arg1.value);
  auto rhs = getConstantVal(arg2.value);

  switch (op) {
    case ir::IROp::ADD: return lhs  + rhs;
    case ir::IROp::SUB: return lhs  - rhs;
    case ir::IROp::MUL: return lhs  * rhs;
    case ir::IROp::DIV: return lhs  / rhs;
    case ir::IROp::EQ:  return static_cast<int>(lhs == rhs);
    case ir::IROp::NEQ: return static_cast<int>(lhs != rhs);
    case ir::IROp::GT:  return static_cast<int>(lhs  > rhs);
    case ir::IROp::GEQ: return static_cast<int>(lhs >= rhs);
    case ir::IROp::LT:  return static_cast<int>(lhs  < rhs);
    case ir::IROp::LEQ: return static_cast<int>(lhs <= rhs);
    default:
      UNREACHABLE(
        std::format("invalid operator {}", ir::irop2str(op))
      );
  }
}

void
CodeGenerator::emitBinary(const ir::IRQuadPtr &code)
{
  if (code->arg1.isConst() && code->arg2.isConst()) {
    auto res = calculateConst(code->op, code->arg1, code->arg2);

    auto dst = memalloc->alloc(code->dst.value, true);

    std::println(out, "  li {}, {}", dst, res);
    return;
  }

  if (code->arg1.isConst() || code->arg2.isConst()) {
    emitImmBinary(code);
    return;
  }

  auto lhs = memalloc->alloc(code->arg1.value, false);
  auto rhs = memalloc->alloc(code->arg2.value, false);
  auto dst = memalloc->alloc(code->dst.value, true);

  switch (code->op) {
    case ir::IROp::ADD: emitAdd(lhs, rhs, dst); return;
    case ir::IROp::SUB: emitSub(lhs, rhs, dst); return;
    case ir::IROp::MUL: emitMul(lhs, rhs, dst); return;
    case ir::IROp::DIV: emitDiv(lhs, rhs, dst); return;
    case ir::IROp::EQ:  emitEq(lhs, rhs, dst);  return;
    case ir::IROp::NEQ: emitNeq(lhs, rhs, dst); return;
    case ir::IROp::GT:  emitGt(lhs, rhs, dst);  return;
    case ir::IROp::GEQ: emitGeq(lhs, rhs, dst); return;
    case ir::IROp::LT:  emitLt(lhs, rhs, dst);  return;
    case ir::IROp::LEQ: emitLeq(lhs, rhs, dst); return;
    default:
      UNREACHABLE(
        std::format("invalid operator {}", ir::irop2str(code->op))
      );
  }
}

void
CodeGenerator::emitImmBinary(const ir::IRQuadPtr &code)
{
  Register lhs;
  int rhs;

  if (code->arg1.isConst()) {
    lhs = memalloc->alloc(code->arg2.value, false);
    rhs = getConstantVal(code->arg1.value);
  } else {
    lhs = memalloc->alloc(code->arg1.value, false);
    rhs = getConstantVal(code->arg2.value);
  }

  auto dst = memalloc->alloc(code->dst.value, true);

  switch (code->op) {
    case ir::IROp::ADD: emitImmAdd(lhs, rhs, dst); return;
    case ir::IROp::SUB: emitImmSub(lhs, rhs, dst); return;
    case ir::IROp::MUL: emitImmMul(lhs, rhs, dst); return;
    case ir::IROp::DIV: emitImmDiv(lhs, rhs, dst); return;
    case ir::IROp::EQ:  emitImmEq(lhs, rhs, dst);  return;
    case ir::IROp::NEQ: emitImmNeq(lhs, rhs, dst); return;
    case ir::IROp::GT:
      if (code->arg2.isConst()) {
        emitImmGt(lhs, rhs, dst);
      } else {
        emitImmLt(lhs, rhs, dst);
      }
      return;
    case ir::IROp::GEQ:
      if (code->arg2.isConst()) {
        emitImmGeq(lhs, rhs, dst);
      } else {
        emitImmLeq(lhs, rhs, dst);
      }
      return;
    case ir::IROp::LT:
      if (code->arg2.isConst()) {
        emitImmLt(lhs, rhs, dst);
      } else {
        emitImmGt(lhs, rhs, dst);
      }
      return;
    case ir::IROp::LEQ:
      if (code->arg2.isConst()) {
        emitImmLeq(lhs, rhs, dst);
      } else {
        emitImmGeq(lhs, rhs, dst);
      }
      return;
    default:
      UNREACHABLE(
        std::format("invalid operator {}", ir::irop2str(code->op))
      );
  }
}

void
CodeGenerator::emitAdd(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  add {}, {}, {}", dst, lhs, rhs);
}

void
CodeGenerator::emitSub(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  sub {}, {}, {}", dst, lhs, rhs);
}

void
CodeGenerator::emitMul(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  mul {}, {}, {}", dst, lhs, rhs);
}

void
CodeGenerator::emitDiv(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  div {}, {}, {}", dst, lhs, rhs);
}

void
CodeGenerator::emitEq(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  xor {}, {}, {}", dst, lhs, rhs);
  std::println(out, "  sltiu {}, {}, 1", dst, dst);
}

void
CodeGenerator::emitNeq(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  xor {}, {}, {}", dst, lhs, rhs);
  std::println(out, "  sltu {}, x0, {}", dst, dst);
}

void
CodeGenerator::emitGt(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  slt {}, {}, {}", dst, rhs, lhs);
}

void
CodeGenerator::emitGeq(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  slt {}, {}, {}", dst, lhs, rhs);
  std::println(out, "  xori {}, {}, 1", dst, dst);
}

void
CodeGenerator::emitLt(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  slt {}, {}, {}", dst, lhs, rhs);
}

void
CodeGenerator::emitLeq(Register lhs, Register rhs, Register dst)
{
  std::println(out, "  slt {}, {}, {}", dst, rhs, lhs);
  std::println(out, "  xori {}, {}, 1", dst, dst);
}

void
CodeGenerator::emitImmAdd(Register lhs, int rhs, Register dst)
{
  std::println(out, " addi {}, {}, {}", dst, lhs, rhs);
}

void
CodeGenerator::emitImmSub(Register lhs, int rhs, Register dst)
{
  int neg_rhs = -rhs;
  std::println(out, "  addi {}, {}, {}", dst, lhs, neg_rhs);
}

void
CodeGenerator::emitImmMul(Register lhs, int rhs, Register dst)
{
  std::println(out, "  li {}, {}", dst, rhs);
  std::println(out, "  mul {}, {}, {}", dst, lhs, dst);
}

void
CodeGenerator::emitImmDiv(Register lhs, int rhs, Register dst)
{
  std::println(out, "  li {}, {}", dst, rhs);
  std::println(out, "  div {}, {}, {}", dst, lhs, dst);
}

void
CodeGenerator::emitImmEq(Register lhs, int rhs, Register dst)
{
  std::println(out, "  xor {}, {}, {}", dst, lhs, rhs);
  std::println(out, "  sltiu {}, {}, 1", dst, dst);
}

void
CodeGenerator::emitImmNeq(Register lhs, int rhs, Register dst)
{
  std::println(out, "  xori {}, {}, {}", dst, lhs, rhs);
  std::println(out, "  sltu {}, x0, {}", dst, dst);
}

void
CodeGenerator::emitImmGt(Register lhs, int rhs, Register dst)
{
  std::println(out, "  li {}, {}", dst, rhs);
  std::println(out, "  slt {}, {}, {}", dst, dst, lhs);
}

void
CodeGenerator::emitImmGeq(Register lhs, int rhs, Register dst)
{
  std::println(out, "  slti {}, {}, {}", dst, lhs, rhs);
  std::println(out, "  xori {}, {}, 1", dst, dst);
}

void
CodeGenerator::emitImmLt(Register lhs, int rhs, Register dst)
{
  std::println(out, "  slti {}, {}, {}", dst, lhs, rhs);
}

void
CodeGenerator::emitImmLeq(Register lhs, int rhs, Register dst)
{
  std::println(out, "  li {}, {}", dst, rhs);
  std::println(out, "  slt {}, {}, {}", dst, dst, lhs);
  std::println(out, "  xori {}, {}, 1", dst, dst);
}

} // namespace cg
