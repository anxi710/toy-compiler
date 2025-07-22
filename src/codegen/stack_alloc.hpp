#pragma once

#include <vector>
#include <fstream>

#include "symbol.hpp"

namespace cg {

// NOTE: RISC-V 要求栈始终以 16 字节对齐
//       如果想维护一个按需分配的汇编栈帧（即：手动栈帧延迟分配策略）
//       那么就需要实现一个 Stack 空间的内存分配器
//       由该分配器分配栈上存储空间，并保证 16 字节对齐的约定

// 下面给出本设计对应的栈帧布局：
// ┌────────────────────────┐ <- old sp（高地址）
// │ Caller Frame           │
// ├────────────────────────┤ <- 如果该函数不需要调用其他函数（即叶子函数）
// │ Callee's saved ra      │    则不需要保存！
// │        ...             │
// │ Local variable         │ <- 按需分配的临时变量空间（必须 16 字节对齐）
// │        ...             │
// │ Callee-saved registers │ <- 若用了 s0-s11 等需保存
// └────────────────────────┘ <- new sp（低地址）

// RISC-V 有 31 个寄存器和恒为 0 的 x0 寄存器（硬连线为 0），
// 此外还有一个独立的 pc 寄存器。根据 RISC-V ABI Specification，
// 这 31 个寄存器被分为 caller 保存寄存器和 callee 保存寄存器，
// 且有可读性更强的 ABI 名称（比如：x0 的 ABI 名称为 zero）。
//
// 其中 ra (return address), t0-t6, a0-a7 为调用者保存寄存器
// sp, s0/fp, s1-s11 为被调用者保存寄存器
// Tips: 可以发现这里少了两个寄存器，这两个寄存器是 gp (global pointer)
//       和 tp (thread pointer)，一般不会碰这两个寄存器！
//
// Q: 为什么叫调用者保存寄存器和被调用者保存寄存器？
// A: 这是因为在调用约定中，被调用者保存寄存器在过程调用前后应该保持不变
//    而调用保存寄存器则不保证这一点。所以，对于调用这保存寄存器而言，
//    如果想要在过程调用之后继续使用，则需要在 call 之前保存；
//    而如果在过程体内使用到了被调用者保存寄存器，那么必须要保存这些寄存器
//    并在过程调用返回前恢复这些寄存器，因为这些寄存器必须在过程调用前后保持不变
//    可以看到，这两个名字反应了寄存器的保存位置！
//
// 该 Allocator 支持 Spilling 策略（在寄存器不够时，将部分变量保存到栈上以腾出寄存器的一种策略）

class StackAllocator {
public:
  StackAllocator(std::ofstream &out) : out(out) {}
public:
  static constexpr std::uint8_t BLOCK_SIZE = 16; // 栈上内存以 16B/Block 的方式分配

public:
  auto alloc(int size, int align) -> int;
  [[nodiscard]] auto mark() const -> int;
  void freeTo(int mark);
  void reset();

  // Scope
  void enterFunc();
  void enterScope();
  void exitScope();

  void retFunc();

  // Spilling
  auto spill(const sym::ValuePtr &val) -> int;

  [[nodiscard]] int getFrameSize() const { return framesize; }
  [[nodiscard]] int offsetFromSP(int stackloc) const { return framesize - stackloc; }

private:
  void spMove(int delta);

private:
  std::ofstream &out;

  int frameusage = 0; // usage amount (栈帧使用量)
  int framesize  = 0; // 栈帧大小（以 16 Byte 对齐）

  int ra_addr;

  std::vector<int> scopemarks; // scope marks
};

} // namespace cg
