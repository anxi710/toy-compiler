#include <regex>
#include <vector>
#include <cstdlib>

#include "lexer.hpp"
#include "token.hpp"
#include "error/err_report.hpp"

namespace lex::base {

Lexer::Lexer(const std::vector<std::string> &t) : text(t)
{
  initKeywordTable();
}

Lexer::Lexer(std::vector<std::string> &&t) : text(std::move(t))
{
  initKeywordTable();
}

/**
 * @brief 重置当前扫描位置
 * @param pos 设置到的位置
 */
void
Lexer::reset(const util::Position &p)
{
  this->pos = p;
  this->peek = this->text[p.row][p.col];
}

/**
 * @brief 设置错误报告器
 * @param reporter 全局错误报告器
 */
void
Lexer::setErrReporter(std::shared_ptr<err::ErrReporter> reporter)
{
  this->reporter = std::move(reporter);
}

/**
 * @brief  获取下一个词法单元
 * @return token / LexErrorPtr
 */
std::expected<token::Token, err::LexErr>
Lexer::nextToken()
{
  using token::Token;
  using token::TokenType;
  static const std::vector<std::pair<TokenType, std::regex>> patterns{
      {TokenType::ID, std::regex{R"(^[a-zA-Z_]\w*)"}},
      {TokenType::INT, std::regex{R"(^\d+)"}}};

  // 检测当前是否已经到达结尾
  if (this->pos.row >= this->text.size()) {
    return Token{TokenType::END, "#", this->pos};
  }

  auto shiftPos = [&](std::size_t delta) {
    this->pos.col += delta;
    if (this->pos.col >= this->text[this->pos.row].length())
    {
      ++this->pos.row;
      this->pos.col = 0;
    }
  };

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

  // 使用正则表达式检测 INT、ID 两类词法单元
  std::string view{this->text[this->pos.row].substr(this->pos.col)};
  for (const auto &[type, expression] : patterns) {
    std::smatch match;
    if (std::regex_search(view, match, expression)) {
      auto p = this->pos;
      shiftPos(match.length(0));
      if (type == TokenType::ID && this->keyword_table.iskeyword(match.str(0))) {
        auto keyword_type = this->keyword_table.getKeyword(match.str(0));
        return Token{keyword_type, match.str(0), p};
      }
      return Token{type, match.str(0), p};
    }
  }

  Token token{};            // 识别到的词法单元
  char first_char{view[0]}; // 当前看到的第一个字符
  char second_char{view.length() > 1 ? view[1] : '\0'}; // 当前看到的第二个字符

  // 检测算符和标点符号
  switch (first_char) {
  default:
    break;
  case '(':
    token = Token{TokenType::LPAREN, std::string{"("}};
    break;
  case ')':
    token = Token{TokenType::RPAREN, std::string{")"}};
    break;
  case '{':
    token = Token{TokenType::LBRACE, std::string{"{"}};
    break;
  case '}':
    token = Token{TokenType::RBRACE, std::string{"}"}};
    break;
  case '[':
    token = Token{TokenType::LBRACK, std::string{"["}};
    break;
  case ']':
    token = Token{TokenType::RBRACK, std::string{"]"}};
    break;
  case ';':
    token = Token{TokenType::SEMICOLON, std::string{";"}};
    break;
  case ':':
    token = Token{TokenType::COLON, std::string{":"}};
    break;
  case ',':
    token = Token{TokenType::COMMA, std::string{","}};
    break;
  case '+':
    token = Token{TokenType::OP_PLUS, std::string{"+"}};
    break;
  case '=':
    if (second_char == '=') {
      token = Token{TokenType::OP_EQ, std::string{"=="}};
    } else {
      token = Token{TokenType::ASSIGN, std::string{"="}};
    }
    break;
  case '-':
    if (second_char == '>') {
      token = Token{TokenType::ARROW, std::string{"->"}};
    } else {
      token = Token{TokenType::OP_MINUS, std::string{"-"}};
    }
    break;
  case '*':
    if (second_char == '/') {
      token = Token{TokenType::RMUL_COM, std::string{"*/"}};
    } else {
      token = Token{TokenType::OP_MUL, std::string{"*"}};
    }
    break;
  case '/':
    if (second_char == '/') {
      token = Token{TokenType::SIN_COM, std::string{"//"}};
    } else if (second_char == '*') {
      token = Token{TokenType::LMUL_COM, std::string{"/*"}};
    } else {
      token = Token{TokenType::OP_DIV, std::string{"/"}};
    }
    break;
  case '>':
    if (second_char == '=') {
      token = Token{TokenType::OP_GE, std::string{">="}};
    } else {
      token = Token{TokenType::OP_GT, std::string{">"}};
    }
    break;
  case '<':
    if (second_char == '=') {
      token = Token{TokenType::OP_LE, std::string{"<="}};
    } else {
      token = Token{TokenType::OP_LT, std::string{"<"}};
    }
    break;
  case '.':
    if (second_char == '.') {
      token = Token{TokenType::DOTS, std::string{".."}};
    } else {
      token = Token{TokenType::DOT, std::string{"."}};
    }
    break;
  case '!':
    if (second_char == '=') {
      token = Token{TokenType::OP_NEQ, std::string{"!="}};
    }
    break;
  case '&':
    token = Token{TokenType::REF, std::string{"&"}};
    break;
  }

  token.setPos(this->pos);
  if (!token.getValue().empty()) {
    shiftPos(token.getValue().length());
    return token;
  }

  util::Position p = this->pos;
  shiftPos(1);

  return std::unexpected(err::LexErr{
    err::LexErrType::UnknownToken,
    "识别到未知的 token: " + view.substr(0, 1),
    p.row,
    p.col,
    view.substr(0, 1)
  });
}

/**
 * @brief 初始化关键词表
 */
void
Lexer::initKeywordTable()
{
  using token::TokenType;
  this->keyword_table.addKeyword("if", TokenType::IF);
  this->keyword_table.addKeyword("fn", TokenType::FN);
  this->keyword_table.addKeyword("in", TokenType::IN);
  this->keyword_table.addKeyword("i32", TokenType::I32);
  this->keyword_table.addKeyword("let", TokenType::LET);
  this->keyword_table.addKeyword("mut", TokenType::MUT);
  this->keyword_table.addKeyword("for", TokenType::FOR);
  this->keyword_table.addKeyword("loop", TokenType::LOOP);
  this->keyword_table.addKeyword("else", TokenType::ELSE);
  this->keyword_table.addKeyword("break", TokenType::BREAK);
  this->keyword_table.addKeyword("while", TokenType::WHILE);
  this->keyword_table.addKeyword("return", TokenType::RETURN);
  this->keyword_table.addKeyword("continue", TokenType::CONTINUE);

  this->keyword_table.setErrReporter(this->reporter);
}

} // namespace lex::base
