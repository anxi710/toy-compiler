#pragma once

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

namespace cpr {

class Compiler {
private:
  std::unique_ptr<lex::base::Lexer> lexer;
  std::unique_ptr<par::base::Parser> parer;
};

} // namespace cpr
