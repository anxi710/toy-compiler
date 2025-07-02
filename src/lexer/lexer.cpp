#include <regex>
#include <vector>
#include <cstdlib>

#include "lexer.hpp"
#include "token.hpp"
#include "err_type.hpp"
#include "err_report.hpp"

namespace lex::base {

using token::Token;
using token::TokenType;

Lexer::Lexer(std::vector<std::string> text, err::ErrReporter &ereporter)
  : text(std::move(text)), ereporter(ereporter)
{
  // 初始化关键字表
  this->keyword_table.addKeyword("if",       TokenType::IF);
  this->keyword_table.addKeyword("fn",       TokenType::FN);
  this->keyword_table.addKeyword("in",       TokenType::IN);
  this->keyword_table.addKeyword("i32",      TokenType::I32);
  this->keyword_table.addKeyword("let",      TokenType::LET);
  this->keyword_table.addKeyword("mut",      TokenType::MUT);
  this->keyword_table.addKeyword("for",      TokenType::FOR);
  this->keyword_table.addKeyword("loop",     TokenType::LOOP);
  this->keyword_table.addKeyword("else",     TokenType::ELSE);
  this->keyword_table.addKeyword("break",    TokenType::BREAK);
  this->keyword_table.addKeyword("while",    TokenType::WHILE);
  this->keyword_table.addKeyword("return",   TokenType::RETURN);
  this->keyword_table.addKeyword("continue", TokenType::CONTINUE);
}

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
      if (type == TokenType::ID && this->keyword_table.iskeyword(match.str(0))) {
        auto keyword_type = this->keyword_table.getKeyword(match.str(0));
        return Token{keyword_type, match.str(0), pos};
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
  Token token{};             // 识别到的词法单元
  switch (fchar) {
    default:
      break;
    case '(':
      token = Token{TokenType::LPAREN, "("};
      break;
    case ')':
      token = Token{TokenType::RPAREN, ")"};
      break;
    case '{':
      token = Token{TokenType::LBRACE, "{"};
      break;
    case '}':
      token = Token{TokenType::RBRACE, "}"};
      break;
    case '[':
      token = Token{TokenType::LBRACK, "["};
      break;
    case ']':
      token = Token{TokenType::RBRACK, "]"};
      break;
    case ';':
      token = Token{TokenType::SEMICOLON, ";"};
      break;
    case ':':
      token = Token{TokenType::COLON, ":"};
      break;
    case ',':
      token = Token{TokenType::COMMA, ","};
      break;
    case '+':
      token = Token{TokenType::PLUS, "+"};
      break;
    case '=':
      if (schar == '=') {
        token = Token{TokenType::EQ, "=="};
      } else {
        token = Token{TokenType::ASSIGN, "="};
      }
      break;
    case '-':
      if (schar == '>') {
        token = Token{TokenType::ARROW, "->"};
      } else {
        token = Token{TokenType::MINUS, "-"};
      }
      break;
    case '*':
      token = Token{TokenType::MUL, "*"};
      break;
    case '/':
      token = Token{TokenType::DIV, "/"};
      break;
    case '>':
      if (schar == '=') {
        token = Token{TokenType::GEQ, ">="};
      } else {
        token = Token{TokenType::GT, ">"};
      }
      break;
    case '<':
      if (schar == '=') {
        token = Token{TokenType::LEQ, "<="};
      } else {
        token = Token{TokenType::LT, "<"};
      }
      break;
    case '.':
      if (schar == '.') {
        token = Token{TokenType::DOTS, ".."};
      } else {
        token = Token{TokenType::DOT, "."};
      }
      break;
    case '!':
      if (schar == '=') {
        token = Token{TokenType::NEQ, "!="};
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
std::optional<token::Token>
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

  ereporter.report(
    err::LexErrType::UNKNOWN_TOKEN,
    "识别到未知的 token: " + view.substr(0, 1),
    pos.row,
    pos.col,
    view.substr(0, 1)
  );

  return std::nullopt;
}

} // namespace lex::base
