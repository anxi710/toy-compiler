# Toy Compiler

[![Continuous Integration](https://github.com/anxi710/toy_compiler/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/anxi710/toy_compiler/actions/workflows/c-cpp.yml) [![C++ Version](https://img.shields.io/badge/C%2B%2B-26-blue.svg)](https://en.cppreference.com/w/cpp/26.html) ![clang++ 20.1.8](https://img.shields.io/badge/clang++-20.1.8-brightgreen?logo=llvm)

> This is a rust-like compiler implemented in c++.
>
> 同济大学 2024-2025 学年暑期 **编译原理课程设计** 作业

项目部分基于该学年第二学期的 **编译原理** 课程作业，点击[这里](https://github.com/anxi710/compiler-principle-project)查看更多信息。

## 项目简介

本项目使用现代 `C++` 实现了一个从类 `Rust` 语言到 `RV32IM` 汇编的编译器。按照编译的执行逻辑，将整个编译器分为了 *preprocess*, *lexer*, *parser*, *semantic check*, *IR builder* 和 *code generate* 六个模块。除此之外，为支持各模块内部的功能实现，还设计了 *AST*, *symbol table*, *type system*, *compiler driver* 和 *error reporter* 五个模块。

该编译器能够根据输入的类 `Rust` 语言源程序，选择性输出中间代码（四元式）表示的程序以及目标代码（`RV32IM` 汇编）的程序。并且最终生成的汇编程序可以使用 `riscv64-linux-gnu-gcc` 编译成可在 `QEMU` 模拟的 `RISC-V64` 机器上运行的可执行程序，这意味着本项目实现的编译器编译的汇编程序符合简单的 `RISC-V Linux ABI` 规定。

项目实现的编译器前端可以完成对使用除 6.2 借用和引用之外的所有产生式编写的类 `Rust` 程序的语义检查和中间代码生成工作；但后端只实现了除 8.1 ~ 9.2 之外的产生式的汇编代码生成（`docs/【Rust版】课程设计.pdf` 中定义了产生式）。

---

为何称之为现代：

```cpp
template <typename... Vecs>
  requires (std::same_as<std::decay_t<Vecs>, std::vector<IRQuadPtr>> && ...)
auto concatIrcode(Vecs&&... vecs) -> std::vector<IRQuadPtr> {
  return std::views::concat(std::forward<Vecs>(vecs)...)
    | std::ranges::to<std::vector>();
}

template <typename T>
concept HasIRCode = requires(const T &node) {
  { node->ircode } -> std::convertible_to<std::vector<IRQuadPtr>>;
};

template <std::ranges::range ASTNodeRange>
  requires HasIRCode<std::ranges::range_value_t<ASTNodeRange>>
auto extractIrcodeAndConcat(const ASTNodeRange &nodes) -> std::vector<IRQuadPtr> {
  return nodes
    | std::views::transform([](const auto &node) {
        return node->ircode;
      })                              // transform
    | std::views::join                // flatten
    | std::ranges::to<std::vector>(); // collect
}
```

本项目使用了变长参数模板、移动语义、完美转发、concept、if constexpr 和函数式编程范式等大量现代 `C++` 引入的新特性。旨在尝试利用这些新特性编写更加高效、简洁与优雅的代码。

---

为何说生成的代码符合基本的 `RISC-V Linux ABI` 规范：

```c
extern int main0();

int
main()
{
  int ret_val = main0();
  printf("return value = %d\n", ret_val);

  return ret_val;
}
```

生成的 `RV32IM` 汇编代码可以使用 `riscv64-linux-gnu-gcc` 与上述 `C` 代码编译链接为一个可执行程序，并且在 `QEMU` 模拟的 `RISC-V 64` 机器上正常运行。这说明生成的汇编代码遵守了基本的栈对齐、寄存器保存、返回值放置等函数调用约定。

## 文件组织说明

```shell
.
├── docs
│   └── 【Rust版】课程设计.pdf
├── Makefile     # 构建文件
├── note         # 部分设计笔记
├── README.md
├── reference    # 参考资料
│   ├── compiler # 编译器设计相关
│   ├── llvm     # LLVM IR
│   ├── riscv    # RISC-V 汇编
│   └── x86_64   # x86_64 汇编
├── report       # 设计报告
├── riscv-asm
│   ├── main.c   # 调用生成的汇编中的函数，并打印返回值
│   └── Makefile # ELF 构建和 QEMU 启动脚本
├── src
│   ├── ast      # AST 结点和 visitor 声明
│   ├── codegen  # 代码生成
│   ├── compiler # 编译器驱动
│   ├── error    # 错误报告器
│   ├── generate_accept.pl # 自动生成 AST 结点的 accept 函数
│   ├── ir       # 中间代码生成
│   ├── lexer    # 词法分析器
│   ├── main.cpp # 程序入口
│   ├── parser   # 语法分析器
│   ├── preproc  # 预处理器
│   ├── semantic # 语义检查器
│   ├── symtab   # 符号表
│   ├── type     # 类型系统
│   └── util     # 工具函数
└── test         # 测试文件
```

## 补充

本项目中的所有书籍资料仅供自学使用，设计文档中的所有思考仅代表我当时的看法，代码实现中的所有策略和技巧仅代表本人当时的编码水平，本人保留自省的权利。
