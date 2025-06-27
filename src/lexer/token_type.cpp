#include "token_type.hpp"

#include <iostream>
#include <unordered_map>

namespace lex::token {

/**
 * @brief  token::Type 转 string
 * @param  type enum class token::Type 中的一个
 * @return 对应的 string
 */
std::string
tokenType2str(TokenType type)
{
  static const std::unordered_map<TokenType, std::string> map {
    {TokenType::END,      "END"},      {TokenType::IF, "IF"},
    {TokenType::ELSE,     "ELSE"},     {TokenType::WHILE, "WHILE"},
    {TokenType::ID,       "ID"},       {TokenType::I32, "I32"},
    {TokenType::LET,      "LET"},      {TokenType::RETURN, "RETURN"},
    {TokenType::MUT,      "MUT"},      {TokenType::FN, "FN"},
    {TokenType::FOR,      "FOR"},      {TokenType::IN, "IN"},
    {TokenType::LOOP,     "LOOP"},     {TokenType::BREAK, "BREAK"},
    {TokenType::CONTINUE, "CONTINUE"}, {TokenType::INT, "INT"},
    {TokenType::REF,      "REF"},      {TokenType::LPAREN,    "LPAREN"},
    {TokenType::RPAREN,   "RPAREN"},   {TokenType::LBRACE,    "LBRACE"},
    {TokenType::RBRACE,   "RBRACE"},   {TokenType::LBRACK,    "LBRACK"},
    {TokenType::RBRACK,   "RBRACK"},   {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::COLON,    "COLON"},    {TokenType::COMMA, "COMMA"},
    {TokenType::OP_PLUS,  "OP_PLUS"},  {TokenType::ASSIGN, "ASSIGN"},
    {TokenType::OP_MINUS, "OP_MINUS"}, {TokenType::OP_MUL, "OP_MUL"},
    {TokenType::OP_DIV,   "OP_DIV"},   {TokenType::OP_GT, "OP_GT"},
    {TokenType::OP_LT,    "OP_LT"},    {TokenType::DOT, "DOT"},
    {TokenType::OP_EQ,    "OP_EQ"},    {TokenType::OP_NEQ, "OP_NEQ"},
    {TokenType::OP_GE,    "OP_GE"},    {TokenType::OP_LE, "OP_LE"},
    {TokenType::DOTS,     "DOTS"},     {TokenType::ARROW, "ARROW"},
    {TokenType::SIN_COM,  "SIN_COM"},  {TokenType::LMUL_COM, "LMUL_COM"},
    {TokenType::RMUL_COM, "RMUL_COM"}
  };

  if (auto iter = map.find(type); iter == map.end()) {
    std::cerr << "tokenType2str(): unknown token type." << std::endl;
    exit(1);
  } else {
    return iter->second;
  }
}

} // namespace lex::token
