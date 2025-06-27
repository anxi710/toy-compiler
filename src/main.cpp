#include <getopt.h>

#include <tuple>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "ir/ir_gen.hpp"
#include "util/print.hpp"
#include "ast/ast2dot.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "preproc/preproc.hpp"
#include "error/err_report.hpp"
#include "symbol/symbol_table.hpp"
#include "semantic/semantic_checker.hpp"

std::unique_ptr<lex::base::Lexer>     lexer{};     // 词法分析器
std::unique_ptr<par::base::Parser>    pars{};      // 语法分析器
std::shared_ptr<sym::SymbolTable>     stable{};    // 符号表
std::unique_ptr<sem::SemanticChecker> schecker{};  // 语义检查器
std::unique_ptr<ir::IrGenerator>      generator{}; // 中间代码生成器
std::shared_ptr<err::ErrReporter>     reporter{};  // 错误报告器

/**
 * @brief 检查文件流是否正常打开
 * @param fs  fstream/ifstream/ofstream
 * @param msg 错误信息
 */
template <typename T>
inline void
checkFileStream(T &fs, const std::string &msg)
{
  static_assert((
      std::is_same_v<T, std::fstream>  ||
      std::is_same_v<T, std::ofstream> ||
      std::is_same_v<T, std::ifstream>),
    "argument (fs) must be a file stream"
  );

  if (!fs) { // 此时 error reporter 还未初始化
    std::cerr << msg;
    exit(1);
  }
}

/**
 * @brief 编译器初始化
 */
void
initialize(std::ifstream &in)
{
  std::ostringstream oss;
  oss << in.rdbuf();

  // 初始化错误报告器
  reporter = std::make_shared<err::ErrReporter>(oss.str()); // 保留原始文本信息

  std::istringstream iss{preproc::removeAnnotations(oss.str())}; // 删除注释

  std::string line{};
  std::vector<std::string> text{};
  while (std::getline(iss, line)) {
    text.push_back(std::move(line)); // 逐行读取待分析代码
  }

  if (!iss.eof()) {
    std::cerr << "编译器初始化时，文件读取错误" << std::endl;
  }

  // 初始化词法分析器
  lexer = std::make_unique<lex::base::Lexer>(std::move(text));

  // 初始化语义检查器
  schecker = std::make_unique<sem::SemanticChecker>();

  // 初始化符号表
  stable = std::make_shared<sym::SymbolTable>();

  // 初始化中间代码生成器
  generator = std::make_unique<ir::IrGenerator>();

  // 设置错误报告器
  lexer->setErrReporter(reporter);
  schecker->setErrReporter(reporter);

  // 设置符号表
  schecker->setSymbolTable(stable);
  generator->setSymbolTable(stable);
}

/**
 * @brief  参数解析
 * @param  argc argument counter
 * @param  argv argument vector
 * @return tuple: flag_default, flag_parse, flag_token, in_file, out_file
 */
auto
argumentParsing(int argc, char *argv[])
{
  // 定义长选项
  static const struct option long_options[] = {
      {.name = "help",     .has_arg = no_argument,       .flag = nullptr, .val = 'h'},
      {.name = "version",  .has_arg = no_argument,       .flag = nullptr, .val = 'v'},
      {.name = "input",    .has_arg = required_argument, .flag = nullptr, .val = 'i'},
      {.name = "output",   .has_arg = required_argument, .flag = nullptr, .val = 'o'},
      {.name = "token",    .has_arg = no_argument,       .flag = nullptr, .val = 't'},
      {.name = "parse",    .has_arg = no_argument,       .flag = nullptr, .val = 'p'},
      {.name = "semantic", .has_arg = no_argument,       .flag = nullptr, .val = 's'},
      {.name = "generate", .has_arg = no_argument,       .flag = nullptr, .val = 'g'},
      {.name = nullptr,    .has_arg = 0,                 .flag = nullptr, .val = 0} // 结束标志
  };

  int opt{}; // option
  int option_index{0};

  bool flag_token{false}; // 输出 token
  bool flag_parse{false}; // 输出 AST
  bool flag_semantic{false};
  bool flag_generate{false};

  std::string in_file{};  // 输入文件名
  std::string out_file{}; // 输出文件名

  // 参数解析
  while ((opt = getopt_long(argc, argv, "hvVi:o:tpsg", long_options, &option_index)) != -1) {
    switch (opt) {
    case 'h': // help
      util::printHelp(argv[0]);
      exit(0);
    case 'v': // version
    case 'V': // version
      util::printVersion();
      exit(0);
    case 'i': // input
      in_file = std::string{optarg};
      break;
    case 'o': // output
      out_file = std::string{optarg};
      break;
    case 't': // token
      flag_token = true;
      break;
    case 'p': // parse
      flag_parse = true;
      break;
    case 's': // semantic check
      flag_semantic = true;
      break;
    case 'g': // ir generate
      flag_generate = true;
      break;
    case '?': // 无效选项
      std::cerr << "解析到未知参数" << std::endl
                << "尝试运行 \'./toy_compiler --help\' 获取更多信息"
                << std::endl;
      exit(1);
    } // end switch
  } // end while

  if (in_file.empty()) {
    std::cerr << "缺失命令行参数: -i/--input" << std::endl;
    exit(1);
  }

  return std::make_tuple(flag_token, flag_parse, flag_semantic, flag_generate,
                         in_file, out_file);
}

