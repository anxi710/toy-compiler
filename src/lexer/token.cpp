#include "token.hpp"
#include "token_type.hpp"

namespace lex::token {

/**
 * @brief  将 token 格式化为一个 string
 * @return 格式化后的字符串
 */
std::string
token2str(const Token &token)
{
  std::ostringstream oss;
  oss << "<type: " << tokenType2str(token.type) << ", value: \"" << token.value << "\">@("
      << token.pos.row + 1 << ", " << token.pos.col + 1 << ")";
  return oss.str();
}

} // namespace lex::token
