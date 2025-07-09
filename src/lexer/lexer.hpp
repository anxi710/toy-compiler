#pragma once

#include <vector>
#include <optional>

#include "token.hpp"
#include "keyword.hpp"

namespace err {

struct LexErr;
class ErrReporter;

} // namespace err

namespace lex {

class Lexer {
public:
  Lexer(std::vector<std::string> text, err::ErrReporter &reporter)
    : text(std::move(text)), reporter(reporter)
  {
    // 初始化关键字表
    this->keytab.addKeyword("if",       TokenType::IF);
    this->keytab.addKeyword("fn",       TokenType::FN);
    this->keytab.addKeyword("in",       TokenType::IN);
    this->keytab.addKeyword("i32",      TokenType::I32);
    this->keytab.addKeyword("bool",     TokenType::BOOL);
    this->keytab.addKeyword("let",      TokenType::LET);
    this->keytab.addKeyword("mut",      TokenType::MUT);
    this->keytab.addKeyword("for",      TokenType::FOR);
    this->keytab.addKeyword("loop",     TokenType::LOOP);
    this->keytab.addKeyword("else",     TokenType::ELSE);
    this->keytab.addKeyword("break",    TokenType::BREAK);
    this->keytab.addKeyword("while",    TokenType::WHILE);
    this->keytab.addKeyword("return",   TokenType::RETURN);
    this->keytab.addKeyword("continue", TokenType::CONTINUE);
    this->keytab.addKeyword("true",     TokenType::TRUE);
    this->keytab.addKeyword("false",    TokenType::FALSE);
  }
  virtual ~Lexer() = default;

public:
  void reset(const util::Position &pos);

  auto nextToken() -> std::optional<Token>;

private:
  void shiftPos(std::size_t delta);

  auto matchThroughRE(const std::string &view) -> std::optional<Token>;
  auto matchThroughDFA(const std::string &view) -> std::optional<Token>;

private:
  char peek; // the next character to be scanned
  util::Position pos; // the next position to be scanned
  std::vector<std::string> text; // text to be scanned

  KeywordTable keytab;
  err::ErrReporter  &reporter; // Error Reporter
};

} // namespace lex
