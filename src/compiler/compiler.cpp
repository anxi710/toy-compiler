#include <print>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "panic.hpp"
#include "preproc.hpp"
#include "ir_quad.hpp"
#include "compiler.hpp"
#include "code_generate.hpp"

namespace cpr {

Compiler::Compiler(const std::string &file)
{
  // 读取输入文件
  std::ifstream in;
  in.open(file);
  if (!in) {
    UNREACHABLE("无法打开输入文件");
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
    UNREACHABLE("编译器初始化时，文件读取错误");
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
Compiler::generateIR(const std::string &file, bool print)
{
  // 准备输出文件流
  std::string base = file.empty() ? "output" : file;
  std::string filename = std::format("{}.ir", base);
  std::ofstream out;
  out.open(filename);
  if (!out) {
    UNREACHABLE("无法打开输出文件（.ir）");
  }

  // 一遍扫描、语法制导地生成中间代码
  ast_root = parser->parseProgram();

#ifdef DEBUG
  std::ofstream out_sym;
  out_sym.open("symbol.txt");
  if (!out_sym) {
    UNREACHABLE("无法打开输出文件（symbol.txt）");
  }
  symtab->dump(out_sym);
#endif

  // 如果扫描过程中发现了错误，则打印错误并退出
  if (reporter->hasErrs()) {
    reporter->displayErrs();
    std::filesystem::remove(filename);
    return;
  }

  if (print) {
    // pretty print
    for (const auto &code : ast_root->ircode) {
      std::string idxstr =
        (code->op == ir::IROp::LABEL || code ->op == ir::IROp::FUNC)
        ? "" : "  ";
      std::println(out, "{}{}", idxstr, code->str());
    }
  } else {
    std::filesystem::remove(filename);
  }
}

void
Compiler::generateAssemble(const std::string &file)
{
  // 准备输出文件流
  std::string base = file.empty() ? "output" : file;
  std::string filename = std::format("{}.s", base);
  std::ofstream out;
  out.open(filename);
  if (!out) {
    UNREACHABLE("无法打开输出文件（.s）");
  }

  if (ast_root == nullptr) {
    generateIR(file, false);
  }

  cg::CodeGenerator codegen{out, *symtab};
  codegen.generate(*ast_root);
}

} // namespace cpr
