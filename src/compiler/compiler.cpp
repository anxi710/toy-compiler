#include <fstream>
#include <sstream>
#include <iostream>

#include "preproc.hpp"
#include "compiler.hpp"

namespace cpr {

Compiler::Compiler(const std::string &file)
{
  // 读取输入文件
  std::ifstream in;
  in.open(file);
  if (!in) {
    std::cerr << "无法打开输入文件" << std::endl;
    exit(1);
  }

  std::ostringstream oss;
  oss << in.rdbuf();

  // 初始化错误报告器
  ereporter = std::make_unique<err::ErrReporter>(oss.str()); // 保留原始文本信息

  // 删除注释
  std::istringstream iss{preproc::removeAnnotations(oss.str())};

  std::string line{};
  std::vector<std::string> text{};
  while (std::getline(iss, line)) {
    text.push_back(std::move(line)); // 逐行读取待分析代码
  }

  if (!iss.eof()) {
    std::cerr << "编译器初始化时，文件读取错误" << std::endl;
  }

  // 初始化各组件
  lexer    = std::make_unique<lex::base::Lexer>(text, *ereporter);
  stable   = std::make_unique<sym::SymbolTable>();
  schecker = std::make_unique<sem::SemanticChecker>(*stable, *ereporter);
  parser   = std::make_unique<par::base::Parser>(*lexer, *stable, *schecker, *ereporter);
}

} // namespace cpr
