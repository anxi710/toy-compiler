#pragma once

#include "lexer.hpp"
#include "parser.hpp"
#include "err_report.hpp"
#include "symbol_table.hpp"
#include "semantic_ir_builder.hpp"

namespace cpr {

constexpr std::uint8_t F_LL     = 1; // 使用 LL 分析法构建的语法分析器
constexpr std::uint8_t F_LR     = 2; // 使用 LR 分析法构建的语法分析器

constexpr std::uint8_t B_X86_84 = 1; // 生成 x86_64 汇编代码
constexpr std::uint8_t B_RISC_V = 2; // 生成 risc-v 汇编代码

// 编译器类，维护编译器模块的调用逻辑
class Compiler {
public:
  Compiler(const std::string &file);

public:
  void generateIR(const std::string &file);
  void generateAssemble();

private:
  ast::ProgPtr ast_root;

  std::unique_ptr<lex::Lexer>             lexer;    // lexer
  std::unique_ptr<par::Parser>            parser;   // parser
  std::unique_ptr<sym::SymbolTable>       symtab;   // symbol table
  std::unique_ptr<par::SemanticIRBuilder> builder;  // semantic ir builder
  std::unique_ptr<err::ErrReporter>       reporter; // error reporter
};

} // namespace cpr
