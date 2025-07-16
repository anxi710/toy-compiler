#include <print>
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
 * @brief 打印未知 Token 错误
 * @param err 词法错误实例
 */
void
ErrReporter::displayUnknownType(const LexErr &err) const
{
  std::cerr << BOLD << RED << "Lexer Error[UnknownToken]" << RESET << BOLD
    << std::format(": 识别到未知 token '{}'", err.token) << RESET << std::endl;

  std::cerr << BLUE << " ---> " << RESET
    << std::format("{}", err.pos + util::Position{1, 1})
    << std::endl;

  std::ostringstream oss;
  oss << std::format("{:<3}", err.pos.row + 1);

  std::cerr << BLUE << "   |  " << std::endl
    << BLUE << oss.str() << "| " << RESET
    << this->text[err.pos.row] << std::endl;

  int delta = oss.str().length() + err.pos.col - 2;
  std::cerr << BLUE << "   |" << std::string(delta, ' ') << "^" << RESET
    << std::endl;
}

/**
 * @brief 分发处理词法错误
 * @param reporter 错误处理器实例
 */
void
LexErr::display(const ErrReporter &reporter) const
{
  switch (type) {
    case LexErrType::UNKNOWN_TOKEN:
      reporter.displayUnknownType(*this);
      break;
  }
}

/*---------------- LexErr ----------------*/

/*---------------- ParErr ----------------*/

/**
 * @brief 打印 unexpected token 错误
 * @param err 语法错误实例
 */
void
ErrReporter::displayUnexpectedToken(const ParErr &err) const
{
  std::cerr << BOLD << RED << "Parser Error[UnexpectedToken]" << RESET << BOLD
    << std::format(": 意料之外的 token '{}'", err.token) << RESET << std::endl;

  std::cerr << BLUE << " ---> " << RESET
    << std::format("{}", err.pos + util::Position{1, 1})
    << std::endl;

  std::ostringstream oss;
  oss << std::format("{:<3}", err.pos.row + 1);

  std::cerr << BLUE << "   |  " << std::endl
    << BLUE << oss.str() << "| " << RESET
    << this->text[err.pos.row] << std::endl;

  int delta = oss.str().length() + err.pos.col - 2;
  std::cerr << BLUE << "   |" << std::string(delta, ' ') << "^" << RESET
    << std::endl;

  std::cerr << BLUE << "   |" << std::endl;
  std::cerr << BLUE << "   =" << RESET << std::format(" Details: {}", err.msg)
    << std::endl << std::endl;
}

/**
 * @brief 分发打印不同的语法错误
 * @param reporter 错误报告器实例
 */
void
ParErr::display(const ErrReporter &reporter) const
{
  reporter.displayUnexpectedToken(*this);
}

/*---------------- ParErr ----------------*/

/*---------------- SemErr ----------------*/

/**
 * @brief  分发处理不同的语义错误
 * @param  type 语义错误类型
 * @return 根据 type 返回特定的错误报告提示
 */
