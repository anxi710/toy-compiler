#pragma once

#include <list>
#include <string>
#include <vector>

#include "err_type.hpp"

namespace err {

// 错误报告器
class ErrReporter {
public:
  ErrReporter(const std::string &t);
  ~ErrReporter() = default;

public:
  void report(LexErrType type, const std::string &msg, std::size_t r,
              std::size_t c, const std::string &token, bool terminate = false);
  void report(const LexErr &le, bool terminate = false);

  void report(ParErrType type, const std::string &msg, std::size_t r,
              std::size_t c, const std::string &token);
  void report(SemErrType type, const std::string &msg, std::size_t r,
              std::size_t c, const std::string &scope_name);

  [[noreturn]] void terminateProg();

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
