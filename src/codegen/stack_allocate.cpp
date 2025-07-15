#include <print>

#include "type.hpp"
#include "panic.hpp"
#include "stack_allocate.hpp"

namespace cg {

// NOTE: RISC-V 和多数现代 CPU 要求数据对齐，因为需要避免非对齐内存访问

/**
 * @brief  向上对齐（正数）/ 向下对齐（负数）
 * @param  x     待对齐值
 * @param  align 对齐基准值
 * @return 对齐后的 x
 */
constexpr int
ceilAlign(int x, int align)
{
  return ((x + align - 1) / align) * align;
}

/**
 * @brief  向下对齐（正数）/ 向上对齐（负数）
 * @param  x     待对齐值
 * @param  align 对齐基准值
 * @return 对齐后的 x
 */
constexpr int
floorAlign(int x, int align)
{
  return (x / align) * align;
}

/**
 * @brief  在栈上分配 size 字节的内存，为避免 misaligned access 错误，
 *         需要先将栈帧中已分配的区域对齐至 size 的整数倍，具体可由 align 设置
 * @param  size  分配的字节数
 * @param  align 对齐基准值
 * @return 分配到的栈帧位置（从栈帧底部开始计算）
 */
int
StackAllocator::alloc(int size, int align)
{
  ASSERT_MSG(
    size > 0 && align >= size && align % size == 0,
    "分配字节数必须大于 0，且栈帧对齐基准值必须为 size 的整数倍"
  );

  int newframeusage = ceilAlign(frameusage, align);
  int addr = newframeusage;

  // 栈帧已分配内存不足，则按需扩展
  if (newframeusage + size > framesize) {
    int newframesize = framesize;

    while (newframesize < newframeusage + size) {
      newframesize += BLOCK_SIZE;
    }

    spMove(newframesize - framesize);
    framesize = newframesize;
  }

  frameusage = newframeusage + size;
  return addr;
}

/**
 * @brief   sp 指针向下移动 delta Byte
 * @details 栈帧必须以 16 字节对齐，因此实际分配前需要对齐 delta
 * @param   delta 增长字节数
 */
void
StackAllocator::spMove(int delta)
{
  // if delta > 0
  // e.g. delta = 2
  //      ceilAlign(delta, BLOCK_SIZE)
  //    = (2 + 16 - 1) / 16 * 16
  //    = 16
  // if delta < 0
  // e.g. delta = -17
  //      ceilAlign(delta, BLOCK_SIZE)
  //    = (-17 + 16 - 1) / 16 * 16
  //    = 0 !!!
  // e.g. delta = -17
  //      floorAlign(delta, BLOCK_SIZE)
  //    = -17 / 16 * 16
  //    = -16
  int delta_aligned =
    (delta > 0) ? ceilAlign(delta, BLOCK_SIZE) : floorAlign(delta, BLOCK_SIZE);
  framesize += delta_aligned;

  // 栈帧是向下增长的因此是移动一个负数！
#ifdef VERBOSE
  std::println(out, "");
  std::println(out, "  # stack grow: {} bytes", delta_aligned);
  std::println(out, "  addi, sp, sp, {}", -delta_aligned);
  std::println(out, "");
#else
  std::println(out, "  addi, sp, sp, {}", -delta_aligned);
#endif
}

/**
 * @brief 返回当前栈帧使用量，作为 freeTo() 的标记
 */
int
StackAllocator::mark() const
{
  return frameusage;
}

void
StackAllocator::freeTo(int mark)
{
  ASSERT_MSG(
    mark <= frameusage,
    "必须是释放到一个比当前栈帧使用量小的值！"
  );

  frameusage = mark;

  int delta = framesize - frameusage;
  if (framesize - frameusage >= BLOCK_SIZE) {
    spMove(-delta);
  }
}

void
StackAllocator::reset()
{
  if (framesize > 0) {
    spMove(-framesize); // reset sp
  }

  // 重置各数据成员
  frameusage = 0;
  framesize = 0;
  spillslot.clear();
  scopemarks.clear();
}

int
StackAllocator::offset(int addr) const
{
  return framesize - addr;
}

void
StackAllocator::enterScope()
{
  scopemarks.push_back(mark());
}

void
StackAllocator::exitScope()
{
  ASSERT_MSG(
    !scopemarks.empty(),
    "There is no scope to exit."
  );
  freeTo(scopemarks.back());
  scopemarks.pop_back();
}

int
StackAllocator::spill(const sym::ValuePtr &val)
{
  if (spillslot.contains(val)) {
    return spillslot[val];
  }

  int addr = alloc(val->type->size(), val->type->size());
  spillslot[val] = addr;

  return addr;
}

} // namespace cg
