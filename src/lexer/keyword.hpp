#pragma once

#include <cassert>
#include <string>
#include <unordered_map>

#include "token_type.hpp"

namespace lex::key {

/**
 * @brief   关键字表
 * @details 内置一个存储所有关键字的 hash map，用于查找判断指定 token
 * 是否为关键字
 */
class KeywordTable {
public:
  KeywordTable() = default;
  ~KeywordTable() = default;

public:
  /**
   * @brief  判断给定的输入值是否是一个关键字
   * @param  v token value
   * @return true / false
   */
  bool iskeyword(const std::string &v) const {
    return (keywords.contains(v));
  }

  /**
   * @brief 向关键词表中添加一个关键词类型
   * @param n keyword name
   * @param t keyword token type
   */
  void addKeyword(std::string n, token::TokenType t) {
    this->keywords.emplace(n, t);
  }

  /**
   * @brief  根据输入值获取对应 token type
   * @param  v token value
   * @return keyword token type
   */
  token::TokenType getKeyword(const std::string &v) const {
    assert(keywords.contains(v));
    return keywords.find(v)->second;
  }

private:
  std::unordered_map<std::string, token::TokenType> keywords; // keyword hash map
};

} // namespace lex::keyword
