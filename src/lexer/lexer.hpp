#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "token.hpp"
#include "keyword.hpp"
#include "position.hpp"

namespace err {

struct LexErr;
class ErrReporter;

} // namespace err

namespace lex::base {

// abstract class lexer: define some utilities and data structure.
class Lexer {
public:
  Lexer(std::vector<std::string> text, err::ErrReporter &ereporter);
  virtual ~Lexer() = default;

public:
  void reset(const util::Position &pos);
  void setErrReporter(std::shared_ptr<err::ErrReporter> p_ereporter);

  auto nextToken() -> std::optional<token::Token>;

private:
  void shiftPos(std::size_t delta);

  auto matchThroughRE(const std::string &view) -> std::optional<token::Token>;
  auto matchThroughDFA(const std::string &view) -> std::optional<token::Token>;

private:
  char peek; // the next character to be scanned
  util::Position pos; // the next position to be scanned
  std::vector<std::string> text; // text to be scanned

  key::KeywordTable keyword_table;
  err::ErrReporter  &ereporter; // Error Reporter
};

} // namespace lex::base
