#include "err_report.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

namespace err {

// 控制字符
static inline const std::string BOLD   = "\033[1m";
static inline const std::string RESET  = "\033[0m";
static inline const std::string RED    = "\033[1;31m";
static inline const std::string BLUE   = "\033[1;34m";
static inline const std::string YELLOW = "\033[1;33m";

/*---------------- LexErr ----------------*/

/**
 * @brief 控制台输出未知Token
 * @param err  词法错误实例
 */
void
ErrReporter::displayUnknownType(const LexErr &err) const
{
  std::cerr << BOLD << YELLOW << "warning[UnknownToken]" << RESET << BOLD
            << ": 识别到未知 token '" << err.token << "'" << RESET << std::endl;

  std::cerr << BLUE << " --> " << RESET << "<row: " << err.row + 1
            << ", col: " << err.col + 1 << ">" << std::endl;

  std::cerr << BLUE << "  |  " << std::endl
            << BLUE << " " << err.row + 1 << " | " << RESET
            << this->text[err.row] << std::endl;

  std::ostringstream oss;
  oss << " " << err.row + 1 << " | ";
  int delta = oss.str().length() + err.col - 3;
  std::cerr << BLUE << "  |" << std::string(delta, ' ') << "^" << RESET
            << std::endl
            << std::endl;
}

/**
 * @brief 分发处理词法错误
 * @param err  词法错误实例
 */
void
ErrReporter::displayLexErr(const LexErr &err) const
{
  switch (err.type) {
  default:
    break;
  case LexErrType::UnknownToken:
    displayUnknownType(err);
    break;
  }
}

/*---------------- LexErr ----------------*/

/*---------------- ParErr ----------------*/

/*---------------- ParErr ----------------*/

/*---------------- SemErr ----------------*/

/**
 * @brief 分发处理不同种类语义错误
 * @param type  语义错误种类
 * @return 根据type返回特定的错误报告提示
 */
std::pair<std::string, std::string>
displaySemErrType(SemErrType type)
{
  switch (type) {
  case SemErrType::ArgCountMismatch:
    return {
      "ArgMismatch",
      "函数参数个数不匹配"
    };
  case SemErrType::VoidFuncReturnValue:
  case SemErrType::FuncReturnTypeMismatch:
    return {
      "FuncReturnMismatch",
      "函数返回值类型不匹配"
    };
  case SemErrType::MissingReturnValue:
    return {
      "MissingReturnValue",
      "函数有返回值但未返回任何值"
    };
  case SemErrType::UndefinedFunctionCall:
    return {
      "UndefinedFunction",
      "函数未定义"
    };
  case SemErrType::UndeclaredVariable:
    return {
      "UndeclaredVariable",
      "变量未声明"
    };
  case SemErrType::UninitializedVariable:
    return {
      "UninitializedVariable",
      "变量未初始化"
    };
  case SemErrType::AssignToNonVariable:
    return {
      "InvalidAssignment",
      "无效赋值语句"
    };
  case SemErrType::AssignToUndeclaredVar:
    return {
      "InvalidAssignment",
      "无效赋值语句"
    };
  case SemErrType::TypeInferenceFailure:
    return {
      "TypeInferenceFailure",
      "变量无法通过自动类型推导确定类型"
    };
  case SemErrType::TypeMismatch:
    return {
      "TypeMismatch",
      "变量类型不匹配"
    };
  }
}

/**
 * @brief 控制台输出语义错误结果
 * @param err  语义错误实例
 */
void
ErrReporter::displaySemanticErr(const SemErr &err) const
{
  auto pair = displaySemErrType(err.type);
  std::cerr << BOLD << RED << "Err[" << pair.first << "]" << RESET << BOLD
            << ": " << pair.second << RESET << std::endl;

  std::cerr << BLUE << "--> " << RESET << "scope: " << err.scope_name
            << " <row: " << err.row + 1 << ", col: " << err.col + 1 << ">"
            << std::endl;

  std::cerr << BLUE << "  |  " << std::endl
            << BLUE << " " << err.row + 1 << " | " << RESET
            << this->text[err.row] << std::endl;

  std::ostringstream oss;
  oss << " " << err.row + 1 << " | ";
  int delta = oss.str().length() + err.col - 3;
  std::cerr << BLUE << "  |" << std::string(delta, ' ') << "^" << RESET
            << std::endl
            << std::endl;

  std::cerr << "    Details: " << err.msg << std::endl << std::endl;
}

/*---------------- SemErr ----------------*/

/*---------------- ErrReporter ----------------*/

ErrReporter::ErrReporter(const std::string &t)
{
  std::istringstream iss{t};

  std::string line{};
  while (std::getline(iss, line))
  {
    this->text.push_back(line);
  }

  assert(iss.eof());
}

/**
 * @brief 报告词法错误
 * @param type      词法错误类型
 * @param msg       错误信息
 * @param r         错误发生的行数
 * @param c         错误发生的列数
 * @param token     错误发生的 token
 * @param terminate 是否终止
 */
void
ErrReporter::report(LexErrType type, const std::string &msg,
                    std::size_t r, std::size_t c, const std::string &token, bool terminate)
{
  lex_errs.emplace_back(type, msg, r, c, token);

  if (terminate)
  {
  }
}

void
ErrReporter::report(const LexErr &le, bool terminate)
{
  lex_errs.push_back(le);

  if (terminate)
  {
  }
}

/**
 * @brief 报告语法错误
 * @param type      词法错误类型
 * @param msg       错误信息
 * @param r         错误发生的行数
 * @param c         错误发生的列数
 * @param token     错误发生的 token
 * @param terminate 是否终止
 */
void
ErrReporter::report(ParErrType type, const std::string &msg,
                    std::size_t r, std::size_t c, const std::string &token)
{
  par_errs.emplace_back(type, msg, r, c, token);
}

/**
 * @brief 报告语义错误
 * @param type       语义错误类型
 * @param msg        错误信息
 * @param r          错误发生的行数
 * @param c          错误发生的列数
 * @param scope_name 作用域
 * @param terminate  是否终止
 */
void
ErrReporter::report(SemErrType type, const std::string &msg,
                    std::size_t r, std::size_t c, const std::string &scope_name)
{
  sem_errs.emplace_back(type, msg, r, c, scope_name);
}

/**
 * @brief 处理所有词法错误
 */
void
ErrReporter::displayLexErrs() const
{
  for (const auto &lex_err : lex_errs) {
    displayLexErr(lex_err);
  }
}

/**
 * @brief 处理所有语义错误
 */
void
ErrReporter::displaySemanticErrs() const
{
  for (const auto &sem_err : sem_errs) {
    displaySemanticErr(sem_err);
  }
}


[[nodiscard]] bool
ErrReporter::hasLexErr() const
{
  return !lex_errs.empty();
}

[[nodiscard]] bool
ErrReporter::hasParErr() const
{
  return !par_errs.empty();
}

[[nodiscard]] bool
ErrReporter::hasSemErr() const
{
  return !sem_errs.empty();
}

[[nodiscard]] bool
ErrReporter::hasErrs() const
{
  return !(lex_errs.empty() && par_errs.empty() && sem_errs.empty());
}

/*---------------- ErrReporter ----------------*/

} // namespace err
