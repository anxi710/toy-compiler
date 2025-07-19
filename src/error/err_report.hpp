#pragma once

#include <print>
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
  void displayErrs() const;
  void displayUnknownType(const LexErr &err) const;
  void displayUnexpectedToken(const ParErr &err) const;
  void displaySemErr(const SemErr &err) const;

  [[nodiscard]] bool hasErrs() const;

private:
  void displaySrc(const util::Position &pos) const;

private:
  std::vector<std::string> text; // 输入文件原始文本
  std::vector<ErrPtr>      errs; // 错误列表
};

/**
 * @brief 打印收集到的错误后终止程序
 */
[[noreturn]] inline void
terminate(err::ErrReporter &reporter) {
  if (reporter.hasErrs()) {
    reporter.displayErrs();
  }

  std::println(stderr, "");
  std::println(stderr, "程序出错，终止运行！");
  exit(1);
}

} // namespace err
