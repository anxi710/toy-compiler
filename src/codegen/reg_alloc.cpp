#include <print>
#include <ranges>

#include "panic.hpp"
#include "asm_dbg.hpp"
#include "reg_alloc.hpp"
#include "stack_alloc.hpp"

namespace cg {

/**
 * @brief 分配一个寄存器
 *
 * 优先分配 caller saved 寄存器，如果 caller saved 寄存器都被使用了
 * 则分配 callee saved 寄存器。如果所有寄存器都被分配了，则 spill 一个
 * 寄存器到栈中
 *
 * @return Register 分配到的寄存器
 */
Register
RegAllocator::alloc(const SymbolPtr &symbol)
{
  Register alloced_reg;
  if (auto [success, reg] = allocReg(); success) {
    alloced_reg = reg.value();
  } else {
    alloced_reg = spill();
  }

  regpool[toIndex(alloced_reg)][symbol->val->str()] = symbol;

  return alloced_reg;
}

void
RegAllocator::reuse(Register reg, const SymbolPtr &symbol)
{
  auto &sympool = regpool[toIndex(reg)];
  sympool[symbol->val->str()] = symbol;
}

/**
 * @brief 分配一个 register
 *
 * @return std::tuple<bool, std::optional<Register>>
 *         是否分配成功，以及分配到的 register
 */
std::tuple<bool, std::optional<Register>>
RegAllocator::allocReg()
{
  for (const auto &[idx, sympool] : std::ranges::enumerate_view(regpool)) {
    // 找到第一个没有被使用的 register
    Register reg = toReg(idx);
    if (sympool.empty()) {
      if (isCaller(reg)) {
        return {true, reg};
      }
      // 如果是第一次使用这个 callee saved register
      // 则需要将其 spill 到栈，保留原始值，然后在
      // 函数 return 前统一恢复其值
      if (!used_callee.contains(reg)) {
        int stackloc = spillReg(reg);
        used_callee[reg] = {.reg = reg, .stackloc = stackloc};
      }
      return {true, reg};
    }
  }
  return {false, std::nullopt};
}

/**
 * @brief 将一个 register 中的值 spill 到栈中保存起来
 *
 * 一般而言，只有 callee saved register 会有这个需要！
 *
 * @param reg 待 spill 的 register
 * @return int 栈上的存储位置（相对于栈底）
 */
int
RegAllocator::spillReg(Register reg)
{
  DBG(out, "  # spill register {}", reg);

  int stackloc = stackalloc.alloc(REG_SIZE, REG_SIZE);
  int offset = stackalloc.offsetFromSP(stackloc);

  std::println(out, "  sd {}, {}(sp)", reg, offset);

  return stackloc;
}

/**
 * @brief 将 register 对应的 symbol pool 中的符号 spill 到栈中
 *
 * @param reg 需要 spill symbol 的 register
 */
void
RegAllocator::spillSymbolIn(Register reg)
{
  RegAllocator::SymPool &sympool = regpool[toIndex(reg)];

  DBG(out, "  # spill symbol in register {}", reg);

  // 如果寄存器对应的符号池中没有符号，则直接返回
  if (sympool.empty()) {
    return;
  }

  for (auto &sympair : sympool) {
    // 对于 symbol pool 中的每个 symbol
    // 如果其没有在栈上分配空间，或者分配了且有被修改
    // 则将寄存器中的值写回栈上
    auto symbol = sympair.second;
    CHECK(symbol != nullptr, "can't find symbol");

    // 如果该符号已经在栈上分配了空间，
    // 并且寄存器中的值并没有被修改，则不需要写回
    if (symbol->on_stack && !symbol->dirty) {
      continue;
    }

    if (!symbol->on_stack) {
      // 没在栈上分配空间则分配
      symbol->stackloc = stackalloc.spill(symbol->val);
      symbol->on_stack = true;
    }

    int delta = stackalloc.getFrameSize() - symbol->stackloc;
    std::println(out, "  sw {}, {}(sp)", reg, delta);
    symbol->dirty    = false;
    symbol->in_reg   = false;
  }

  sympool.clear();
}

void
RegAllocator::spillExcept(const SymbolPtr &symbol)
{
  RegAllocator::SymPool &sympool = regpool[toIndex(symbol->regloc)];

  DBG(out,
    "  # spill symbol in register {} except {}",
    symbol->regloc,
    symbol->val->str()
  );

  for (auto &sympair : sympool) {
    if (sympair.first == symbol->val->str()) {
      continue;
    }

    // 对于 symbol pool 中的每个 symbol
    // 如果其没有在栈上分配空间，或者分配了且有被修改
    // 则将寄存器中的值写回栈上
    auto symbol = sympair.second;
    CHECK(symbol != nullptr, "can't find symbol");

    // 如果该符号已经在栈上分配了空间，
    // 并且寄存器中的值并没有被修改，则不需要写回
    if (symbol->on_stack && !symbol->dirty) {
      continue;
    }

    if (!symbol->on_stack) {
      // 没在栈上分配空间则分配
      symbol->stackloc = stackalloc.spill(symbol->val);
      symbol->on_stack = true;
    }

    int delta = stackalloc.getFrameSize() - symbol->stackloc;
    std::println(out, "  sw {}, {}(sp)", symbol->regloc, delta);
    symbol->dirty    = false;
    symbol->in_reg   = false;
  }

  sympool.clear();
  sympool[symbol->val->str()] = symbol;
}

/**
 * @brief 将 caller saved register 中的符号写回栈上
 */
void
RegAllocator::spillCaller()
{
  for (const auto &reg : CALLER_SAVED_REGS) {
    spillSymbolIn(reg);
  }
}

/**
 * @brief 恢复使用过的 callee saved register 中的值
 */
void
RegAllocator::restoreUsedCallee()
{
  for (const auto &callee_pair : used_callee) {
    const auto &callee = callee_pair.second;
    DBG(out,"  # restore register {}", callee.reg);
    std::println(out, "  ld {}, {}(sp)", callee.reg, callee.stackloc);
  }
}

/**
 * @brief 在所有寄存器都被使用了的情况下，按照指定算法
 *        选择一个寄存器，将其 spill 到栈上（即将其
 *        symbol pool 中的符号写回栈上）
 *
 * @return Register spill 的寄存器
 */
Register
RegAllocator::spill()
{
  ASSERT_MSG(!regpool[toIndex(spill_reg)].empty(), "spill register ");

  Register ret = spill_reg;
  spillSymbolIn(spill_reg);

  int idx = (toIndex(spill_reg) + 1) % AVAILABLE_REG_CNT;
  spill_reg = toReg(idx);

  return ret;
}

/**
 * @brief 释放一个寄存器
 *
 * @param reg 待释放的寄存器
 */
void
RegAllocator::free(Register reg)
{
  spillSymbolIn(reg); // 释放一个寄存器只需要将符号池中的符号 spill 到栈上
}

void
RegAllocator::free(const SymbolPtr &symbol)
{
  DBG(out, "  # free symbol {}", symbol->val->str());

  if (!symbol->in_reg) {
    DBG(out, "  # this symbol is not in a register");
    return;
  }

  if (symbol->on_stack && symbol->dirty) {
    std::println(out,
      "  sw {}, {}(sp)",
      symbol->regloc,
      stackalloc.offsetFromSP(symbol->stackloc)
    );
  }

  auto &sympool = regpool[toIndex(symbol->regloc)];
  sympool.erase(sympool.find(symbol->val->str()));
}

} // namespace cg
