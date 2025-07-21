#include <getopt.h>

#include <print>

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
  std::println("Toy compiler: version 0.7.1");
  std::println("This is a toy compiler developed by xh, csx and qsw.");
  std::println("Have fun with it!");
}

/**
 * @brief 打印帮助信息
 * @param exec 可执行文件名
 */
void
printHelp(const char *const exec)
{
  std::println("Usage: {} [options]", exec);
  std::println("");
  std::println("This is a Rust-like programming language compiler.");
  std::println("");
  std::println("Options:");
  std::println("  -h, --help             show help");
  std::println("  -v, -V, --version      show version");
  std::println("  -i, --input filename   set input file (with suffix, must set an input filename)");
  std::println("  -o, --output filename  set output file (without suffix)");
  std::println("  --ir,                  generate IR only");
  std::println("");
  std::println("Examples:");
  std::println("  $ path/to/toy_compiler --ir -i test.txt");
  std::println("  $ path/to/toy_compiler --ir -i test.txt -o output");
  std::println("");
  std::println("Tips:");
  std::println("  Upon completion of the program execution, you can run this command");
  std::println("  to generate the abstract syntax tree diagram:");
  std::println("    $ dot -Tpng path/to/output.dot -o AST.png");
}

// 定义长选项
static constexpr struct option options[] = {
    {.name = "help",    .has_arg = no_argument,       .flag = nullptr, .val = 'h'},
    {.name = "version", .has_arg = no_argument,       .flag = nullptr, .val = 'v'},
    {.name = "input",   .has_arg = required_argument, .flag = nullptr, .val = 'i'},
    {.name = "output",  .has_arg = required_argument, .flag = nullptr, .val = 'o'},
    {.name = "ir",      .has_arg = no_argument,       .flag = nullptr, .val = 'r'},
    {.name = nullptr,   .has_arg = 0,                 .flag = nullptr, .val = 0} // 结束标志
};

/**
 * @brief  参数解析
 * @param  argc argument counter
 * @param  argv argument vector
 * @return tuple: flag_ir, in_file, out_file
 */
auto
argumentParsing(int argc, char *argv[])
{
  int opt; // option

  bool flag_ir = false;

  std::string in_file{};  // 输入文件名
  std::string out_file{}; // 输出文件名

  // 参数解析
  while ((opt = getopt_long(argc, argv, "hvVi:o:r", options, nullptr)) != -1) {
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
      case 'r': // ir
        flag_ir = true;
        break;
      case '?': // 无效选项
        std::println(stderr, "解析到未知参数");
        std::println(stderr, "尝试运行 \'./toy_compiler --help\' 获取更多信息");
        exit(1);
    } // end switch
  } // end while

  if (in_file.empty()) {
    std::println(stderr, "缺失命令行参数: -i/--input");
    exit(1);
  }

  return std::make_tuple(flag_ir, in_file, out_file);
}

/**
 * @brief   主函数
 * @details 初始化 compiler 并调用其提供的函数完成任务
 */
int
main(int argc, char *argv[])
{
  auto [flag_ir, in_file, out_file] = argumentParsing(argc, argv);

  cpr::Compiler compiler(in_file);

  if (flag_ir) {
    compiler.generateIR(out_file);
  }

  return 0;
}
