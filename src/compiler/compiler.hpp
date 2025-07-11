#pragma once

#include "lexer.hpp"
#include "parser.hpp"
#include "err_report.hpp"
#include "symbol_table.hpp"
#include "semantic_ir_builder.hpp"

namespace cpr {

constexpr std::uint8_t F_LL     = 1;
constexpr std::uint8_t F_LR     = 2;

constexpr std::uint8_t B_X86_84 = 1;
constexpr std::uint8_t B_RISC_V = 2;

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
