#pragma once

#include <memory>
#include <vector>
#include <fstream>
#include <optional>
#include <unordered_map>

namespace sym {

struct Value;
using ValuePtr = std::shared_ptr<Value>;

} // namespace sym

namespace cg {

class StackAllocator;
class RegAllocator;
struct Symbol;
using SymbolPtr = std::shared_ptr<Symbol>;
enum class Register : std::uint8_t;

class MemAllocator {
public:
  MemAllocator(std::ofstream &out, RegAllocator &regalloc, StackAllocator &stackalloc)
    : out(out), regalloc(regalloc), stackalloc(stackalloc) {}

public:
  void reset() { symtab.clear(); }

  auto alloc(const sym::ValuePtr &val, bool be_assigned) -> Register;
  void reuseReg(Register reg, const sym::ValuePtr &val);

  void prepareParam(const std::vector<sym::ValuePtr> &params);

  void load(const SymbolPtr &symbol);

  auto lookup(const sym::ValuePtr &val) -> std::optional<SymbolPtr>;

  void allocArgv(const std::vector<sym::ValuePtr> &argv);

private:
  std::ofstream &out;

  RegAllocator   &regalloc;
  StackAllocator &stackalloc;

  std::unordered_map<std::string, SymbolPtr> symtab;
};

} // namespace cg
