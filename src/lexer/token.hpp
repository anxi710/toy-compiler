#pragma once

#include <string>

#include "token_type.hpp"
#include "util/position.hpp"

namespace lex::token {

enum class Type : std::uint8_t;

class Token {
public:
  Token() = default;

  explicit Token(const TokenType &t, std::string v,
                 const util::Position &p = util::Position{0, 0})
    : type(t), value(std::move(v)), pos(p) {}

  Token(const Token &other) = default;

  Token(Token &&other) noexcept
    : type(other.type), value(std::move(other.value)), pos(other.pos) {}

  ~Token() = default;

  auto operator=(const Token &rhs) -> Token & = default;

  bool operator==(const Token &rhs);

public:
  [[nodiscard]]
  auto getValue() const -> const std::string&;
  [[nodiscard]]
  auto getType() const -> TokenType;

  void setPos(const util::Position &p);
  void setPos(std::size_t r, std::size_t c);
  [[nodiscard]]
  auto getPos() const -> util::Position;

  [[nodiscard]]
  auto toString() const -> std::string;

private:
  TokenType type;     // token type
  std::string value;  // 组成 token 的字符串
  util::Position pos; // position
};

} // namespace lex::token
