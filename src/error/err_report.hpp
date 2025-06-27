#pragma once

#include <list>
#include <string>
#include <vector>

#include "err_type.hpp"

namespace err {

struct Err {
  std::string msg; // error message

  Err() = default;
  Err(const std::string &m) : msg(m) {}
  Err(std::string &&m) : msg(std::move(m)) {}

  [[nodiscard]]
  virtual constexpr auto kind() const -> ErrType = 0;
  virtual ~Err() = default;
};

// 词法错误
struct LexErr : Err
{
  LexErrType type;
  std::size_t row;
  std::size_t col;
  std::string token;

  LexErr() = delete;

  explicit LexErr(LexErrType type, const std::string &msg,
                  std::size_t r, std::size_t c, std::string token)
    : Err(msg), type(type), row(r), col(c), token(std::move(token))
  {
  }

  ~LexErr() override = default;

  [[nodiscard]] constexpr ErrType
  kind() const override
  {
    return ErrType::Lex;
  }
};

// 语法错误
struct ParErr : Err {
  ParErrType type;
  std::size_t row;
  std::size_t col;
  std::string token;

  ParErr() = delete;

  explicit ParErr(ParErrType type, const std::string &msg,
                  std::size_t r, std::size_t c, std::string token)
    : Err(msg), type(type), row(r), col(c), token(std::move(token))
  {
  }

  ~ParErr() override = default;

  [[nodiscard]] constexpr ErrType
  kind() const override
  {
    return ErrType::Par;
  }
};

// 语义错误
struct SemErr : Err {
  SemErrType type;
  std::size_t row;
  std::size_t col;
  std::string scope_name;

  SemErr() = delete;
  explicit SemErr(SemErrType type, const std::string &msg,
                  std::size_t r, std::size_t c, std::string scope_name)
    : Err(msg), type(type), row(r), col(c), scope_name(std::move(scope_name))
  {
  }

  ~SemErr() override = default;

  [[nodiscard]] constexpr ErrType
  kind() const override
  {
    return ErrType::Sem;
  }
};

// 错误报告器
class ErrReporter {
public:
  ErrReporter() = delete;
  explicit ErrReporter(const std::string &t);

  ~ErrReporter() = default;

public:
  void report(LexErrType type, const std::string &msg, std::size_t r,
              std::size_t c, const std::string &token, bool terminate = false);
  void report(const LexErr &le, bool terminate = false);

  void report(ParErrType type, const std::string &msg, std::size_t r,
              std::size_t c, const std::string &token);
  void report(SemErrType type, const std::string &msg, std::size_t r,
              std::size_t c, const std::string &scope_name);

public:
  void displayLexErrs() const;
  void displayParseErrs() const;
  void displayInternalErrs() const;
  void displaySemanticErrs() const;

  [[nodiscard]] bool hasLexErr() const;
  [[nodiscard]] bool hasParErr() const;
  [[nodiscard]] bool hasSemErr() const;

private:
  [[nodiscard]] bool hasErrs() const;

  void displayLexErr(const LexErr &err) const;
  void displayUnknownType(const LexErr &err) const;

  void displaySemanticErr(const SemErr &err) const;

private:
  std::vector<std::string> text;          // 输入文件原始文本
  std::list<LexErr> lex_errs;           // 词法错误列表
  std::list<ParErr> par_errs;       // 语法错误列表
  std::list<SemErr> sem_errs; // 语义错误列表
};

} // namespace error
