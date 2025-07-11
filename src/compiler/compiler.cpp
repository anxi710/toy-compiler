#include <print>
#include <fstream>
#include <sstream>

#include "panic.hpp"
#include "preproc.hpp"
#include "compiler.hpp"

namespace cpr {

Compiler::Compiler(const std::string &file)
{
  // 读取输入文件
  std::ifstream in;
  in.open(file);
  if (!in) {
    util::runtime_error("无法打开输入文件");
  }

  std::ostringstream oss;
  oss << in.rdbuf();

  // 初始化错误报告器
  reporter = std::make_unique<err::ErrReporter>(oss.str()); // 保留原始文本信息

  // 删除注释
  std::istringstream iss{preproc::removeAnnotations(oss.str())};

  std::string line{};
  std::vector<std::string> text{};
  while (std::getline(iss, line)) {
    text.push_back(std::move(line)); // 逐行读取待分析代码
  }

  if (!iss.eof()) {
    util::runtime_error("编译器初始化时，文件读取错误");
  }

  // 初始化各组件
  this->lexer   = std::make_unique<lex::Lexer>(text, *reporter);
  this->symtab  = std::make_unique<sym::SymbolTable>();
  this->builder = std::make_unique<par::SemanticIRBuilder>(*symtab, *reporter);
  this->parser  = std::make_unique<par::Parser>(*lexer, *builder, *reporter);
}

/**
 * @brief 生成中间代码
 * @param file 输出文件名（不带后缀）
 */
void
Compiler::generateIR(const std::string &file)
{
  // 准备输出文件流
  std::string base = file.empty() ? "output" : file;
  std::ofstream out;
  out.open(base + std::string{".ir"});
  if (!out) {
    util::runtime_error("无法打开输出文件（.ir）");
  }

  // 一遍扫描、语法制导地生成中间代码
  ast_root = parser->parseProgram();

  // pretty print
  for (const auto &code : ast_root->ircode) {
    out << code->str() << std::endl;
  }
}

} // namespace cpr

