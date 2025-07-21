#pragma once

#include <string>
#include <cstdint>

namespace lex {

// 使用 X-Macro 自动生成 TokenType 和相应的转换函数
// NOTE: X-Macro 是一种 C/C++ 中用于代码自动生成的技巧，它通过预处理器宏机制，
//       将一份数据定义用于多个用途，如：枚举定义、字符串映射、表驱动结构、自动代码生成等
//       本质思想是把所有”重复的数据“集中定义在一个宏列表中（通常叫做 X-Macro 表）
//       然后通过宏重定义的方式多次展开以生成不同用途的代码
//       这是一种预处理阶段的编程技巧

#define TOKEN_LIST(_) \
  _(END)        \
  _(IF)         \
  _(ELSE)       \
  _(WHILE)      \
  _(ID)         \
  _(I32)        \
  _(BOOL)       \
  _(LET)        \
  _(RETURN)     \
  _(MUT)        \
  _(FN)         \
  _(FOR)        \
  _(IN)         \
  _(LOOP)       \
  _(BREAK)      \
  _(CONTINUE)   \
  _(TRUE)       \
  _(FALSE)      \
  _(INT)        \
  _(LPAREN)     \
  _(RPAREN)     \
  _(LBRACE)     \
  _(RBRACE)     \
  _(LBRACK)     \
  _(RBRACK)     \
  _(SEMICOLON)  \
  _(COLON)      \
  _(COMMA)      \
  _(PLUS)       \
  _(ASSIGN)     \
  _(MINUS)      \
  _(MUL)        \
  _(DIV)        \
  _(GT)         \
  _(LT)         \
  _(DOT)        \
  _(EQ)         \
  _(NEQ)        \
  _(GEQ)        \
  _(LEQ)        \
  _(DOTS)       \
  _(ARROW)

// token 类型
enum class TokenType : std::uint8_t {
#define X(name) name,
  TOKEN_LIST(X)
#undef X
  COUNT // 占位符，用于推导 enum class 中的元素个数
};

auto tokenType2str(TokenType type) -> std::string;

} // namespace lex
