#include <cassert>
#include <sstream>
#include <iostream>

#include "err_report.hpp"

namespace err {

// 控制字符
static inline constexpr std::string BOLD   = "\033[1m";
static inline constexpr std::string RESET  = "\033[0m";
static inline constexpr std::string RED    = "\033[1;31m";
static inline constexpr std::string BLUE   = "\033[1;34m";
static inline constexpr std::string YELLOW = "\033[1;33m";

/*---------------- LexErr ----------------*/

/**
 * @brief 控制台输出未知Token
 * @param err  词法错误实例
 */
void
ErrReporter::displayUnknownType(const LexErr &err) const
{
  std::cerr << BOLD << YELLOW << "warning[UnknownToken]" << RESET << BOLD
    << std::format(": 识别到未知 token '{}'", err.token) << RESET << std::endl;

  std::cerr << BLUE << " --> " << RESET
    << std::format("<row: {}, col: {}>", err.pos.row + 1, err.pos.col + 1)
    << std::endl;

  std::cerr << BLUE << "  |  " << std::endl
    << BLUE << " " << err.pos.row + 1 << " | " << RESET
    << this->text[err.pos.row] << std::endl;

  std::ostringstream oss;
  std::print(oss, " {} | ", err.pos.row + 1);
  int delta = oss.str().length() + err.pos.col - 3;
  std::cerr << BLUE << "  |" << std::string(delta, ' ') << "^" << RESET
    << std::endl << std::endl;
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
    case LexErrType::UNKNOWN_TOKEN:
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
  //DEBUG
  switch (type) {
    case SemErrType::ARG_CNT_MISMATCH:
      return {
        "ArgMismatch",
        "函数参数个数不匹配"
      };
    case SemErrType::VOID_FUNC_RET_VAL:
    case SemErrType::RETTYPE_MISMATCH:
      return {
        "FuncReturnMismatch",
        "函数返回值类型不匹配"
      };
    case SemErrType::MISSING_RETVAL:
      return {
        "MissingReturnValue",
        "函数有返回值但未返回任何值"
      };
    case SemErrType::CALL_UNDEFINED_FUNC:
      return {
        "UndefinedFunction",
        "函数未定义"
      };
    case SemErrType::UNDECLARED_VAR:
      return {
        "UndeclaredVariable",
        "变量未声明"
      };
    case SemErrType::UNINITIALIZED_VAR:
      return {
        "UninitializedVariable",
        "变量未初始化"
      };
    case SemErrType::ASSIGN_NOT_VAR:
      return {
        "InvalidAssignment",
        "无效赋值语句"
      };
    case SemErrType::ASSIGN_UNDECLARED_VAR:
      return {
        "InvalidAssignment",
        "无效赋值语句"
      };
    case SemErrType::TYPE_INFER_FAILURE:
      return {
        "TypeInferenceFailure",
        "变量无法通过自动类型推导确定类型"
      };
    case SemErrType::TYPE_MISMATCH:
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
  std::cerr << BOLD << RED << std::format("Err[{}]", pair.first) << RESET << BOLD
    << ": " << pair.second << RESET << std::endl;

  std::cerr << BLUE << "--> " << RESET <<
    std::format(
      "scope: {} <row: {}, col: {}>",
      err.scope_name, err.pos.row + 1, err.pos.col + 1
    ) << std::endl;

  std::cerr << BLUE << "  |  " << std::endl
    << BLUE << " " << err.pos.row + 1 << " | " << RESET
    << this->text[err.pos.row] << std::endl;

  std::ostringstream oss;
  std::print(oss, " {} | ", err.pos.row + 1);
  int delta = oss.str().length() + err.pos.col - 3;
  std::cerr << BLUE << "  |" << std::string(delta, ' ') << "^" << RESET
    << std::endl << std::endl;

  std::cerr << std::format("    Details: {}", err.msg)
    << std::endl << std::endl;
}

/*---------------- SemErr ----------------*/

/*---------------- ErrReporter ----------------*/

ErrReporter::ErrReporter(const std::string &t)
{
  std::istringstream iss{t};

  std::string line{};
  while (std::getline(iss, line)) {
    this->text.push_back(line);
  }

  assert(iss.eof());
}

/**
 * @brief 报告词法错误
 * @param type      词法错误类型
 * @param msg       错误信息
 * @param token     错误发生的 token
 */
void
ErrReporter::report(LexErrType type, const std::string &msg,
  util::Position pos, const std::string &token)
{
  lex_errs.emplace_back(type, msg, pos, token);
}

/**
 * @brief 报告语法错误
 * @param type      词法错误类型
 * @param msg       错误信息
 * @param pos       错误发生的位置
 * @param token     错误发生的 token
 */
void
ErrReporter::report(ParErrType type, const std::string &msg,
  util::Position pos, const std::string &token)
{
  par_errs.emplace_back(type, msg, pos, token);
}

/**
 * @brief 报告语义错误
 * @param type       语义错误类型
 * @param msg        错误信息
 * @param pos       错误发生的位置
 * @param scope_name 作用域
 */
void
ErrReporter::report(SemErrType type, const std::string &msg,
  util::Position pos, const std::string &scope_name)
{
  sem_errs.emplace_back(type, msg, pos, scope_name);
}

void
ErrReporter::displayLexErrs() const
{
  for (const auto &lex_err : lex_errs) {
    displayLexErr(lex_err);
  }
}

void
ErrReporter::displaySemanticErrs() const
{
  for (const auto &sem_err : sem_errs) {
    displaySemanticErr(sem_err);
  }
}


bool
ErrReporter::hasLexErr() const
{
  return !lex_errs.empty();
}

bool
ErrReporter::hasParErr() const
{
  return !par_errs.empty();
}

bool
ErrReporter::hasSemErr() const
{
  return !sem_errs.empty();
}

bool
ErrReporter::hasErrs() const
{
  return !(lex_errs.empty() && par_errs.empty() && sem_errs.empty());
}

/*---------------- ErrReporter ----------------*/

} // namespace err
