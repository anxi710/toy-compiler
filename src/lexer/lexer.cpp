#include <regex>
#include <vector>
#include <cstdlib>

#include "lexer.hpp"
#include "token.hpp"
#include "err_type.hpp"
#include "err_report.hpp"

namespace lex {

/**
 * @brief 重置当前扫描位置
 * @param pos 设置到的位置
 */
void
Lexer::reset(const util::Position &pos)
{
  this->pos = pos;
  this->peek = this->text[pos.row][pos.col];
}

void
Lexer::shiftPos(std::size_t delta)
{
  pos.col += delta;
  if (pos.col >= text[pos.row].length()) {
    ++pos.row;
    pos.col = 0;
  }
};

std::optional<Token>
Lexer::matchThroughRE(const std::string &view)
{
  static const std::vector<std::pair<TokenType, std::regex>> patterns {
    {TokenType::ID,  std::regex{R"(^[a-zA-Z_]\w*)"}},
    {TokenType::INT, std::regex{R"(^\d+)"}}
  };

  // 使用正则表达式检测 INT、ID 两类词法单元
  for (const auto &[type, expression] : patterns) {
    std::smatch match;
    if (std::regex_search(view, match, expression)) {
      auto pos = this->pos;
      shiftPos(match.length(0));
      if (type == TokenType::ID && this->keytab.iskeyword(match.str(0))) {
        auto keytype = this->keytab.getKeyword(match.str(0));
        return Token{keytype, match.str(0), pos};
      }
      return Token{type, match.str(0), pos};
    }
  }

  return std::nullopt;
}

std::optional<Token>
Lexer::matchThroughDFA(const std::string &view)
{
  char fchar{view[0]}; // first char
  char schar{view.length() > 1 ? view[1] : '\0'}; // second char

  // 检测算符和标点符号
  Token token{}; // 识别到的词法单元
  switch (fchar) {
    default:
      break;
    case '(':
      token = {TokenType::LPAREN, "("};
      break;
    case ')':
      token = {TokenType::RPAREN, ")"};
      break;
    case '{':
      token = {TokenType::LBRACE, "{"};
      break;
    case '}':
      token = {TokenType::RBRACE, "}"};
      break;
    case '[':
      token = {TokenType::LBRACK, "["};
      break;
    case ']':
      token = {TokenType::RBRACK, "]"};
      break;
    case ';':
      token = {TokenType::SEMICOLON, ";"};
      break;
    case ':':
      token = {TokenType::COLON, ":"};
      break;
    case ',':
      token = {TokenType::COMMA, ","};
      break;
    case '+':
      token = {TokenType::PLUS, "+"};
      break;
    case '=':
      if (schar == '=') {
        token = {TokenType::EQ, "=="};
      } else {
        token = {TokenType::ASSIGN, "="};
      }
      break;
    case '-':
      if (schar == '>') {
        token = {TokenType::ARROW, "->"};
      } else {
        token = {TokenType::MINUS, "-"};
      }
      break;
    case '*':
      token = {TokenType::MUL, "*"};
      break;
    case '/':
      token = {TokenType::DIV, "/"};
      break;
    case '>':
      if (schar == '=') {
        token = {TokenType::GEQ, ">="};
      } else {
        token = {TokenType::GT, ">"};
      }
      break;
    case '<':
      if (schar == '=') {
        token = {TokenType::LEQ, "<="};
      } else {
        token = {TokenType::LT, "<"};
      }
      break;
    case '.':
      if (schar == '.') {
        token = {TokenType::DOTS, ".."};
      } else {
        token = {TokenType::DOT, "."};
      }
      break;
    case '!':
      if (schar == '=') {
        token = {TokenType::NEQ, "!="};
      }
      break;
  }

  token.pos = this->pos;
  if (!token.value.empty()) {
    shiftPos(token.value.length());
    return token;
  }

  return std::nullopt;
}

/**
 * @brief  获取下一个词法单元
 * @return token
 */
std::optional<Token>
Lexer::nextToken()
{
  // 检测当前是否已经到达结尾
  if (this->pos.row >= this->text.size()) {
    return Token{TokenType::END, "#", this->pos};
  }

  // 忽略所有空白字符
  while (this->pos.row < this->text.size()) {
    if (this->text[this->pos.row].empty()) {
      ++this->pos.row;
      this->pos.col = 0;
    } else if (
      static_cast<bool>(std::isspace(this->text[this->pos.row][this->pos.col]))
    ) {
      shiftPos(1);
    } else {
      break;
    }
  }

  // 再次判断是否到结尾
  if (this->pos.row >= this->text.size()) {
    return Token{TokenType::END, "#", this->pos};
  }

  // 使用正则表达式识别 INT、ID
  std::string view{this->text[this->pos.row].substr(this->pos.col)};
  auto token = matchThroughRE(view);
  if (token.has_value()) {
    return token;
  }

  // 使用 DFA 识别算符、界符等符号
  token = matchThroughDFA(view);
  if (token.has_value()) {
    return token;
  }

  util::Position pos = this->pos;
  shiftPos(1);

  reporter.report(
    err::LexErrType::UNKNOWN_TOKEN,
    std::format("识别到未知的 token: {}", view.substr(0, 1)),
    pos, view.substr(0, 1)
  );

  return std::nullopt;
}

} // namespace lex::base
