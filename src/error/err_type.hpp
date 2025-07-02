#pragma once

#include <string>
#include <cstdint>

namespace err {

// 错误类型
enum class ErrKind : std::uint8_t {
  LEX,      // 词法错误
  PAR,    // 语法错误
  SEM, // 语义错误
};

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
  FuncReturnTypeMismatch, // 函数返回值类型错误
  VoidFuncReturnValue,    // void函数返回了值
  MissingReturnValue,     // 非void函数未返回值
  UndefinedFunctionCall,  // 调用未定义函数
  ArgCountMismatch,       // 参数数量不匹配
  UndeclaredVariable,     // 变量未声明
  UninitializedVariable,  // 变量未初始化
  AssignToNonVariable,    // 赋值左侧非变量
  AssignToUndeclaredVar,  // 赋值给未声明变量
  TypeInferenceFailure,   // 变量无法通过自动类型推导确定类型
  TypeMismatch,           // 变量类型不匹配
  CannotCompar,           // 比较运算符的操作数无法比较
};

struct Err {
  ErrKind     kind;
  std::string msg; // error message

  Err(ErrKind kind, std::string msg) : kind(kind), msg(msg) {}
  virtual ~Err() = default;
};

// 词法错误
struct LexErr : Err {
  LexErrType  type;
  std::size_t row;
  std::size_t col;
  std::string token;

  LexErr(LexErrType type, std::string msg,
    std::size_t r, std::size_t c, std::string token
  ) : Err(ErrKind::LEX, msg), type(type),
      row(r), col(c), token(std::move(token)) {}
  ~LexErr() override = default;
};

// 语法错误
struct ParErr : Err {
  ParErrType  type;
  std::size_t row;
  std::size_t col;
  std::string token;

  ParErr(ParErrType type, std::string msg,
    std::size_t r, std::size_t c, std::string token
  ) : Err(ErrKind::PAR, msg), type(type),
      row(r), col(c), token(std::move(token)) {}
  ~ParErr() override = default;
};

// 语义错误
struct SemErr : Err {
  SemErrType  type;
  std::size_t row;
  std::size_t col;
  std::string scope_name;

  SemErr(SemErrType type, std::string msg,
    std::size_t r, std::size_t c, std::string scope_name
  ) : Err(ErrKind::SEM, msg), type(type),
      row(r), col(c), scope_name(std::move(scope_name)) {}
  ~SemErr() override = default;
};

} // namespace err
