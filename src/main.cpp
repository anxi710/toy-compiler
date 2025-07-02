#include <getopt.h>

#include <tuple>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "compiler.hpp"

/**
 * @brief 打印版本信息
 */
void
printVersion()
{
  /*
   * 使用语义版本控制 (SemVer) 原则设置版本号
   * major.minor.patch
   */
  std::cout << "Toy compiler: version 0.6.2" << std::endl
            << "This is a toy compiler developed by xh, csx and qsw."
            << std::endl
            << "Have fun with it!" << std::endl;
}

/**
 * @brief 打印帮助信息
 * @param exec 可执行文件名
 */
void
printHelp(const char *const exec)
{
  std::cout
      << "Usage: " << exec << " [options]" << std::endl
      << std::endl
      << "This is a Rust-like programming language compiler." << std::endl
      << std::endl
      << "Options:" << std::endl
      << "  -h, --help             show help" << std::endl
      << "  -v, -V, --version      show version" << std::endl
      << "  -i, --input filename   set input file (with suffix, must set an "
         "input filename)"
      << std::endl
      << "  -o, --output filename  set output file (without suffix)"
      << std::endl
      << "  -t, --token            output the segmented token only" << std::endl
      << "  -p, --parse            output the abstract syntax tree (AST) only"
      << std::endl
      << "  -s, --semantic         check the semantics only" << std::endl
      << "  -g, --generate         generate IR only" << std::endl
      << std::endl
      << "Examples:" << std::endl
      << "  $ path/to/toy_compiler -t -i test.txt" << std::endl
      << "  $ path/to/toy_compiler -p -i test.txt" << std::endl
      << "  $ path/to/toy_compiler -t -p -i test.txt" << std::endl
      << "  $ path/to/toy_compiler -s -i test.txt -o output" << std::endl
      << std::endl
      << "Tips:" << std::endl
      << "  Upon completion of the program execution, you can run this command"
      << std::endl
      << "  to generate the abstract syntax tree diagram:" << std::endl
      << "    $ dot -Tpng path/to/output.dot -o AST.png" << std::endl;
}

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
        printHelp(argv[0]);
        exit(0);
      case 'v': // version
      case 'V': // version
        printVersion();
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
 * @brief   toy compiler 主函数
 * @details 拼装各组件
 */
int
main(int argc, char *argv[])
{
  auto [flag_token, flag_parse, flag_semantic, flag_generate, in_file,
        out_file] = argumentParsing(argc, argv);

  cpr::Compiler compiler(in_file);

  std::ifstream in{};
  std::ofstream out_token{};
  std::ofstream out_parse{};
  std::ofstream out_semantic{};
  std::ofstream out_generate{};


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

  bool token_ok{false};
  bool parse_ok{false};
  bool semantic_ok{false};
  bool generate_ok{false};


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
