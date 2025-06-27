#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

#include "error/err_report.hpp"
#include "token_type.hpp"

namespace lex::keyword {

/**
 * @brief   关键字表
 * @details 内置一个存储所有关键字的 hash map，用于查找判断指定 token
 * 是否为关键字
 */
class KeywordTable {
public:
  KeywordTable() = default;
  ~KeywordTable() = default;

  void setErrReporter(std::shared_ptr<err::ErrReporter> rep);

public:
  bool iskeyword(const std::string &v) const;
  void addKeyword(std::string n, token::TokenType t);
  token::TokenType getKeyword(const std::string &v) const;

private:
  std::shared_ptr<err::ErrReporter> reporter; // error reporter
  std::unordered_map<std::string, token::TokenType> keywords; // keyword hash map
};

} // namespace lex::keyword
