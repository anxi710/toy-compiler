#pragma once

#include <cstdint>
#include <string>

namespace lex::token {

// token 类型
enum class TokenType : std::uint8_t {
  // Group 0
  END, // end of file

  // Group 1
  ID,
  INT, // identifier, integer
  IF,
  ELSE,
  WHILE,
  FOR,
  I32,
  LET,
  RETURN,
  MUT,
  FN,
  IN,
  LOOP,
  BREAK,
  CONTINUE,

  REF,       //  &
  LPAREN,    //  (
  RPAREN,    //  )
  LBRACE,    //  {
  RBRACE,    //  }
  LBRACK,    //  [
  RBRACK,    //  ]
  SEMICOLON, //  ;
  COLON,     //  :
  COMMA,     //  ,

  OP_PLUS, //  +

  // Group 2
  ASSIGN,   //  =
  OP_MINUS, //  -
  OP_MUL,   //  *
  OP_DIV,   //  /
  OP_GT,    //  >
  OP_LT,    //  <
  DOT,      //  .

  OP_EQ,    //  ==
  OP_NEQ,   //  !=
  OP_GE,    //  >=
  OP_LE,    //  <=
  DOTS,     //  ..
  ARROW,    //  ->
  SIN_COM,  //  //
  LMUL_COM, //  /*
  RMUL_COM  //  */
};

auto tokenType2str(TokenType type) -> std::string;

} // namespace lex::token
