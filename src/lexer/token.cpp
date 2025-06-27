#include "token.hpp"

#include <sstream>

namespace lex::token {

bool
Token::operator==(const Token &rhs)
{
  // == 并不考虑位置！
  return this->type == rhs.type && this->value == rhs.value;
}

/**
 * @brief  获取 token 的值
 * @return token value
 */
const std::string&
Token::getValue() const
{
  return this->value;
}

/**
 * @brief  获取 token 的类型
 * @return token type
 */
TokenType
Token::getType() const
{
  return this->type;
}

/**
 * @brief  获取 token 的位置
 * @return Position
 */
util::Position
Token::getPos() const
{
  return this->pos;
}

/**
 * @brief 设置 token 所在的文本位置
 * @param p struct Position {row, col}
 */
void
Token::setPos(const util::Position &p)
{
  this->pos = p;
}

/**
 * @brief 设置 token 所在的文本位置
 * @param r row
 * @param c col
 */
void
Token::setPos(std::size_t r, std::size_t c)
{
  this->pos.row = r;
  this->pos.col = c;
}

/* function member definition */

/**
 * @brief  将 token 格式化为一个 string
 * @return 格式化后的字符串
 */
std::string
Token::toString() const
{
  std::ostringstream oss;
  oss << "<type: " << tokenType2str(type) << ", value: \"" << value << "\">@("
      << this->pos.row + 1 << ", " << this->pos.col + 1 << ")";
  return oss.str();
}

/* function member definition */

} // namespace lex::token
