#pragma once

#include <cstdint>

namespace err {

// 错误类型
enum class ErrType : std::uint8_t {
  Lex,      // 词法错误
  Par,    // 语法错误
  Sem, // 语义错误
};

// 词法错误码
enum class LexErrType : std::uint8_t {
  UnknownToken, // 未知的 token
};

// 语法错误码
enum class ParErrType : std::uint8_t {
  UnexpectToken, // 并非期望 token
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

};

} // namespace err
