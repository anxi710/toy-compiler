#pragma once

#include <string>
#include <cstdint>

#include "position.hpp"

namespace err {

// 词法错误码
enum class LexErrType : std::uint8_t {
  UNKNOWN_TOKEN, // 未知的 token
};

// 语法错误码
enum class ParErrType : std::uint8_t {
  UNEXPECT_TOKEN, // 并非期望 token
};

// 语义错误码
enum class SemErrType : std::uint8_t {
  RETTYPE_MISMATCH,      // 函数返回值类型错误
  VOID_FUNC_RET_VAL,     // void函数返回了值
  MISSING_RETVAL,        // 非void函数未返回值
  CALL_UNDEFINED_FUNC,   // 调用未定义函数
  ARG_CNT_MISMATCH,      // 参数数量不匹配
  UNDECLARED_VAR,        // 变量未声明
  UNINITIALIZED_VAR,     // 变量未初始化
  ASSIGN_NOT_VAR,        // 赋值左侧非变量
  ASSIGN_UNDECLARED_VAR, // 赋值给未声明变量
  TYPE_INFER_FAILURE,    // 变量无法通过自动类型推导确定类型
  TYPE_MISMATCH,         // 变量类型不匹配
  CANNOT_COMPAR,         // 比较运算符的操作数无法比较
};

struct Err {
  std::string    msg; // error message
  util::Position pos;

  Err(std::string msg, util::Position pos)
    : msg(std::move(msg)), pos(pos) {}
  virtual ~Err() = default;
};

// 词法错误
struct LexErr : Err {
  LexErrType  type;
  std::string token;

  LexErr(LexErrType type, std::string msg,
    util::Position pos, std::string token
  ) : Err(msg, pos), type(type),
      token(std::move(token)) {}
  ~LexErr() override = default;
};

// 语法错误
struct ParErr : Err {
  ParErrType  type;
  std::string token;

  ParErr(ParErrType type, std::string msg,
    util::Position pos, std::string token
  ) : Err(msg, pos), type(type),
      token(std::move(token)) {}
  ~ParErr() override = default;
};

// 语义错误
struct SemErr : Err {
  SemErrType  type;
  std::string scope_name;

  SemErr(SemErrType type, std::string msg,
    util::Position pos, std::string scope_name
  ) : Err(msg, pos), type(type),
      scope_name(std::move(scope_name)) {}
  ~SemErr() override = default;
};

} // namespace err
