#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include "position.hpp"

namespace err {

class ErrReporter;

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
  MISSING_RETVAL,        // 函数缺少返回值
  TYPE_INFER_FAILURE,    // 变量无法通过自动类型推导确定类型
  TYPE_MISMATCH,         // 类型不匹配
  UNEXPECTED_EXPR_TYPE,  // 意料之外的表达式类型
  BREAK_CTX_ERROR,       // break 所在上下文有误
  BREAK_TYPE_MISMATCH,   // break 返回的值的类型不一致
  CONTINUE_CTX_ERROR,    // continue 表达式所在上下文有误
  UNDECLARED_VAR,        // 变量未声明
  OUTOFBOUNDS_ACCESS,    // 访问越界
  ASSIGN_IMMUTABLE,      // 赋值表达式左值不可变
  ASSIGN_MISMATCH,       // 左值和右值类型不匹配
  INCOMPARABLE_TYPES,    // 两值的类型不可比较
  UNCOMPUTABLE_TYPES,    // 两值的类型不可进行算术运算
  RETTYPE_MISMATCH,      // 函数返回值类型错误
  CALL_UNDECLARED_FUNC,  // 调用未声明函数
  ARG_CNT_MISMATCH,      // 参数数量不匹配
  ARG_TYPE_MISMATCH,     // 参数类型不匹配
  MISSING_ELSE,          // 缺少 else 分支
  UNINITIALIZED_VAR,     // 变量未初始化
};

struct Err {
  std::string    msg; // error message
  util::Position pos; // 错误发生的位置

  Err(std::string msg, util::Position pos)
    : msg(std::move(msg)), pos(pos) {}
  virtual ~Err() = default;
  virtual void display(const ErrReporter &reporter) const = 0;
};
using ErrPtr = std::shared_ptr<Err>;

// 词法错误
struct LexErr : Err {
  LexErrType  type;
  std::string token;

  LexErr(LexErrType type, std::string msg,
    util::Position pos, std::string token
  ) : Err(std::move(msg), pos), type(type),
      token(std::move(token)) {}
  ~LexErr() override = default;
  void display(const ErrReporter &reporter) const override;
};
using LexErrPtr = std::shared_ptr<LexErr>;

// 语法错误
struct ParErr : Err {
  ParErrType  type;
  std::string token;

  ParErr(ParErrType type, std::string msg,
    util::Position pos, std::string token
  ) : Err(std::move(msg), pos), type(type),
      token(std::move(token)) {}
  ~ParErr() override = default;
  void display(const ErrReporter &reporter) const override;
};
using ParErrPtr = std::shared_ptr<ParErr>;

// 语义错误
struct SemErr : Err {
  SemErrType  type;
  std::string scope_name;

  SemErr(SemErrType type, std::string msg,
    util::Position pos, std::string scope_name
  ) : Err(std::move(msg), pos), type(type),
      scope_name(std::move(scope_name)) {}
  ~SemErr() override = default;
  void display(const ErrReporter &reporter) const override;
};
using SemErrPtr = std::shared_ptr<SemErr>;

} // namespace err
