#include <regex>
#include <vector>
#include <cstdlib>

#include "lexer.hpp"
#include "token.hpp"
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

/**
 * @brief lexer 前指向位置移动指定字符数
 * @param delta 位移量
 */
void
Lexer::shiftPos(std::size_t delta)
{
  pos.col += delta;
  // 若到达句尾，则换行
  if (pos.col >= text[pos.row].length()) {
    ++pos.row;
    pos.col = 0;
  }
};

/**
 * @brief  通过正则表达式匹配 token
 * @param  view 扫描窗口
 * @return 识别到的 token，没有则返回 std::nullopt
 */
std::optional<Token>
Lexer::matchThroughRE(const std::string &view)
{
  static const std::vector<std::pair<TokenType, std::regex>> patterns {
    {TokenType::ID,  std::regex{R"(^[a-zA-Z_]\w*)"}},
    {TokenType::INT, std::regex{R"(^\d+)"}}
  };

  // 使用正则表达式检测 INT、ID 两类词法单元
  for (const auto &[type, re] : patterns) {
    std::smatch match;
    // 根据 pattern 识别，结果存储在 match 中
    if (std::regex_search(view, match, re)) {
      auto pos = this->pos;
      shiftPos(match.length(0));
      // 检查识别到的标识符是否是关键字
      if (type == TokenType::ID && this->keytab.iskeyword(match.str(0))) {
        auto keytype = this->keytab.getKeyword(match.str(0));
        return Token{keytype, match.str(0), pos};
      } // enf of if
      return Token{type, match.str(0), pos};
    } // end of if
  } // end of for

  return std::nullopt;
}

/**
 * @brief  通过 DFA 匹配 token
 * @param  view 扫描窗口
 * @return 识别到的 token，没有则返回 std::nullopt
 */
std::optional<Token>
Lexer::matchThroughDFA(const std::string &view)
{
  char fchar{view[0]}; // first char
  char schar{view.length() > 1 ? view[1] : '\0'}; // second char

  // 检测算符和标点符号
  Token token{}; // 识别到的词法单元
  switch (fchar) {
    default: break;
    case '(': token = {TokenType::LPAREN,    "("}; break;
    case ')': token = {TokenType::RPAREN,    ")"}; break;
    case '{': token = {TokenType::LBRACE,    "{"}; break;
    case '}': token = {TokenType::RBRACE,    "}"}; break;
    case '[': token = {TokenType::LBRACK,    "["}; break;
    case ']': token = {TokenType::RBRACK,    "]"}; break;
    case ';': token = {TokenType::SEMICOLON, ";"}; break;
    case ':': token = {TokenType::COLON,     ":"}; break;
    case ',': token = {TokenType::COMMA,     ","}; break;
    case '+': token = {TokenType::PLUS,      "+"}; break;
    case '*': token = {TokenType::MUL,       "*"}; break;
    case '/': token = {TokenType::DIV,       "/"}; break;
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
  } // end of switch

  token.pos = this->pos;
  if (!token.value.empty()) {
    shiftPos(token.value.length());
    return token;
  }

  return std::nullopt;
}

/**
 * @brief  获取下一个词法单元
 * @return next token，如果识别到未知 token 则返回 std::nullopt
 */
std::optional<Token>
Lexer::nextToken()
{
  // 检测当前是否已经到达结尾
  if (pos.row >= text.size()) {
    return Token{TokenType::END, "#", pos};
  }

  // 忽略所有空白字符
  while (pos.row < text.size()) {
    if (text[pos.row].empty()) { // 忽略空行
      ++pos.row;
      pos.col = 0;
    } else if ( // 忽略空白字符
      static_cast<bool>(std::isspace(text[pos.row][pos.col]))
    ) {
      shiftPos(1);
    } else {
      break;
    } // end of if
  } // end of while

  // 再次判断是否到结尾
  if (pos.row >= text.size()) {
    return Token{TokenType::END, "#", pos};
  }

  // 使用正则表达式识别 INT、ID
  std::string view{text[pos.row].substr(pos.col)};
  auto token = matchThroughRE(view);
  if (token.has_value()) {
    return token;
  }

  // 使用 DFA 识别算符、界符等符号
  token = matchThroughDFA(view);
  if (token.has_value()) {
    return token;
  }

  auto errpos = pos;
  shiftPos(1);

  reporter.report(
    err::LexErrType::UNKNOWN_TOKEN,
    std::format("识别到未知的 token: {}", view.substr(0, 1)),
    errpos, view.substr(0, 1)
  );

  return std::nullopt;
}

} // namespace lex
