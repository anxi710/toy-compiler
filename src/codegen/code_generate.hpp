#pragma once

#include <memory>
#include <vector>
#include <fstream>

namespace ast { struct Prog; }

namespace ir {

struct IRQuad;
using IRQuadPtr = std::shared_ptr<IRQuad>;

} // namespace ir

namespace sym { class SymbolTable; }

namespace cg {

class StackAllocator;
class RegAllocator;
class MemAllocator;
enum class Register : std::uint8_t;

class CodeGenerator {
public:
  CodeGenerator(std::ofstream &out, sym::SymbolTable &symtab);

public:
  void generate(const ast::Prog &prog);

private:
  void generateFunc(const std::vector<ir::IRQuadPtr> &funccode);

  void emitFunc(const ir::IRQuadPtr &code);
  void emitRet(const ir::IRQuadPtr &code);
  void emitAssign(const ir::IRQuadPtr &code);
  void emitGoto(const ir::IRQuadPtr &code);
  void emitBeqz(const ir::IRQuadPtr &code);
  void emitBnez(const ir::IRQuadPtr &code);
  void emitBge(const ir::IRQuadPtr &code);
  void emitLabel(const ir::IRQuadPtr &code);
  void emitCall(const ir::IRQuadPtr &code);

  void emitBinary(const ir::IRQuadPtr &code);
  void emitImmBinary(const ir::IRQuadPtr &code);
  inline void emitAdd(Register lhs, Register rhs, Register dst);
  inline void emitSub(Register lhs, Register rhs, Register dst);
  inline void emitMul(Register lhs, Register rhs, Register dst);
  inline void emitDiv(Register lhs, Register rhs, Register dst);
  inline void emitEq(Register lhs, Register rhs, Register dst);
  inline void emitNeq(Register lhs, Register rhs, Register dst);
  inline void emitGt(Register lhs, Register rhs, Register dst);
  inline void emitGeq(Register lhs, Register rhs, Register dst);
  inline void emitLt(Register lhs, Register rhs, Register dst);
  inline void emitLeq(Register lhs, Register rhs, Register dst);
  inline void emitImmAdd(Register lhs, int rhs, Register dst);
  inline void emitImmSub(Register lhs, int rhs, Register dst);
  inline void emitImmMul(Register lhs, int rhs, Register dst);
  inline void emitImmDiv(Register lhs, int rhs, Register dst);
  inline void emitImmEq(Register lhs, int rhs, Register dst);
  inline void emitImmNeq(Register lhs, int rhs, Register dst);
  inline void emitImmGt(Register lhs, int rhs, Register dst);
  inline void emitImmGeq(Register lhs, int rhs, Register dst);
  inline void emitImmLt(Register lhs, int rhs, Register dst);
  inline void emitImmLeq(Register lhs, int rhs, Register dst);
private:
  std::ofstream &out;
  sym::SymbolTable &symtab;

  std::unique_ptr<StackAllocator> stackalloc;
  std::unique_ptr<RegAllocator>   regalloc;
  std::unique_ptr<MemAllocator>   memalloc;
};

} // namespace cg
