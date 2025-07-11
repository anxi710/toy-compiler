#pragma once

#include <string>
#include <cstdint>

namespace lex {

// token 类型
enum class TokenType : std::uint8_t {
  // Group 0
  END, // end of file

  // Group 1
  FN, RETURN,
  ID,
  INT,
  IF, ELSE,
  WHILE, FOR, LOOP,
  I32, BOOL,
  LET, MUT,
  IN,
  BREAK, CONTINUE,
  TRUE, FALSE,

  LPAREN,    //  (
  RPAREN,    //  )
  LBRACE,    //  {
  RBRACE,    //  }
  LBRACK,    //  [
  RBRACK,    //  ]
  SEMICOLON, //  ;
  COLON,     //  :
  COMMA,     //  ,

  PLUS, //  +

  // Group 2
  ASSIGN, //  =
  MINUS,  //  -
  MUL,    //  *
  DIV,    //  /
  GT,     //  >
  LT,     //  <
  DOT,    //  .

  EQ,     //  ==
  NEQ,    //  !=
  GEQ,    //  >=
  LEQ,    //  <=
  DOTS,   //  ..
  ARROW   //  ->
};

auto tokenType2str(TokenType type) -> std::string;

} // namespace lex
