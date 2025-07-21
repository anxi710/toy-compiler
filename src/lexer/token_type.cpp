#include "panic.hpp"
#include "token_type.hpp"

namespace lex {

/**
 * @brief  token type 转 string
 * @param  type enum class TokenType 中的一个
 * @return 对应 string
 */
std::string
tokenType2str(TokenType type)
{
  switch (type) {
#define X(name) case TokenType::name: return #name;
  TOKEN_LIST(X)
#undef X
    default: break;
  }

  UNREACHABLE("token type undefined");
}

} // namespace lex