std::pair<std::string, std::string>
displaySemErrType(SemErrType type)
{
  switch (type) {
    case SemErrType::MISSING_RETVAL:
      return {
        "MissingReturnValue",
        "函数有返回值但未返回任何值"
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
    case SemErrType::UNEXPECTED_EXPR_TYPE:
      return {
        "UnexpectedExprType",
        "意料之外的表达式类型"
      };
    case SemErrType::BREAK_CTX_ERROR:
      return {
        "BreakContextError",
        "break 所处上下文有误"
      };
    case SemErrType::BREAK_TYPE_MISMATCH:
      return {
        "BreakTypeMismatch",
        "break 返回的表达式的类型与推导不一致"
      };
    case SemErrType::CONTINUE_CTX_ERROR:
      return {
        "ContinueContextError",
        "continue 所处上下文有误"
      };
    case SemErrType::UNDECLARED_VAR:
      return {
        "UndeclaredVariable",
        "变量未声明"
      };
    case SemErrType::OUTOFBOUNDS_ACCESS:
      return {
        "OutOfBoundsAccess",
        "访问越界"
      };
    case SemErrType::ASSIGN_IMMUTABLE:
      return {
        "AssignImmutable",
        "赋值表达式左值不可变"
      };
    case SemErrType::ASSIGN_MISMATCH:
      return {
        "AssignMismatch",
        "赋值表达式左右值类型不匹配"
      };
    case SemErrType::INCOMPARABLE_TYPES:
      return {
        "IncomparableTypes",
        "左值和右值的类型不可比较"
      };
    case SemErrType::UNCOMPUTABLE_TYPES:
      return {
        "UncomputableTypes",
        "左值和右值的类型不可进行算术运算"
      };
    case SemErrType::RETTYPE_MISMATCH:
      return {
        "FuncReturnMismatch",
        "函数返回值类型不匹配"
      };
    case SemErrType::CALL_UNDECLARED_FUNC:
      return {
        "UndefinedFunction",
        "函数未定义"
      };
    case SemErrType::ARG_CNT_MISMATCH:
      return {
        "ArgMismatch",
        "函数参数个数不匹配"
      };
    case SemErrType::ARG_TYPE_MISMATCH:
      return {
        "ArgTypeMismatch",
        "函数参数类型不匹配"
      };
    case SemErrType::MISSING_ELSE:
      return {
        "MissignElse",
        "if 表达式缺少 else 分支"
      };
    case SemErrType::UNINITIALIZED_VAR:
      return {
        "UninitializedVariable",
        "变量未初始化"
      };
  }
}

/**
 * @brief 打印语义错误
 * @param err 语义错误实例
 */
void
ErrReporter::displaySemErr(const SemErr &err) const
{
  auto pair = displaySemErrType(err.type);
  std::cerr << BOLD << RED << std::format("Semantic Error[{}]", pair.first) << RESET << BOLD
    << ": " << pair.second << RESET << std::endl;

  std::cerr << BLUE << "  --> " << RESET <<
    std::format(
      "scope: {} {}",
      err.scope_name,
      err.pos + util::Position{1, 1}
    ) << std::endl;

  std::ostringstream oss;
  oss << std::format("{:<3}", err.pos.row + 1);

  std::cerr << BLUE << "   |  " << std::endl
    << BLUE << oss.str() << "| " << RESET
    << this->text[err.pos.row] << std::endl;

  int delta = oss.str().length() + err.pos.col - 2;
  std::cerr << BLUE << "   |" << std::string(delta, ' ') << "^" << RESET
    << std::endl;

  std::cerr << BLUE << "   |" << std::endl;
  std::cerr << BLUE << "   =" << RESET << std::format(" Details: {}", err.msg)
    << std::endl << std::endl;
}

/**
 * @brief 打印语义错误
 * @param reporter 错误报告器实例
 */
void
SemErr::display(const ErrReporter &reporter) const
{
  reporter.displaySemErr(*this);
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
 * @param type  词法错误类型
 * @param msg   错误信息
 * @param pos   错误发生的位置
 * @param token 错误发生的 token
 */
void
ErrReporter::report(LexErrType type, const std::string &msg,
  util::Position pos, const std::string &token)
{
  auto lexerr = std::make_shared<LexErr>(type, msg, pos, token);
  errs.push_back(lexerr);
}

/**
 * @brief 报告语法错误
 * @param type  语法错误类型
 * @param msg   错误信息
 * @param pos   错误发生的位置
 * @param token 错误发生的 token
 */
void
ErrReporter::report(ParErrType type, const std::string &msg,
  util::Position pos, const std::string &token)
{
  auto parerr = std::make_shared<ParErr>(type, msg, pos, token);
  errs.push_back(parerr);
}

/**
 * @brief 报告语义错误
 * @param type       语义错误类型
 * @param msg        错误信息
 * @param pos        错误发生的位置
 * @param scope_name 作用域
 */
void
ErrReporter::report(SemErrType type, const std::string &msg,
  util::Position pos, const std::string &scope_name)
{
  auto semerr = std::make_shared<SemErr>(type, msg, pos, scope_name);
  errs.push_back(semerr);
}

/**
 * @brief 打印错误
 */
void
ErrReporter::displayErrs() const
{
  for (const auto &err : errs) {
    err->display(*this);
  }

  // 输出错误总数
  std::cerr << RED << "Error" << RESET;
  std::println(": {} errors emitted", errs.size());
}

/**
 * @brief 是否检测出错误
 */
bool
ErrReporter::hasErrs() const
{
  return !errs.empty();
}

/*---------------- ErrReporter ----------------*/

} // namespace err
