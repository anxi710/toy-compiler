#pragma once

#include <expected>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "token.hpp"
#include "keyword.hpp"
#include "util/position.hpp"

namespace err {

struct LexErr;
class ErrReporter;

} // namespace err

namespace lex::base {

// abstract class lexer: define some utilities and data structure.
class Lexer {
public:
  Lexer() = delete;
  explicit Lexer(const std::vector<std::string> &t);
  explicit Lexer(std::vector<std::string> &&t);
  virtual ~Lexer() = default;

  void initKeywordTable();

public:
  auto nextToken() -> std::expected<token::Token, err::LexErr>;

public:
  void reset(const util::Position &p);
  void setErrReporter(std::shared_ptr<err::ErrReporter> reporter);

private:
  std::shared_ptr<err::ErrReporter> reporter; // error reporter

  std::vector<std::string> text; // text to be scanned
  util::Position pos;            // the next position to be scanned
  char peek;                     // the next character to be scanned

  keyword::KeywordTable keyword_table; // 关键词表
};

} // namespace lex::base
