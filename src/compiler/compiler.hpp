#pragma once

#include "lexer.hpp"
#include "parser.hpp"
#include "err_report.hpp"
#include "symbol_table.hpp"
#include "semantic_checker.hpp"

namespace cpr {

constexpr std::uint8_t F_LL     = 1;
constexpr std::uint8_t F_LR     = 2;

constexpr std::uint8_t B_X86_84 = 1;
constexpr std::uint8_t B_RISC_V = 2;

class Compiler {
public:
  Compiler(const std::string &file);

public:
  void generateToken();
  void generateAST();
  void generateSymbol();
  void generateIR();
  void generateAssemble();

private:
  ast::ProgPtr ast_root;
  std::vector<lex::token::Token> tokens;

  std::unique_ptr<lex::base::Lexer>     lexer;
  std::unique_ptr<par::base::Parser>    parser;
  std::unique_ptr<sym::SymbolTable>     stable;
  std::unique_ptr<sem::SemanticChecker> schecker;
  std::unique_ptr<err::ErrReporter>     ereporter;
};

} // namespace cpr
