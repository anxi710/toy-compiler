#include "keyword.hpp"

namespace lex::keyword {


/**
 * @brief  判断给定的输入值是否是一个关键字
 * @param  v token value
 * @return true / false
 */
bool
KeywordTable::iskeyword(const std::string &v) const
{
  return (keywords.contains(v));
}

/**
 * @brief  根据输入值获取对应 token type
 * @param  v token value
 * @return keyword token type
 */
token::TokenType
KeywordTable::getKeyword(const std::string &v) const
{
  assert(keywords.contains(v));
  return keywords.find(v)->second;
}

/**
 * @brief 向关键词表中添加一个关键词类型
 * @param n keyword name
 * @param t keyword token type
 */
void
KeywordTable::addKeyword(std::string n, token::TokenType t)
{
  this->keywords.emplace(n, t);
}

/**
 * @brief 设置错误报告器
 * @param reporter 全局错误报告器
 */
void
KeywordTable::setErrReporter(std::shared_ptr<err::ErrReporter> rep)
{
  this->reporter = std::move(rep);
}

} // namespace lex::keyword
