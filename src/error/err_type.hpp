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

#define SEMANTIC_ERROR_LIST(_) \
  _(MISSING_RETVAL, "MissingReturnValue", "函数有返回值但未返回任何值") \
  _(TYPE_INFER_FAILURE, "TypeInferenceFailure", "变量无法通过自动类型推导确定类型") \
  _(TYPE_MISMATCH, "TypeMismatch", "变量类型不匹配") \
  _(UNEXPECTED_EXPR_TYPE, "UnexpectedExprType", "意料之外的表达式类型") \
  _(BREAK_CTX_ERROR, "BreakContextError", "break 所处上下文有误") \
  _(BREAK_TYPE_MISMATCH, "BreakTypeMismatch", "break 返回的表达式的类型与推导不一致") \
  _(CONTINUE_CTX_ERROR, "ContinueContextError", "continue 所处上下文有误") \
  _(UNDECLARED_VAR, "UndeclaredVariable", "变量未声明") \
  _(OUT_OF_BOUNDS_ACCESS, "OutOfBoundsAccess", "访问越界") \
  _(ASSIGN_IMMUTABLE, "AssignImmutable", "赋值表达式左值不可变") \
  _(ASSIGN_MISMATCH, "AssignMismatch", "赋值表达式左右值类型不匹配") \
  _(INCOMPARABLE_TYPES, "IncomparableTypes", "左值和右值的类型不可比较") \
  _(NON_COMPUTABLE_TYPES, "Non-ComputableTypes", "左值和右值的类型不可进行算术运算") \
  _(RETTYPE_MISMATCH, "FuncReturnMismatch", "函数返回值类型不匹配") \
  _(CALL_UNDECLARED_FUNC, "UndefinedFunction", "函数未定义") \
  _(ARG_CNT_MISMATCH, "ArgMismatch", "函数参数个数不匹配") \
  _(ARG_TYPE_MISMATCH, "ArgTypeMismatch", "函数参数类型不匹配") \
  _(MISSING_ELSE, "MissingElse", "if 表达式缺少 else 分支") \
  _(UNINITIALIZED_VAR, "UninitializedVariable", "变量未初始化")

// 语义错误码
enum class SemErrType : std::uint8_t {
#define DEFINE_ENUM(name, _, __) name,
  SEMANTIC_ERROR_LIST(DEFINE_ENUM)
#undef DEFINE_ENUM
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
