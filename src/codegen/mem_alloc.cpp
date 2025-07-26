#include <print>
#include <ranges>

#include "panic.hpp"
#include "symbol.hpp"
#include "asm_dbg.hpp"
#include "mem_alloc.hpp"
#include "reg_alloc.hpp"
#include "stack_alloc.hpp"

namespace cg {

Register
MemAllocator::alloc(const sym::ValuePtr &val, bool be_assigned)
{
  ASSERT_MSG(
    val->kind != sym::Value::Kind::CONST,
    "alloc register for constant"
  );

  if (symtab.contains(val->str())) {
    SymbolPtr symbol = symtab[val->str()];

    if (symbol->in_reg && be_assigned) {
      regalloc.spillExcept(symbol);
    }

    load(symbol);

    if (symbol->on_stack && be_assigned) {
      symbol->dirty = true;
    }
    return symbol->regloc;
  }

  auto symbol = std::make_shared<Symbol>();
  symbol->val      = val;
  symbol->on_stack = false;
  symbol->in_reg   = true;
  symbol->dirty    = false;
  symbol->regloc   = regalloc.alloc(symbol);

  symtab[val->str()] = symbol;
  return symbol->regloc;
}

void
MemAllocator::reuseReg(Register reg, const sym::ValuePtr &val)
{
  SymbolPtr symbol;
  if (const auto &it = symtab.find(val->str());
    it != symtab.end())
  {
    symbol = it->second;
    if (symbol->in_reg) {
      // 释放掉原符号占据的 register
      regalloc.free(symbol);
    }
    if (symbol->on_stack) {
      symbol->dirty = true;
    }
  } else {
    symbol = std::make_shared<Symbol>();
    symbol->on_stack = false;
    symbol->dirty = false;
    symbol->val = val;

    symtab[symbol->val->str()] = symbol;
  }

  regalloc.reuse(reg, symbol);
}

void
MemAllocator::load(const SymbolPtr &symbol)
{
  if (symbol->in_reg) {
    return;
  }

  CHECK(symbol->on_stack, "symbol not on stack!");

  symbol->regloc = regalloc.alloc(symbol);
  symbol->in_reg = true;
  symbol->dirty  = false;

  DBG(out, "  # load symbol {}", symbol->val->str());

  int offset = stackalloc.offsetFromSP(symbol->stackloc);
  std::println(out, "  lw {}, {}(sp)", symbol->regloc, offset);
}

std::optional<SymbolPtr>
MemAllocator::lookup(const sym::ValuePtr &val)
{
  if (const auto &it = symtab.find(val->str());
    it != symtab.end())
  {
    return it->second;
  }
  return std::nullopt;
}

void
MemAllocator::allocArgv(const std::vector<sym::ValuePtr> &argv)
{
  CHECK(argv.size() <= 8, "argument > 8");
  for (const auto &[idx, arg] : std::ranges::enumerate_view(argv)) {
    const auto &reg = alloc(arg, true);
    ASSERT_MSG(reg == toReg(idx), "argument register error");
  }
}

void
MemAllocator::prepareParam(const std::vector<sym::ValuePtr> &params)
{
  CHECK(params.size() <= 8, "parameter > 8");
  for (const auto &[idx, param] : std::ranges::enumerate_view(params)) {
    if (param->isConst()) {
      std::println(out, "  li {}, {}", toReg(idx), param->str());
    } else {
      const auto &it = symtab.find(param->str());
      ASSERT_MSG(it != symtab.end(), "can't find param symbol");
      auto symbol = it->second;
      ASSERT_MSG(symbol->on_stack, "symbol don't on stack");
      std::println(out,
        "  lw {}, {}(sp)",
        toReg(idx),
        stackalloc.offsetFromSP(symbol->stackloc)
      );
    }
  }
}

} // namespace cg
