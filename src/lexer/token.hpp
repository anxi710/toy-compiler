#pragma once

#include <string>
#include <cstdint>
#include <sstream>

#include "position.hpp"
#include "token_type.hpp"

namespace lex {

enum class TokenType : std::uint8_t;

struct Token {
  TokenType      type;  // token type
  std::string    value; // 组成 token 的字符串
  util::Position pos;   // position

  Token() = default;
  Token(TokenType type, std::string value,
    util::Position pos = {0, 0}
  ) : type(type), value(std::move(value)), pos(pos) {}
  ~Token() = default;

  std::string str() const {
    return std::format("<type: {}, value: {}>@{}",
      tokenType2str(type), value, pos
    );
  }
};

} // namespace lex