/**
 * @brief 打印分析出的所有 token 到指定文件
 * @param out 输出文件流
 */
bool
printToken(std::ofstream &out)
{
  while (true) {
    if (auto token = lexer->nextToken(); token.has_value()) {
      if (token->getType() == lex::token::TokenType::END) {
        break;
      }
      out << token.value().toString() << std::endl;
    } else { // 未正确识别 token
      reporter->report(token.error());
    }
  }
  out.flush();

  if (reporter->hasLexErr()) {
    reporter->displayLexErrs();
    return false;
  }

  return true;
}

/**
 * @brief 以 dot 格式打印 AST
 * @param out 输出文件流
 */
bool
printAST(std::ofstream &out)
{ // 初始化 parser
  auto nextTokenFunc = []() {
    return lexer->nextToken(); // 封装 nextToken() 方法
  };
  pars = std::make_unique<par::base::Parser>(nextTokenFunc);

  auto p_prog = pars->parseProgram();
  std::cout << "Parsing success" << std::endl;

  par::ast::ast2Dot(out, p_prog);

  return true;
}

/**
 * @brief 检查语义
 * @param out 输出文件流
 */
auto checkSemantic(std::ofstream &out) -> bool {
  auto nextTokenFunc = []() {
    return lexer->nextToken(); // 封装 nextToken() 方法
  };
  pars = std::make_unique<par::base::Parser>(nextTokenFunc);

  auto p_prog = pars->parseProgram();
  schecker->checkProg(p_prog);

  if (reporter->hasSemErr()) {
    reporter->displaySemanticErrs();
    return false;
  }

  stable->printSymbol(out);

  return true;
}

/**
 * @brief 生成中间代码
 * @param out 输出文件流
 */
auto generateIr(std::ofstream &out) -> bool {
  auto nextTokenFunc = []() {
    return lexer->nextToken(); // 封装 nextToken() 方法
  };
  pars = std::make_unique<par::base::Parser>(nextTokenFunc);

  auto p_prog = pars->parseProgram();
  schecker->checkProg(p_prog);
  if (reporter->hasSemErr()) {
    reporter->displaySemanticErrs();
    return false;
  }

  generator->generateProg(p_prog);

  generator->printQuads(out);

  return true;
}

/**
 * @brief   toy compiler 主函数
 * @details 拼装各组件
 */
int
main(int argc, char *argv[])
{
  auto [flag_token, flag_parse, flag_semantic, flag_generate, in_file,
        out_file] = argumentParsing(argc, argv);

  std::ifstream in{};
  std::ofstream out_token{};
  std::ofstream out_parse{};
  std::ofstream out_semantic{};
  std::ofstream out_generate{};

  in.open(in_file);
  checkFileStream(in, std::string{"Failed to open input file."});

  std::string base = out_file.empty() ? "output" : out_file;
  out_token.open(base + std::string{".token"});
  out_parse.open(base + std::string{".dot"});
  out_semantic.open(base + std::string{".symbol"});
  out_generate.open(base + std::string{".ir"});

  checkFileStream(out_token, std::string{"Failed to open output file (token)"});
  checkFileStream(out_parse, std::string{"Failed to open output file (parse)"});
  checkFileStream(out_semantic,
                  std::string{"Failed to open output file (semantic)"});
  checkFileStream(out_generate,
                  std::string{"Failed to open output file (ir generate)"});

  initialize(in);

  bool token_ok{false};
  bool parse_ok{false};
  bool semantic_ok{false};
  bool generate_ok{false};

  if (flag_token) {
    token_ok = printToken(out_token);
  }
  if (flag_parse) {
    lexer->reset(util::Position(0, 0));
    parse_ok = printAST(out_parse);
  }
  if (flag_semantic) {
    lexer->reset(util::Position(0, 0));
    semantic_ok = checkSemantic(out_semantic);
  }
  if (flag_generate) {
    lexer->reset(util::Position(0, 0));
    generate_ok = generateIr(out_generate);
  }

  in.close();
  out_token.close();
  out_parse.close();
  out_semantic.close();
  out_generate.close();

  if (!flag_token || !token_ok) {
    std::filesystem::remove(base + ".token");
  }
  if (!flag_parse || !parse_ok) {
    std::filesystem::remove(base + ".dot");
  }
  if (!flag_semantic || !semantic_ok) {
    std::filesystem::remove(base + ".symbol");
  }
  if (!flag_generate || !generate_ok) {
    std::filesystem::remove(base + ".ir");
  }

  return 0;
}
