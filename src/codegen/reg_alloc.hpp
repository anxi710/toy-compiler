#pragma once

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <optional>
#include <unordered_map>

#include "riscv_reg.hpp"

namespace sym {

struct Value;
using ValuePtr = std::shared_ptr<Value>;

} // namespace sym

namespace cg {

class StackAllocator;

struct Symbol {
  sym::ValuePtr val; // semantic check 阶段搜集的 symbol
  bool on_stack;     // 是否在栈上分配了空间
  bool in_reg;       // 是否分配了寄存器
  bool dirty;        // 如果即在栈上分配了空间，又分配了存储器
                     // 则可能出现二者保存的数据不一致的情况
                     // 且通常是寄存器中的数据更新
                     // 所以标记 dirty 意味着 stack 上的数据不能直接使用！
  int      stackloc;
  Register regloc;
};
using SymbolPtr = std::shared_ptr<Symbol>;

class RegAllocator {
public:
  RegAllocator(std::ofstream &out, StackAllocator &stackalloc)
    : out(out), stackalloc(stackalloc) {}

private:
  using SymPool = std::unordered_map<std::string, SymbolPtr>;

public:
  void reset() {
    regpool.clear();
    regpool.resize(AVAILABLE_REG_CNT);
    used_callee.clear();
  }

  auto alloc(const SymbolPtr &symbol) -> Register;
  void reuse(Register reg, const SymbolPtr &symbol);

  void free(Register reg);
  void free(const SymbolPtr &symbol);

  void spillCaller();
  void restoreUsedCallee();

  void spillExcept(const SymbolPtr &symbol);

private:
  auto spill() -> Register;

  auto allocReg() -> std::tuple<bool, std::optional<Register>>;

  auto spillReg(Register reg) -> int;
  void spillSymbolIn(Register reg);

private:
  std::ofstream  &out;
  StackAllocator &stackalloc;

  std::vector<SymPool> regpool;
  Register spill_reg = Register::A0;

  struct SavedCallee {
    Register reg;
    int      stackloc;
  };
  std::unordered_map<Register, SavedCallee> used_callee;
};

} // namespace cg
