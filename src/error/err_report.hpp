#pragma once

#include <vector>

#include "position.hpp"
#include "err_type.hpp"

namespace err {

// 错误报告器
class ErrReporter {
public:
  ErrReporter(const std::string &t);
  ~ErrReporter() = default;

public:
  void report(LexErrType type, const std::string &msg,
    util::Position pos, const std::string &token);

  void report(ParErrType type, const std::string &msg,
    util::Position pos, const std::string &token);
  void report(SemErrType type, const std::string &msg,
    util::Position pos, const std::string &scope_name);

public:
  void displayLexErrs() const;
  void displayParseErrs() const;
  void displayInternalErrs() const;
  void displaySemanticErrs() const;

  bool hasLexErr() const;
  bool hasParErr() const;
  bool hasSemErr() const;

private:
  bool hasErrs() const;

  void displayLexErr(const LexErr &err) const;
  void displayUnknownType(const LexErr &err) const;

  void displaySemanticErr(const SemErr &err) const;

private:
  std::vector<std::string> text;     // 输入文件原始文本
  std::vector<LexErr>      lex_errs; // 词法错误列表
  std::vector<ParErr>      par_errs; // 语法错误列表
  std::vector<SemErr>      sem_errs; // 语义错误列表
};

} // namespace error
