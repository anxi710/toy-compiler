#include "panic.hpp"
#include "token_type.hpp"

namespace lex {

/**
 * @brief  token type 转 string
 * @param  type enum class TokenType 中的一个
 * @return 对应 string
 */
std::string
tokenType2str(TokenType type)
{
  switch (type) {
    case TokenType::END:       return "END";
    case TokenType::IF:        return "IF";
    case TokenType::ELSE:      return "ELSE";
    case TokenType::WHILE:     return "WHILE";
    case TokenType::ID:        return "ID";
    case TokenType::I32:       return "I32";
    case TokenType::BOOL:      return "BOOL";
    case TokenType::LET:       return "LET";
    case TokenType::RETURN:    return "RETURN";
    case TokenType::MUT:       return "MUT";
    case TokenType::FN:        return "FN";
    case TokenType::FOR:       return "FOR";
    case TokenType::IN:        return "IN";
    case TokenType::LOOP:      return "LOOP";
    case TokenType::BREAK:     return "BREAK";
    case TokenType::CONTINUE:  return "CONTINUE";
    case TokenType::TRUE:      return "TRUE";
    case TokenType::FALSE:     return "FALSE";
    case TokenType::INT:       return "INT";
    case TokenType::LPAREN:    return "LPAREN";
    case TokenType::RPAREN:    return "RPAREN";
    case TokenType::LBRACE:    return "LBRACE";
    case TokenType::RBRACE:    return "RBRACE";
    case TokenType::LBRACK:    return "LBRACK";
    case TokenType::RBRACK:    return "RBRACK";
    case TokenType::SEMICOLON: return "SEMICOLON";
    case TokenType::COLON:     return "COLON";
    case TokenType::COMMA:     return "COMMA";
    case TokenType::PLUS:      return "PLUS";
    case TokenType::ASSIGN:    return "ASSIGN";
    case TokenType::MINUS:     return "MINUS";
    case TokenType::MUL:       return "MUL";
    case TokenType::DIV:       return "DIV";
    case TokenType::GT:        return "GT";
    case TokenType::LT:        return "LT";
    case TokenType::DOT:       return "DOT";
    case TokenType::EQ:        return "EQ";
    case TokenType::NEQ:       return "NEQ";
    case TokenType::GEQ:       return "GEQ";
    case TokenType::LEQ:       return "LEQ";
    case TokenType::DOTS:      return "DOTS";
    case TokenType::ARROW:     return "ARROW";
  } // end of switch

  UNREACHABLE("lex::tokenType2str()");
}

} // namespace lex
