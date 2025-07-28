<head>
    <meta charset="UTF-8">
    <title>编译原理课程设计-设计说明书</title>
    <link rel="stylesheet" href="styles.css">
</head>

# 编译原理课程设计 —— 设计说明书

## 1 总述

### 1.1 项目介绍

本项目实现了一个由类 Rust 语言源代码到 RV32IM 指令集架构汇编代码的完整编译器，采用现代 C++ 编写，并遵循模块化设计原则。整个编译器按照编译流程划分为以下六个主要模块：

- **preprocess**（预处理器）
- **lexer**（词法分析器）
- **parser**（语法分析器）
- **semantic check**（语义检查器）
- **IR builder**（中间代码生成器）
- **code generate**（汇编生成器）

此外，为支撑各阶段功能实现，项目还设计了以下通用基础模块：

- **AST**（抽象语法树结构与访问器）
- **symbol table**（符号表）
- **type system**（类型系统）
- **compiler driver**（编译器驱动）
- **error reporter**（统一错误处理机制）

编译器支持将类 Rust 源程序编译为：

- 中间代码（四元式形式），便于调试和后端优化；
- 目标代码（RV32IM 汇编），可通过 riscv64-linux-gnu-gcc 与运行时文件链接成 ELF 可执行文件，并在 QEMU 模拟的 RISC-V64 环境中运行。

这表明本编译器生成的目标代码符合 RISC-V Linux ABI 的基本规范，并具备一定的移植与实际部署能力。

当前编译器前端支持除 6.2 中的“借用与引用”产生式外的所有语法结构，后端则实现了除第 8.1 至 9.2 所列产生式以外的大部分功能，基本覆盖了常用的函数定义、流程控制、基本数据结构与表达式语义。

### 1.2 文件组织架构说明

项目文件结构如下所示，展示了各个模块的分布：

```shell
.
├── Makefile                 # 编译配置文件，定义构建规则
├── riscv-asm                # RV32 汇编运行环境与测试接口
│   ├── main.c               # 调用生成汇编中的入口函数并打印返回值
│   └── Makefile             # 构建 ELF 可执行文件并运行 QEMU
├── src
│   ├── ast                  # AST 节点定义及 Visitor 模板
│   ├── codegen              # 汇编代码生成器
│   ├── compiler             # 编译器入口与编译流程调度
│   ├── error                # 错误报告与处理机制
│   ├── generate_accept.pl   # AST 节点 accept 函数的自动生成脚本
│   ├── ir                   # 四元式 IR 中间表示生成逻辑
│   ├── lexer                # 词法分析器实现
│   ├── main.cpp             # 编译器入口主程序
│   ├── parser               # 递归下降语法分析器
│   ├── preproc              # 预处理逻辑
│   ├── semantic             # 语义检查器及作用域分析
│   ├── symtab               # 符号表管理
│   ├── type                 # 类型系统推导与检查
│   └── util                 # 通用工具函数与辅助类
└── test                     # 测试用例集合与验证输入
```

### 1.3 持续集成与开发辅助工具

为保障项目的开发效率、代码质量和协作可维护性，编译器实现中集成了一套完整的自动化工具链与持续集成（CI）流程，包括但不限于以下内容：

1. 自动构建与测试

   - 使用 GitHub Actions 配置自动化工作流，基于 ubuntu-latest 环境；

   - 每次提交（push）或拉取请求（PR）将自动触发构建与测试任务；

   - 保障主分支始终处于可编译、可测试的稳定状态。

2. 代码质量保障机制

   - 静态分析工具：采用 clang-tidy 对源代码进行静态检查，及时发现潜在的未定义行为、内存使用错误及风格不一致问题；

   - 统一格式规范：借助 clang-format 与 .editorconfig 实现跨编辑器的格式规范统一，并在本地和 CI 中均支持自动格式化与校验；

   - 模块化与类型安全设计：核心模块均以接口驱动设计，最大程度减少耦合，并通过现代 C++ 类型系统增强可维护性。

3. 本地 Git 提交流程增强

   - 提供 pre-commit 钩子脚本，用于本地提交前自动执行：

     - 格式检查（clang-format）

     - 静态分析（clang-tidy）

     - 快速编译测试等任务

4. 提交信息规范化

   - 采用 [Conventional Commit](https://www.conventionalcommits.org/en/v1.0.0/) 规范，统一 Git 提交记录格式；

   - 有助于版本号自动化管理（Semantic Versioning）与变更日志生成（如通过 semantic-release 工具链）。

<br>

## 2 总体设计

### 2.1 系统架构

本项目采用模块化设计，按功能划分为以下子系统，各自独立又协同运行：

- **Preprocess**: 实现源码预处理，目前主要功能是删除注释；为词法分析提供清洗后的输入。

- **Lexer**（词法分析器）
  - 封装为 `nextToken` 接口供语法分析调用；
  - 内部维护 **Keyword Table**，识别语言关键字；
  - 将识别到的词法单元封装为 `Token`，包含：
    - 类型；
    - 原始字面量；
    - 在源码中的位置信息（行列号）。

- **Parser**（语法分析器）
  - 实现自顶向下、一遍扫描的递归下降分析器；
  - 支持边构建边分析，即构建 AST 结点的同时完成语义检查与 IR 生成；
  - 设计亮点：
    - **AST** 结构采用数据与行为分离原则；
    - 使用 CRTP 模式 和传统 双分派模式 支持灵活的 AST 访问；
    - 嵌入式 **Semantic IR Builder** 实现边分析边生成中间代码的语法制导翻译。

- **Semantic Checker**（语义检查器）
  - 基于 visitor 模式，对 AST 实现结构化语义验证；
  - 设计为解耦子组件，独立实现部分语义规则：
    - **ReturnChecker**: 验证函数路径是否完整返回；
    - **BreakChecker**: 验证 break 是否合法；
    - **SemanticContext**: 用于语义信息传递，如作用域链、函数上下文、变量类型等。

- **IR Builder**（中间代码生成器）
  - 使用 visitor 对 AST 结点生成四元式 IR；
  - IR 结构：
    - `IROp`: 操作类型（如 add, call, goto 等）；
    - `arg1`, `arg2`, `dst`: 操作数与目标变量；
    - `label`: 跳转目标；
    - `elems`: 支持数组/元组元素构造；
  - 提供统一的 **QuadFactory** 和 **TempFactory**，规范化中间代码与临时变量创建。

- **Code Generate**（目标代码生成器）
  - 面向 RISC-V RV32IM 指令集生成汇编代码；
  - 核心组件：
    1. **Register Allocator**: 使用轮转策略，支持 spill；
    2. **Stack Allocator**: 遵循 RISC-V ABI，负责对齐、分配、溢出处理；
    3. **Memory Allocator**: 统一调度寄存器与栈资源；
    4. **Register** 枚举清晰区分 Caller/Callee Saved。

- **Type System**（类型系统）
  - 定义静态类型体系：
    - 基础类型：Unit, Bool, I32, Array, Tuple；
    - 辅助类型：Unknown, Any；
  - 使用 **TypeFactory** 保证类型实例唯一性，简化类型比较与匹配。

- **Symbol Table**（符号表）
  - 支持嵌套作用域与作用域隔离；
  - 支持四类符号实体：
    - `Temp`（临时变量）；
    - `Variable`（局部变量）；
    - `Constant`（常量）；
    - `Function`（函数）；
  - 提供常量表支持与查询机制。

- **Compiler Driver**（编译器驱动器）
  - 组织各模块初始化与调用，作为顶层控制器；
  - 提供外部接口：`generateIR()` 与 `generateAssemble()`；
  - 避免使用全局变量，模块之间通过显式所有权传递。

- **Error Reporter**（错误报告器）
  - 模仿 rustc 实现带颜色与位置信息的终端美化报错；
  - 区分错误类型：词法错误、语法错误、语义错误；
  - 支持多错误收集与最终统一输出。

- **Util**（工具库）
  - `Position`: 表示源代码中的位置信息；
  - 自定义断言宏，提供带上下文的调试输出。

- **main.cpp**: 实现命令行参数解析；驱动整个编译流程，作为程序入口。

### 2.2 工作流程

1. 命令行入口
   - `main()` 接收命令行参数，确认输入输出路径；
   - 实例化 Compiler Driver。

2. 预处理阶段
   - 调用 Preprocess 删除注释，准备干净的源代码。

3. 语法分析阶段
   - 使用 Parser 构造 AST；
   - 在构造每个结点的同时：
     - 触发语义检查；
     - 同步生成中间代码（四元式）。

4. 中间代码阶段
   - 收集 IR 结果；
   - 提供给 Code Generator 用于后续汇编生成。

5. 汇编生成阶段
   - 调用 Code Generator 输出符合 RISC-V 规范的汇编代码；
   - 管理寄存器与栈空间，确保代码合法且高效。

6. 错误处理
   - 编译过程中如遇错误，调用 Error Reporter 报告；
   - 所有错误在流程结束后统一输出。

<br>

## 3 词法分析详细设计

本节介绍词法分析模块的核心组成与实现方式，涵盖 `Token` 类型设计、关键词识别机制、正则匹配策略以及有限状态自动机的状态转移逻辑。

### 3.1 Token 设计

#### 3.1.1 Token 数据结构

`Token` 是词法分析器的基本输出单元，封装了三类信息：

- `type`：词法单元类型（枚举 TokenType）；
- `value`：原始文本内容；
- `pos`：在源程序中的位置信息（行、列）；

其结构如下：

```cpp
struct Token {
  TokenType      type;  // token type
  std::string    value; // 组成 token 的字符串
  util::Position pos;   // position
};
```

#### 3.1.2 Token 类型定义

`TokenType` 使用枚举类定义，涵盖所有关键字、标识符、常量、运算符与分隔符。为便于维护与扩展，采用 X-Macro 技巧自动展开枚举和字符串映射：

```cpp
#define TOKEN_LIST(_) \
  _(END) _(IF) _(ELSE) _(WHILE) _(ID) _(I32) _(BOOL) _(LET) \
  _(RETURN) _(MUT) _(FN) _(FOR) _(IN) _(LOOP) _(BREAK) \
  _(CONTINUE) _(TRUE) _(FALSE) _(INT) _(LPAREN) _(RPAREN) \
  _(LBRACE) _(RBRACE) _(LBRACK) _(RBRACK) _(SEMICOLON) \
  _(COLON) _(COMMA) _(PLUS) _(ASSIGN) _(MINUS) _(MUL) \
  _(DIV) _(GT) _(LT) _(DOT) _(EQ) _(NEQ) _(GEQ) _(LEQ) \
  _(DOTS) _(ARROW)

enum class TokenType : std::uint8_t {
#define X(name) name,
  TOKEN_LIST(X)
#undef X
  COUNT
};

std::string tokenType2str(TokenType type) {
  switch (type) {
#define X(name) case TokenType::name: return #name;
  TOKEN_LIST(X)
#undef X
    default: UNREACHABLE("token type undefined");
  }
}
```

### 3.2 Lexer 实现

词法分析器按自左向右逐字符扫描方式处理源程序，分阶段执行空白跳过、模式匹配、Token 生成等操作。

#### 3.2.1 关键字表实现

采用哈希表（`std::unordered_map`）存储所有语言关键字，支持高效查询与类型映射：

```cpp
class KeywordTable {
public:
  bool iskeyword(const std::string &v) const;
  void addKeyword(std::string name, TokenType type);
  TokenType getKeyword(const std::string &v) const;
private:
  std::unordered_map<std::string, TokenType> keywords;
};
```

Lexer 构造时自动初始化关键字表：

```cpp
this->keytab.addKeyword("if",       TokenType::IF);
this->keytab.addKeyword("fn",       TokenType::FN);
// ... 其他关键字
```

#### 3.3 有限自动机实现

词法分析器整体逻辑可抽象为一台确定性有限自动机（DFA），主要分三个阶段：

Step1: 跳过空白字符

空白字符（如空格、换行、Tab）在语义上无效，直接跳过：

```cpp
while (pos.row < text.size()) {
  if (text[pos.row].empty()) { // 忽略空行
    ++pos.row;
    pos.col = 0;
  } else if ( // 忽略空白字符
    static_cast<bool>(std::isspace(text[pos.row][pos.col]))
  ) {
    shiftPos(1);
  } else {
    break;
  } // end if
} // end while
```

step2: 正则匹配标识符与整数字面量

使用 `std::regex` 对 *ID* 和 *INT* 进行模式匹配：

```cpp
static const std::vector<std::pair<TokenType, std::regex>> patterns {
  {TokenType::ID,  std::regex{R"(^[a-zA-Z_]\w*)"}},
  {TokenType::INT, std::regex{R"(^\d+)"}}
};
```

匹配成功后：

- 如果为 *ID*，需进一步查询是否为关键字；
- 更新位置信息并返回对应 `Token`。

step 3: 状态机识别运算符与分隔符

使用 switch-case 实现的有限状态转移逻辑，支持一字符与双字符符号识别：

```cpp
switch (fchar) {
  case '=':
    token = (schar == '=') ?
      Token{TokenType::EQ, "=="}
    : Token{TokenType::ASSIGN, "="};
    break;
  case '-':
    token = (schar == '>') ?
      Token{TokenType::ARROW, "->"}
    : Token{TokenType::MINUS, "-"};
    break;
  case '!':
    if (schar == '=')
      token = {TokenType::NEQ, "!="};
    break;
  // ...其余符号略
}
```

**附图**：词法分析自动机

<img src="img/lexer_DFA_1.png" alt="DFA part1" height=500/>

<img src="img/lexer_DFA_2.png" alt="DFA part2" height=500/>

<br>

## 4 语法分析详细设计

### 4.1 文法设计

#### 4.1.1 类 Rust 语言的文法定义

我们采用兼容 *LL(2)* 的 *上下文无关文法（Context-Free Grammar, CFG）* 对类 Rust 语言的语法结构进行形式化建模。本设计目标是在保持语义模型与 Rust 接近的基础上，实现可预测、易解析、结构一致的语法体系，适用于递归下降分析器的高效实现。

为增强表达能力与可读性，文法采用 EBNF（扩展巴科斯-瑙尔范式） 进行书写。以下为语法规则定义：

```bash
InnerVarDecl    -> ("mut")? "<ID>"

Type            -> "()" | "i32" | "bool" | ArrayType | TupleType
ArrayType       -> "[" Type ";" "<NUM>" "]"
TupleType       -> "(" Type "," TypeList ")"
TypeList        -> Type ("," Type)* | ε

Prog            -> (FuncDecl)*

FuncDecl        -> FuncHeaderDecl StmtBlockExpr
FuncHeaderDecl  -> "fn" "<ID>" "(" Args ")" ("->" Type)?
Args            -> ε | Arg ("," Arg)*
Arg             -> InnerVarDecl ":" Type

StmtBlockExpr   -> "{" (Stmt)* "}"

Stmt            -> EmptyStmt | VarDeclStmt | ExprStmt
EmptyStmt       -> ";"
VarDeclStmt     -> "let" InnerVarDecl (":" Type)? ("=" Expr)? ";"
ExprStmt        -> Expr (";")?

Expr            -> RetExpr
                 | BreakExpr
                 | ContinueExpr
                 | IfExpr
                 | LoopExpr
                 | WhileLoopExpr
                 | ForLoopExpr
                 | StmtBlockExpr
                 | CallExpr
                 | AssignExpr
                 | CmpExpr

RetExpr         -> "return" (Expr)?
BreakExpr       -> "break" (Expr)?
ContinueExpr    -> "continue"

AssignExpr      -> AssignElem "=" Expr
AssignElem      -> Variable | ArrAcc | TupAcc
ArrAcc          -> Value "[" Expr "]"
                 | ArrAcc "[" Expr "]"
                 | TupAcc "[" Expr "]"
TupAcc          -> Value "." "<NUM>"
                 | TupAcc "." "<NUM>"
                 | ArrAcc "." "<NUM>"

Value           -> BracketExpr | CallExpr | Variable
BracketExpr     -> "(" Expr ")"
CallExpr        -> "<ID>" "(" ArgList ")"
ArgList         -> Expr ("," Expr)* | ε
Variable        -> "<ID>"

CmpExpr         -> (CmpExpr ["<" | "<=" | ">" | ">=" | "==" | "!="])* AddExpr
AddExpr         -> (AddExpr ["+" | "-"])* MulExpr
MulExpr         -> (MulExpr ["*" | "/"])* Factor

Factor          -> ArrElems | TupElems | Element
ArrElems        -> "[" (Expr)? ("," Expr)* "]"
TupElems        -> "(" (Expr "," TupElem)? ")"
TupElem         -> Expr ("," Expr)* | ε

Element         -> "<NUM>" | Value | AssignElem

IfExpr          -> "if" Expr StmtBlockExpr ElseClause
ElseClause      -> "else" "if" Expr StmtBlockExpr ElseClause
                 | "else" StmtBlockExpr
                 | ε

LoopExpr        -> WhileLoopExpr
                 | ForLoopExpr
                 | "loop" StmtBlockExpr

WhileLoopExpr   -> "while" Expr StmtBlockExpr
ForLoopExpr     -> "for" InnerVarDecl "in" Iterable StmtBlockExpr
Iterable        -> RangeExpr | IterableVal
RangeExpr       -> Expr ".." Expr
IterableVal     -> Expr
```

**语法统一性设计动机**：

与原始文法相比，当前版本进行了结构性的重构，核心思想源于 `Rust` 的语义特征 —— *Everything is an expression*。这一理念不仅使语言更具函数式编程风格，也显著提升了解析器设计的统一性与抽象能力。

具体来说，除了变量声明以外，所有语句都被统一建模为表达式（Expr）。这样的设计带来以下优势：

1. 表达式语义统一：如 return、loop、if、for 等结构都可以嵌套出现在赋值或调用表达式中；

2. 解析器构建友好：配合 LL(2) 分析技术，在无需回溯的前提下实现递归下降解析；

3. 可支持嵌套表达式块：例如 loop { break a } 和 return return 等嵌套语法都可以自然地被支持。

**示例说明**：

以下示例代码展示了表达式语义统一的强大能力：

```rust
fn main() {
  let a = [1, 2, 3];

  for i in loop { break a; } {
    println!("{}", i);
  }

  return return
}
```

在这段代码中：

- `loop { break a; }` 是一个合法的表达式，其结果值为 a；

- `return return` 也是合法的，其中内层 return 作为表达式返回值被外层返回；

- 这些嵌套结构在当前文法中都能自然解析，无需特殊处理。

通过将语法建模建立在表达式之上，我们不仅更贴近 Rust 本身的设计哲学，也在构建解析器时获得了结构上的简洁性和一致性。

#### 4.1.2 左递归问题

在本节文法中存在以下左递归形式：

```bash
ArrAcc  -> ArrAcc "[" Expr "]"
TupAcc  -> TupAcc "." "<NUM>"
CmpExpr -> (CmpExpr ["<" | "<=" | ">" | ">=" | "==" | "!="])* AddExpr
AddExpr -> (AddExpr ["+" | "-"])* MulExpr
MulExpr -> (MulExpr ["*" | "/"])* Factor
```

**左递归转换方式**：

我们未使用标准消除左递归的方式，而是通过 **语法特征前缀引导 + 显式循环展开** 替代递归。例如：

```cpp
while (check(TokenType::LBRACK) || check(TokenType::DOT)) {
  if (check(TokenType::LBRACK)) {
    assign_elem = parseArrAcc(assign_elem);
  } else if (check(TokenType::DOT)) {
    assign_elem = parseTupAcc(assign_elem);
  }
}
```

原因：`[ ]` 和 `.` 为明显的解析引导符号，易于构造循环处理逻辑，避免了无限递归风险。

同理，表达式中的递归也通过循环形式解析，如下所示：

```cpp
ast::ExprPtr
Parser::parseAddExpr(std::optional<ast::ExprPtr> expr)
{
  ast::ExprPtr lhs = Parser::parseMulExpr(std::move(expr));
  while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
    TokenType op = cur.type;
    advance();
    ast::ExprPtr rhs = parseMulExpr();
    lhs = std::make_shared<ast::AriExpr>(
      lhs, tokenType2AriOper(op), rhs
    );
  }
  return lhs;
}
```

#### 4.1.3 使用 vector 替代右递归展开

例如参数列表规则：

```bash
Args -> (Arg ("," Arg)*)?
```

直接使用 `std::vector` 顺序收集参数，无需构造链式节点或右递归结构：

```cpp
std::vector<ast::ArgPtr> argv{};
while (!check(TokenType::RPAREN)) {
  argv.push_back(parseArg());
  if (check(TokenType::RPAREN)) break;
  consume(TokenType::COMMA, "Expect ','");
}
advance();
```

这种方式在处理实参、元组元素、数组初始化等多处均可复用，统一结构同时提高效率。

#### 4.1.3 小结

本文设计的类 Rust 文法具备以下特点：

1. 表达式统一建模：简化语义，提升语言灵活性；

2. LL(2) 兼容性强：依赖有限前瞻，避免回溯与冲突；

3. 左递归转循环结构清晰：配合语法前缀轻松转换；

4. 右递归转迭代结构：通过 vector 收集方式统一管理；

整体文法清晰、语义一致、实现简单，具有良好的可扩展性和工程可落地性。


### 4.2 Parser 实现

#### 4.2.1 Token 流管理机制

在递归下降语法分析中，token 流的管理是解析器稳定性与前瞻能力的基础。本项目采用一种轻量而高效的 **双缓冲（三明治）** 模型，结合 LL(2) 策略，实现了可预测、可容错的 token 流调度机制。

解析器类定义如下所示：

```cpp
class Parser {
private:
  auto nextToken() -> lex::Token;
  void advance();
  bool match(lex::TokenType type);
  bool check(lex::TokenType type) const;
  bool checkAhead(lex::TokenType type);
  void consume(lex::TokenType type, const std::string &msg);

private:
  lex::Token                cur; // current token
  std::optional<lex::Token> la;  // look ahead token
};
```

**三明治模型简介**：

该模型使用两个指针 cur 和 la，分别表示：

1. `cur`：当前解析器正在处理的 token；
2. `la`：向前看一个 token 的缓存，用于实现 LL(2) 分析所需的 lookahead 能力。

如下图所示，解析器总是位于 token 流的当前与未来之间，前后夹持，形成 “sandwich” 式结构：

```lua
← token stream →
 --------------------------
 | ... | cur | lookahead |
         ↑     ↑
    当前指针    向前看缓存
```

**核心接口实现**：

以下为 token 控制核心函数的定义及其行为说明：

```cpp
/**
 * @brief  获取下一个 token
 * @return 返回类型为 Token；失败时调用终止接口
 */
Token Parser::nextToken() {
  if (auto token = lexer.nextToken(); token.has_value()) {
    return token.value();
  }
  // 识别到未知 token，立即终止分析流程
  err::terminate(reporter);
}

/**
 * @brief 推进 token 流
 * 若 la 中已有值则直接使用，否则从词法分析器中拉取
 */
void Parser::advance() {
  if (la.has_value()) {
    cur = la.value();
    la.reset();
  } else {
    cur = nextToken();
  }
}

/**
 * @brief 匹配并消耗指定类型的 token（如成功则调用 advance）
 * @return 是否匹配成功
 */
bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

/**
 * @brief 检查当前 token 是否为指定类型（不消耗）
 */
bool Parser::check(TokenType type) const {
  return cur.type == type;
}

/**
 * @brief  向前检查一个 token 是否为指定类型
 */
bool Parser::checkAhead(TokenType type) {
  if (!la.has_value()) {
    la = nextToken();
  }
  return la.has_value() && la.value().type == type;
}

/**
 * @brief 消耗指定类型的 token，若类型不匹配则报告语法错误
 * @param msg 错误提示信息
 */
void Parser::consume(TokenType type, const std::string &msg) {
  if (!match(type)) {
    reporter.report(
      err::ParErrType::UNEXPECT_TOKEN,
      msg,
      cur.pos,
      cur.value
    );
  }
}
```

**错误报告与容错机制**：

- 所有语法错误均通过 reporter 接口记录，不立即中断主流程；

- `consume` 是非致命操作，仅报告错误，允许继续尝试分析后续语法结构；

- 致命错误（如 nextToken 获取失败）则调用 `terminate()`，立即停止解析过程，保证错误不会传播至中间表示构建阶段。

该机制为后续实现语法级错误恢复策略（如同步集、panic mode）提供了良好基础。

**小结**：

本节所实现的 token 流模型具备以下特点：

- 高效的 LL(2) 支持：利用预读缓存避免了回溯需求；

- 结构简单，接口清晰：五个基本操作涵盖所有常见场景；

- 良好的错误处理接口：既能中断致命错误，也支持容忍非致命偏差；

- 可拓展性强：适用于后续所有表达式、语句、类型等递归下降函数的构建。

下一小节将基于该 token 管理接口，展开表达式语法的递归下降实现。

#### 4.2.2 LL(2) 解析器设计

本节介绍基于 LL(2) 策略的递归下降语法分析器实现，包括语法接口组织、分派调度机制，以及表达式处理的具体策略。为适配前文文法设计中统一表达式结构的特点，解析器采用具有 lookahead 能力的派发结构，避免了回溯，提升了解析效率与稳定性。

##### 4.2.2.1 递归下降解析

语法分析器中，每个非终结符都有对应的解析函数，返回其对应的 AST 结点指针。接口示意如下：

```cpp
auto parseProgram() -> ast::ProgPtr;
auto parseID() -> std::pair<std::string, util::Position>;
auto parseInnerVarDecl() -> std::tuple<bool, std::string, util::Position>;
auto parseFuncDecl() -> ast::FuncDeclPtr;
auto parseFuncHeaderDecl() -> ast::FuncHeaderDeclPtr;
auto parseArg() -> ast::ArgPtr;
auto parseType() -> ast::Type;
auto parseStmtBlockExpr() -> ast::StmtBlockExprPtr;
auto parseStmt() -> ast::StmtPtr;
auto parseVarDeclStmt() -> ast::VarDeclStmtPtr;
auto parseExprStmt() -> ast::ExprStmtPtr;

auto parseExpr() -> ast::ExprPtr;
auto parseRetExpr() -> ast::RetExprPtr;
auto parseBreakExpr() -> ast::BreakExprPtr;
auto parseContinueExpr() -> ast::ContinueExprPtr;
auto parseAssignExpr(ast::AssignElemPtr&&) -> ast::AssignExprPtr;
auto parseAssignElem(std::optional<ast::ExprPtr>) -> ast::AssignElemPtr;
auto parseArrAcc(ast::ExprPtr) -> ast::ArrAccPtr;
auto parseTupAcc(ast::ExprPtr) -> ast::TupAccPtr;
auto parseValue() -> ast::ExprPtr;

auto parseCmpExpr(std::optional<ast::ExprPtr>) -> ast::ExprPtr;
auto parseAddExpr(std::optional<ast::ExprPtr>) -> ast::ExprPtr;
auto parseMulExpr(std::optional<ast::ExprPtr>) -> ast::ExprPtr;
auto parseFactor(std::optional<ast::ExprPtr>) -> ast::ExprPtr;
auto parseArrElems() -> ast::ExprPtr;
auto parseTupElems() -> ast::ExprPtr;
auto parseElement(std::optional<ast::ExprPtr>) -> ast::ExprPtr;

auto parseCallExpr() -> ast::CallExprPtr;
auto parseIfExpr() -> ast::IfExprPtr;
auto parseElseClause() -> ast::ElseClausePtr;
auto parseForLoopExpr() -> ast::ForLoopExprPtr;
auto parseIterable() -> ast::ExprPtr;
auto parseWhileLoopExpr() -> ast::WhileLoopExprPtr;
auto parseLoopExpr() -> ast::LoopExprPtr;
```

这些函数共同构成了解析器的自顶向下派发网络，入口为 parseProgram()，向下递归解析整个源码文件。

##### 4.2.2.2 解析函数示例

以下通过两个代表性函数展示解析器的 LL(2) 实现风格与结构设计：

##### 示例一：函数头解析

```cpp
ast::FuncHeaderDeclPtr Parser::parseFuncHeaderDecl() {
  // FuncHeaderDecl -> fn <ID> ( (arg)? (, arg)* ) (-> Type)?
  consume(TokenType::FN, "Expect 'fn'");

  auto [funcname, declpos] = parseID();

  builder.ctx->enterFunc(funcname, declpos);

  consume(TokenType::LPAREN, "Expect '('");

  // (arg)? (, arg)*
  std::vector<ast::ArgPtr> argv{};
  while (!check(TokenType::RPAREN)) {
    argv.push_back(parseArg());
    if (check(TokenType::RPAREN)) break;
    consume(TokenType::COMMA, "Expect ','");
  }
  advance(); // consume )

  // (-> Type)?
  ast::Type rettype{type::TypeFactory::UNIT_TYPE};
  if (check(TokenType::ARROW)) {
    advance();
    rettype = parseType();
  }

  auto funcheaderdecl =
    std::make_shared<ast::FuncHeaderDecl>(funcname, argv, rettype);
  funcheaderdecl->pos = declpos;
  builder.build(*funcheaderdecl);
  return funcheaderdecl;
}
```

该函数完整对应语法规则：

```bash
FuncHeaderDecl -> fn <ID> "(" (arg ("," arg)*)? ")" ("->" Type)?
```

设计要点：

- 使用 `consume` 做语法匹配 + 错误报告；
- 使用循环替代右递归，借助 `std::vector` 收集参数；
- 封装语义检查和中间代码生成为 `builder.build(...)`。

##### 示例二：表达式解析分派

```cpp
ast::ExprPtr Parser::parseExpr() {
  switch (cur.type) {
    case TokenType::RETURN:   return parseRetExpr();
    case TokenType::BREAK:    return parseBreakExpr();
    case TokenType::CONTINUE: return parseContinueExpr();
    case TokenType::IF:       return parseIfExpr();
    case TokenType::WHILE:    return parseWhileLoopExpr();
    case TokenType::FOR:      return parseForLoopExpr();
    case TokenType::LOOP:     return parseLoopExpr();
    case TokenType::LBRACE: {
      builder.ctx->enterBlockExpr();
      auto expr = parseStmtBlockExpr();
      builder.ctx->exitScope();
      return expr;
    }
    case TokenType::ID: {
      ast::ExprPtr expr;
      ast::AssignElemPtr assign_elem = nullptr;
      util::Position declpos;
      if (checkAhead(TokenType::LPAREN)) {
        expr = parseCallExpr();
      } else {
        declpos = cur.pos;
        expr = parseValue();
        if (check(TokenType::LBRACK) || check(TokenType::DOT)) {
          assign_elem = parseAssignElem(expr);
        }
      }

      if (check(TokenType::ASSIGN)) {
        if (assign_elem == nullptr) {
          assign_elem = std::make_shared<ast::AssignElem>(expr);
          assign_elem->kind = ast::AssignElem::Kind::VARIABLE;
          assign_elem->pos = declpos;
          builder.build(*assign_elem);
        }
        return parseAssignExpr(std::move(assign_elem));
      }

      return parseCmpExpr(
        assign_elem == nullptr ? expr : assign_elem
      );
    }
    case TokenType::INT:
    case TokenType::LPAREN:
    case TokenType::LBRACK:
      return parseCmpExpr();
    default:
      UNREACHABLE("parseExpr()");
  }
}
```

该函数作为 Expression 的主解析函数，采用 LL(2) 策略的多路分发模型：

- 基于当前 `cur.type` 快速分派；
- 使用 `checkAhead()` 实现二阶前瞻（判断 `CallExpr` vs `Variable`）；
- 支持将同一 token 序列根据上下文解析为不同结构（如 `x = ...` vs `x`）；
- 将所有非运算表达式优先识别，最后进入 `parseCmpExpr()` 处理二元运算链。

这一分发方式兼容 “everything is an expression” 模型，能有效解析如 `return loop { break 1 }` 等高度表达式化语法结构。

##### 小结

本节展示了基于 LL(2) 策略的递归下降解析器核心设计：

- 每个文法构件绑定唯一解析函数，形成自顶向下的语法调度结构；
- 表达式解析通过多路 switch + lookahead 实现高效分类分发；
- 解析器设计高度贴合文法规则，便于后续扩展与错误恢复机制集成。

#### 4.2.3 表达式优先级与左递归展开策略

表达式解析器是整个语言前端中结构最复杂、语义最丰富的部分。为了避免左递归造成无限递归调用，同时精确控制表达式的操作优先级和结合性，本项目采用如下策略：

1. 所有具有左递归结构的产生式均转换为迭代形式；

2. 按操作符优先级分层设计递归下降函数，各优先级层级对应一个专属的 `parseXXXExpr()` 函数；

3. 对结构歧义（如括号与元组）使用上下文条件判定。

4. 所有操作符解析函数采用 `while(...)` 循环持续匹配，模拟左结合结构。

本节具体说明每层表达式处理函数的结构、行为与设计动机。

##### 4.2.3.1 递归结构的平铺（左递归转换为迭代）

以下为简化后的语法结构：

```bash
CmpExpr -> AddExpr (CmpOper AddExpr)*
AddExpr -> MulExpr (("+"|"-") MulExpr)*
MulExpr -> Factor  (("*"|"/") Factor)*
```

上述表达式规则本质上是左递归链，例如：

```bash
a + b + c -> ((a + b) + c)
```

为避免递归爆栈与语义二义性，我们在每一层表达式解析函数中使用迭代累积方式展开左递归。

示例：`AddExpr` 的左递归转化为如下迭代结构：

```cpp
ast::ExprPtr lhs = parseMulExpr();
while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
  auto rhs = parseMulExpr();
  lhs = makeBinaryExpr(lhs, op, rhs); // 结果挂载回 lhs
}
return lhs;
```

这种写法严格保证了左结合的操作顺序。

##### 4.2.3.2 比较表达式解析

```cpp
ast::ExprPtr Parser::parseCmpExpr(std::optional<ast::ExprPtr> expr)
```

该函数用于处理 `<, <=, >, >=, ==, !=` 等比较运算。采用左结合解析，内部调用 `parseAddExpr()` 获取左右操作数。

例子：

```rust
a + b < c * d
```

构建顺序为：

```bash
CmpExpr
 ├── lhs: AddExpr -> (a + b)
 ├── op: <
 └── rhs: AddExpr -> MulExpr -> (c * d)
```

##### 4.2.3.3 算术表达式解析

`AddExpr` 和 `MulExpr` 结构类似，均为左结合链，且遵循：乘除（* /） > 加减（+ -） > 比较（< > == ...）。在 `parseAddExpr()` 和 `parseMulExpr()` 中采用下述方式解析：

```cpp
ast::ExprPtr lhs = parseMulExpr();
while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
  TokenType op = cur.type;
  advance();
  ast::ExprPtr rhs = parseMulExpr();
  lhs = std::make_shared<ast::AriExpr>(
    lhs, tokenType2AriOper(op), rhs
  );
}
```

每一轮迭代构建一个二元表达式节点，将其作为新的左操作数传入下一轮，形成左结合链。确保整个表达式树的结构符合运算优先级与结合律。

这种方式在构建 AST 时：无需回溯，能避免括号混淆，且可自然支持扩展（如支持 % 或 ** 操作时，仅需新增优先级层）。

##### 4.2.3.4 原子表达式与因子解析

最低层表达式由 `parseFactor()` 实现，结构如下：

```bash
Factor -> ArrElems | TupElems | Element
```

该函数负责识别以下结构：

1. 数组字面量：`[1, 2, 3]` -> `parseArrElems()`

2. 元组或括号表达式：`(x, y)` 或 `(x)` -> `parseTupElems()`

3. 基础单元：整数、变量、函数调用 -> `parseElement()`

其中 `parseTupElems()` 内部根据逗号存在与否判断是否构造元组节点或括号表达式节点，解决 `(x)` 与 `(x,)` 的语义分歧。

此外，为支持嵌套访问，如 `[1,2][0]` 或 `(a,b).0`，`parseFactor()` 会在元素解析后立即检查是否接有 `[` 或 `.` 并调用 `parseAssignElem()` 实现后续索引链。

##### 4.2.3.5 括号表达式与元组歧义消解

括号表达式与元组在语法上存在歧义。通过如下策略判断结构类型：

```cpp
if (!is_tuple_elem) {
  return std::make_shared<BracketExpr>(elems[0]);  // (x)
} else {
  return std::make_shared<TupElems>(elems);       // (x,)
}
```

- 当遇到 `()` 或无逗号 `,` 时，视为普通表达式；
- 当存在逗号，即便只有一个元素，也视为元组；
- 括号表达式被包装为 `BracketExpr` 节点，供后续优化或消解优先级使用。

##### 4.2.3.6 数组与元组字面量解析

数组与元组字面量采用统一策略处理元素列表：

```cpp
std::vector<ExprPtr> elems;
if (!check(RBRACK or RPAREN)) {
  elems.push_back(parseExpr());
}
while (!check(RBRACK or RPAREN)) {
  consume(COMMA, "Expect ','");
  elems.push_back(parseExpr());
}
```

该结构对尾逗号（`,`）进行容错处理，并在元素构造后立即判断是否存在索引/字段访问，确保对 `[1, 2][0]` 或 `(1, 2).1` 等结构的正确建树。

##### 4.2.3.7 Element 层次的语义合并

`parseElement()` 处理 `<INT>` 常量、`Variable`、`CallExpr` 以及索引链结构。其语义聚合规则如下：

| 类型 | 示例 | 节点类型 |
| - | - | - |
| 数字 | `123` | `Number` |
| 变量 | `x` | `Variable` |
| 函数调用 | `foo(x)` | `CallExpr` |
| 索引访问 | `a[1], a.0` | `AssignElem` |

如果当前 `Value` 后续跟有 `[` 或 `.`，则自动进入 `parseAssignElem()` 构建复合访问链，保证左值解析与表达式链解析一致。

##### 4.2.3.8 小结

本节展示了基于 LL(2) 的表达式优先级建模方法。设计特点如下：

- 使用分层解析函数明确表达式优先级；

- 通过迭代替代左递归，实现左结合表达式构造；

- 解析器可处理嵌套链式访问、混合括号结构与尾逗号容错；

- 括号与元组的歧义通过上下文判断进行语义分裂；

- 整个表达式系统结构统一、优先级明确、构造稳定。

该策略确保了解析器具备强表达能力的同时，维持 LL(2) 可解析性，无需回溯或上下文回传，便于语法树构造与后续静态分析。

#### 4.2.4 错误恢复与同步集机制

在实际源程序中，语法错误不可避免。为提高前端健壮性，本解析器实现了基于 **panic-mode** 的错误恢复机制，并结合 **同步集（synchronization set）** 策略，在出现非致命错误时尝试局部跳过非法 token，继续分析后续结构。

##### 4.2.4.1 错误分类与处理原则

语法分析阶段的错误主要分为两类：

| 类型 | 描述 | 处理方式 |
| - | - | - |
| 可恢复错误 | 某语法结构缺失、token 不匹配、缺少分隔符等 | 报告错误，跳过同步点后继续 |
| 致命错误 | 词法分析失败、Token 流终止、结构栈不一致等 | 立即终止 |

解析器中的 `consume()` 函数专门用于检测 token 类型并处理不匹配情况：

```cpp
void Parser::consume(TokenType type, const std::string &msg) {
  if (!match(type)) {
    reporter.report(
      err::ParErrType::UNEXPECT_TOKEN,
      msg,
      cur.pos,
      cur.value
    );
  }
}
```

该函数在语法结构中起双重作用：

1. 成功匹配 -> 正常推进；
2. 匹配失败 -> 上报错误，但继续尝试向前推进或恢复。

##### 4.2.4.2 Panic-Mode 恢复机制

Panic-mode 是一种经典的语法错误恢复策略，其核心思想是：

> 在出现语法错误时，跳过 token，直到遇到可作为下一个语法结构起点的同步点 token（同步集）为止。

在本解析器中，常见同步点包括：

1. 语句结束符：`;`

2. 块结束符：`}`

3. 控制结构关键字：`fn`, `let`, `return`, `if`, `for`, `loop`

在实际实现中，只是跳过了当前 token，而没有做循环的跳跃逻辑。

##### 4.2.4.3 小结

本节所实现的错误恢复机制具备以下特征：

- 使用 panic-mode + 同步集 在结构级别实现非致命容错；

- 所有语法错误通过统一接口 reporter 上报，便于后续格式化输出；

- 错误信息具备位置、上下文与建议提示，提高调试效率。

该机制可大幅提升解析器的鲁棒性，使其能够正确处理部分语法异常并继续分析。

## 5 AST详细设计

### 5.1 AST 节点设计

在本项目中，我们为语言的各类语句与表达式设计了一套 **结构化、面向对象的 AST（抽象语法树）节点体系**。每类语法成分均对应一个派生自抽象基类 `Node` 的具体节点结构，并通过智能指针统一管理节点生命周期，便于后续分析与可视化处理。

#### 5.1.1 设计原则

1. **语法对齐**：每种语言结构均映射为一个对应的 AST 节点类，确保语法到语义结构的准确还原。

2. **层次分明**：节点之间通过组合表达语义层级。

3. **类型枚举分发**：每个节点实现 `type()` 方法，返回 `NodeType` 枚举值，用于类型匹配与动态分发。

4. **可视化友好**：节点设计天然支持转为可视化结构 `DOT` 格式，用于调试与展示。

#### 5.1.2 节点类型

根据上述实际实现的产生式，对不同类型的节点予以定义，由于本次实现功能已经超出基本要求，在 AST 节点定义中已经给出所有要求所需要的节点类型。

- **基础节点类型**
  - Prog：程序入口，包含所有顶层声明。
  - Decl：通用声明类型，是函数声明等的基类。
  - Stmt：语句基类。
  - Expr：表达式基类。
  - VarType：变量类型（如 i32、\[i32 ; 3\]、(i32, i32)）。
  - Arg：函数参数节点。

- **声明与语句类节点**
  - FuncDecl：函数定义，包括头部和主体。
  - FuncHeaderDecl：函数头，包含名称、返回类型与参数。
  - BlockStmt：语句块，表示 { ... }。
  - ExprStmt：表达式语句。
  - RetStmt：返回语句。
  - VarDeclStmt：变量声明。
  - AssignStmt：赋值语句。
  - VarDeclAssignStmt：声明并赋值语句。
  - IfStmt：if 条件语句，包含多个 ElseClause。
  - ElseClause：if 的 else 或 else-if 子句。
  - WhileStmt：while 循环语句。
  - ForStmt：for 循环语句。
  - LoopStmt：无限循环语句。
  - BreakStmt：中断语句。
  - ContinueStmt：跳过语句。
  - NullStmt：空语句 ;
- **表达式类节点**
  - Number：数字常量。
  - Factor：因子节点，表达式的基本单位。
  - ComparExpr：比较表达式。
  - ArithExpr：算术表达式。
  - CallExpr：函数调用表达式。
  - ParenthesisExpr：括号表达式。
  - FuncExprBlockStmt：匿名函数表达式（带语句块）。
  - IfExpr：条件表达式（if 表达式）。
  - ArrayElements：数组元素列表。
  - TupleElements：元组元素列表。
- **类型节点**
  - Integer：整数类型。
  - Array：数组类型。
  - Tuple：元组类型。
- **左值与访问节点**
  - Variable：变量。
  - Dereference：解引用。
  - ArrayAccess：数组访问（如 a[i]）。
  - TupleAccess：元组访问（如 t.0）。

各节点通过重写 `NodeType type() const` 方法实现运行时类型识别，并支持向下转型操作，以支持语法结构分析与转换。

### 5.2 AST 构建

抽象语法树（AST）的构建是在语法分析阶段完成的。每一种语法结构都对应一个具体的 AST 节点类型，这些节点都派生自统一的基类 `Node`。节点的构造过程与语法规则一一对应，在语法匹配的过程中由递归下降分析器动态生成。

整个 AST 结构通过智能指针组织为 **一棵具有层级关系的树**，能够完整表达源代码的语法结构。下面是 AST 中基础与关键结构体的定义的示例：

- 基础节点类型定义

```cpp
// 所有 AST 结点的基类
struct Node {
  util::Position pos{0, 0};

  Node() = default;
  virtual ~Node() = default;

  void setPos(std::size_t row, std::size_t col) { pos = util::Position{row, col}; }
  void setPos(util::Position pos) { this->pos = pos; }
  [[nodiscard]] auto getPos() const -> util::Position { return pos; }
  [[nodiscard]] virtual auto type() const -> NodeType = 0;
};
using NodePtr = std::shared_ptr<Node>;
```

- 程序入口节点定义

```cpp
// Program
struct Prog : Node {
  std::vector<DeclPtr> decls; // declarations

  Prog() = default;
  explicit Prog(const std::vector<DeclPtr>& ds);
  explicit Prog(std::vector<DeclPtr>&& ds);

  NodeType type() const override {
    return NodeType::Prog;
  }
};
using  ProgPtr = std::shared_ptr<Prog>;
```

- 声明节点基类

```cpp
// Declaration 节点
struct Decl : Node {
  NodeType type() const override {
    return NodeType::Decl;
  }
  virtual ~Decl() = default;
};
using DeclPtr = std::shared_ptr<Decl>;

// Variable Type
struct VarType : Node {
  RefType ref_type = RefType::Normal;

  VarType() = default;
  explicit VarType(RefType rt) : ref_type(rt) {}
  virtual ~VarType() = default;

  NodeType type() const override {
    return NodeType::VarType;
  }
};
using  VarTypePtr = std::shared_ptr<VarType>;

// Argument
struct Arg : Node {
  VarDeclBodyPtr variable; // variable
  VarTypePtr     var_type; // variable type

  Arg() = default;
  explicit Arg(const VarDeclBodyPtr& var, const VarTypePtr& vt)
    : variable(var), var_type(vt) {}
  explicit Arg(VarDeclBodyPtr&& var, VarTypePtr&& vt)
    : variable(std::move(var)), var_type(std::move(vt)) {}

  NodeType type() const override {
    return NodeType::Arg;
  }
};
using  ArgPtr = std::shared_ptr<Arg>;

// Statement
struct Stmt : Node {
  virtual ~Stmt() = default;
  NodeType type() const override {
    return NodeType::Stmt;
  }
};
using  StmtPtr = std::shared_ptr<Stmt>;

// Expression
struct Expr : Node {
  virtual ~Expr() = default;
  NodeType type() const override {
    return NodeType::Expr;
  }
};
using  ExprPtr = std::shared_ptr<Expr>;
```

- 具体节点拓展，此处以 **函数头声明节点** 为例

```cpp
// Function header declaration
struct FuncHeaderDecl : Decl {
  std::string               name;        // 函数名
  std::vector<ArgPtr>       argv;        // 参数列表
  std::optional<VarTypePtr> retval_type; // 返回类型

  FuncHeaderDecl() = default;
  explicit FuncHeaderDecl(const std::string& n,
    const std::vector<ArgPtr>& av,
    const std::optional<VarTypePtr>& rt)
    : name(n), argv(av), retval_type(rt) {}
  explicit FuncHeaderDecl(std::string&& n,
    std::vector<ArgPtr>&& av,
    std::optional<VarTypePtr>&& rt);

  NodeType type() const override {
    return NodeType::FuncHeaderDecl;
  }
};
using FuncHeaderDeclPtr = std::shared_ptr<FuncHeaderDecl>;
```

下面按照逻辑模块进行分层展示各节点之间的结构与类别：

```shell
Node
├── Prog
├── Arg
├── Decl
│   ├── FuncDecl
│   ├── VarDeclStmt
│   │   └── VarDeclAssignStmt
│   └── FuncHeaderDecl
├── Stmt
│   ├── BlockStmt
│   │   └── FuncExprBlockStmt
│   ├── ExprStmt
│   ├── RetStmt
│   ├── VarDeclStmt
│   │   └── VarDeclAssignStmt
│   ├── AssignStmt
│   ├── VarDeclAssignStmt
│   ├── ElseClause
│   ├── IfStmt
│   ├── WhileStmt
│   ├── ForStmt
│   ├── LoopStmt
│   ├── BreakStmt
│   ├── ContinueStmt
│   └── NullStmt
├── Expr
│   ├── Number
│   ├── Factor
│   ├── ComparExpr
│   ├── ArithExpr
│   ├── CallExpr
│   ├── ParenthesisExpr
│   ├── FuncExprBlockStmt
│   ├── LoopStmt
│   ├── IfExpr
│   ├── ArrayElements
│   └── TupleElements
├── VarType
│   ├── Integer
│   ├── Array
│   └── Tuple
├── VarDeclBody
└── AssignElement
    ├── Variable
    ├── Dereference
    ├── ArrayAccess
    └── TupleAccess
```

<br>

## 6 符号表详细设计

在编译器的构建过程中，语义分析是继词法分析和语法分析之后的关键阶段。其主要任务是对抽象语法树（AST）进行语义层面的检查，确保源程序不仅在结构上合法，更在语义上正确。本项目作为编译原理课程的实验内容，目标是实现一个简化语言的语义检查模块，识别出诸如未声明变量、变量重复声明、未初始化使用、函数调用不匹配等常见语义错误，具体会在下面给出。

语义检查的实现依赖于 *符号表（Symbol Table）* 维护程序中的变量、函数等标识符的信息，并通过遍历抽象语法树逐个检查各类语句与表达式是否满足语言的语义规则。本模块与前期的语法分析器相衔接，作为后续中间代码生成和优化的前置保障，其准确性和鲁棒性直接影响整个编译流程的可靠性。

### 6.1 符号表结构

本项目中的符号表模块由命名空间 `symbol` 下的多个结构体与类构成，主要包括 `Symbol、Variable、Function` 以及核心管理类 `SymbolTable`，实现了对变量与函数的统一管理和作用域支持。

```cpp
struct Symbol {
  std::string name;
};
using SymbolPtr = std::shared_ptr<Symbol>;

enum class VarType : std::uint8_t {
  Null,  // 返回值类型可以为空，即代表不返回任何值
  I32,
  Unknown,
};

struct Variable : Symbol {
  bool initialized = false;  // 是否已经初始化
  bool formal = false;
  VarType var_type = VarType::I32;  // 变量类型
};
using VariablePtr = std::shared_ptr<Variable>;

struct Integer : Variable {
  std::optional<std::int32_t> init_val;  // 初值
};
using IntegerPtr = std::shared_ptr<Integer>;

struct Function : Symbol {
  int argc;  // 参数个数
  VarType retval_type;
  void setRetvalType(VarType rvt) { retval_type = rvt; }
};
using FunctionPtr = std::shared_ptr<Function>;
```

- Symbol 为所有符号的抽象基类，仅包含符号名称；

- Variable 继承自 Symbol，表示一个变量，包含以下核心属性：

  - initialized：指示变量是否已经初始化；

  - formal：是否为函数形参；

  - var_type：变量的类型，当前仅完成基础规则，当前默认`I32`；

- Integer 继承自 Variable，进一步扩展支持整数初值 init_val，用于后续中间代码生成；

- Function 表示一个函数符号，包含函数名、参数个数 argc 与返回值类型 retval_type。

### 6.2 作用域支持

SymbolTable 类采用嵌套作用域管理的方式设计，使用 `std::unordered_map<std::string, ScopePtr>` 存储多个作用域（Scope），每个作用域本质上是一个从变量名到变量指针的哈希表。作用域通过名称 `cscope_name` 进行区分，进入或退出作用域时会修改当前作用域指针 `p_cscope`。

```cpp
class SymbolTable {
public:
  SymbolTable() {
    p_cscope = std::make_shared<Scope>();
    cscope_name = "global";
    scopes[cscope_name] = p_cscope;
  }
  ~SymbolTable() = default;

public:
  void enterScope(const std::string& name, bool create_scope = true);
  auto exitScope() -> std::string;

  void declareFunc(const std::string& fname, FunctionPtr p_func);
  void declareVar(const std::string& vname, VariablePtr p_var);
  // 允许函数和变量同名，因此分成两部分处理
  [[nodiscard]]
  optional<FunctionPtr> lookupFunc(const string& name) const;
  [[nodiscard]]
  optional<VariablePtr> lookupVar(const std::string& name) const;
  void printSymbol(std::ofstream& out);
  auto getCurScope() const -> const std::string&;
  auto getTempValName() -> std::string;
  auto getFuncName() -> std::string;
  std::vector<std::string> checkAutoTypeInference() const;
private:
  using Scope = std::unordered_map<std::string, VariablePtr>;
  using ScopePtr = std::shared_ptr<Scope>;
  ScopePtr p_cscope;        // pointer (current scope)
  std::string cscope_name;  // 作用域限定符

  int tv_cnt;  // temp value counter
  std::unordered_map<std::string, ScopePtr> scopes;
  std::unordered_map<std::string, FunctionPtr> funcs;
};
```

这种设计允许灵活支持局部变量与变量屏蔽机制，为后续实现作用域嵌套与函数体内变量管理提供良好支持。

### 6.3 查询与声明机制

为了支持变量与函数同名，`SymbolTable` **分别对变量与函数使用不同哈希表** 管理：

- `lookupVar(name)`：按当前作用域名查找变量；

```cpp
/**
 * @brief  查找变量符号
 * @param  name 变量名
 * @return std::optional<VariablePtr> 需要检查是否能查到
 */
[[nodiscard]]
optional<VariablePtr> SymbolTable::lookupVar(const string& name)
{
  auto exit_scope = [](std::string& scope_name) -> bool {
    std::size_t idx = scope_name.find_last_of(':');
    if (idx >= scope_name.length()) {
      return false;
    }
    scope_name = scope_name.substr(0, idx - 1);
    return true;
  };

  auto scope_name = cscope_name;

  bool flag = false;
  VariablePtr p_var;
  do {
    auto p_scope = scopes.find(scope_name)->second;
    if (p_scope->contains(name)) {
      flag = true;
      p_var = p_scope->find(name)->second;
      break;
    }
  } while (exit_scope(scope_name));

  return flag ? std::optional<VariablePtr>{p_var} : std::nullopt;
}
```

- lookupFunc(name)：全局查找函数；

```cpp
/**
 * @brief  查找函数符号
 * @param  name 函数名
 * @return std::optional<FunctionPtr> 需要检查是否能查到
 */
[[nodiscard]]
optional<FunctionPtr> SymbolTable::lookupFunc(const string& name)
{
  if (funcs.contains(name)) {
    return funcs.find(name)->second;
  }
  return std::nullopt;
}
```

- declareVar() 与 declareFunc() 方法则用于声明符号，若重复声明则会在语义分析阶段报错。

此外还提供临时值命名 getTempValName()、当前函数名获取 getFuncName()、类型自动推导检查 checkAutoTypeInference() 等辅助功能，便于后续的语义检查和中间代码生成与分析。

### 6.4 符号表输出

以如下代码为例：

```rs
fn foo(mut a : i32, mut b : i32) -> i32
{
    let mut a;
    a=b;
    return a+foo(a,b);
}
fn foo2(mut c: i32)
{
    let mut x;
    if c > 1 {
        x=c;
        c=c*2;
    }
    while c>1{
        let mut c;
        c=5;
        if c>10{
        }
        else{
            c=c+1;
        }
    }
}
fn main(mut a :i32)
{
    let mut b : i32;
    a=1;
    b=666+999*(2<4);
    1;
    (((a)));
    b=a*2;
    a=foo(0,0);
    foo(0,0);
    foo2(foo(a,b));
}
```

检查正确后，可以得到以下的符号表 **output.symbol**：

<img src="ex_symboltable.png" width=500/>

<p>
<br>

## 7 语义检查详细设计

语义检查阶段是编译器前端的重要环节，主要用于对词法与语法分析通过的抽象语法树（AST）进行更深层次的分析，以确保程序符号使用合理、类型匹配、作用域合法等语义规范。本项目语义检查的核心实现集中在 `SemanticChecker` 类中。

### 7.1 设计原则

- 确保变量作用域正确，避免使用未声明变量

- 检查变量是否初始化后使用

- 检查函数调用是否合法：是否存在，参数是否匹配

- 检查函数返回是否合法：是否有返回类型，是否有返回表达式

- 保证表达式类型一致性

- 支持作用域嵌套与遮蔽，及`变量重影`

### 7.2 类结构概览

**SemanticChecker** 是本项目中负责整个语义分析流程的核心类，提供了统一的入口与多个私有子程序，用于检查不同类型的语句与表达式：

```cpp
class SemanticChecker {
public:
  SemanticChecker() = default;
  ~SemanticChecker() = default;

  void setErrorReporter(std::shared_ptr<error::ErrorReporter> p_ereporter);
  void setSymbolTable(std::shared_ptr<symbol::SymbolTable> p_stable);

public:
  void checkProg(const ProgPtr& p_prog);

private:
  void checkFuncDecl(const FuncDeclPtr& p_fdecl);
  void checkFuncHeaderDecl(const FuncHeaderDeclPtr& p_fhdecl);
  bool checkBlockStmt(const BlockStmtPtr& p_bstmt);
  void checkVarDeclStmt(const VarDeclStmtPtr& p_vdstmt);
  void checkRetStmt(const RetStmtPtr& p_rstmt);
  auto checkExprStmt(const ExprStmtPtr& p_estmt) -> VarType;
  auto checkExpr(const ExprPtr& p_expr) -> VarType;
  auto checkCallExpr(const CallExprPtr& p_caexpr) -> VarType;
  auto checkComparExpr(const ComparExprPtr& p_coexpr) -> VarType;
  auto checkArithExpr(const ArithExprPtr& p_aexpr) -> VarType;
  auto checkFactor(const FactorPtr& p_factor) -> VarType;
  auto checkVariable(const VariablePtr& p_variable) -> VarType;
  auto checkNumber(const NumberPtr& p_number) -> VarType;
  void checkAssignStmt(const AssignStmtPtr& p_astmt);
  void checkIfStmt(const IfStmtPtr& p_istmt);
  void checkWhileStmt(const WhileStmtPtr& p_wstmt);

private:
  std::shared_ptr<symbol::SymbolTable> p_stable;
  std::shared_ptr<error::ErrorReporter> p_ereporter;
};
```

该类通过 **setSymbolTable** 和 **setErrorReporter** 方法与符号表和错误报告器进行联动，形成语义检查三大组件之一。

### 7.3 检查流程说明

#### 7.3.1 程序级检查

```cpp
void SemanticChecker::checkProg(const ProgPtr& p_prog) {
  for (const auto& p_decl : p_prog->decls) {
    // 最顶层的产生式为 Prog -> (FuncDecl)*
    auto p_fdecl = std::dynamic_pointer_cast<FuncDecl>(p_decl);
    assert(p_fdecl);
    checkFuncDecl(p_fdecl);
  }
}
```

作为整个语义分析的入口，**checkProg** 接收语法分析生成的程序树，依次遍历所有函数定义与语句块，进行全面检查。

#### 7.3.2 函数与作用域

```cpp
void SemanticChecker::checkFuncDecl(const FuncDeclPtr& p_fdecl) {
  p_stable->enterScope(p_fdecl->header->name);
  checkFuncHeaderDecl(p_fdecl->header);
  if (!checkBlockStmt(p_fdecl->body)) {
    // 函数内无return语句
    std::string cfunc_name = p_stable->getFuncName();
    auto opt_func = p_stable->lookupFunc(cfunc_name);
    assert(opt_func.has_value());

    const auto& p_func = opt_func.value();
    if (p_func->retval_type != symbol::VarType::Null) {
      // 有返回类型但无返回值表达式
      p_ereporter->report(
        error::SemanticErrorType::MissingReturnValue,
        std::format(
          "函数 '{}' 需要返回值，但函数内没有 return 语句",
          cfunc_name),
        p_stable->getCurScope());
    }
  }

  p_stable->exitScope();
}

void SemanticChecker::checkFuncHeaderDecl(const FuncHeaderDeclPtr& p_fhdecl)
{
  int argc = p_fhdecl->argv.size();
  for (const auto& arg : p_fhdecl->argv) {
    std::string name = arg->variable->name;
    // 形参的初始值在函数调用时才会被赋予
    auto p_fparam = make_shared<Integer>(name, true, nullopt);
    p_stable->declareVar(name, p_fparam);
  }

  auto rt = p_fhdecl->retval_type.has_value() ? I32 : Null;
  auto p_func = make_shared<Function>(p_fhdecl->name, argc, rt);
  p_stable->declareFunc(p_fhdecl->name, p_func);
}
```

对于每一个函数声明，先声明函数头并注册到符号表，再进入函数体对应作用域检查局部变量声明与语句合法性。

#### 7.3.3 变量与表达式检查

1. 在变量声明阶段，若未提供类型，则将类型标记为 **Unknown**，若在其作用域内没有自动类型推导，则会在退出作用域调用 **checkAutoTypeInference()** 时报错；

```cpp
void SemanticChecker::checkVarDeclStmt(const VarDeclStmtPtr& p_vdstmt)
{
  // 需要实现的产生式中，所有变量声明语句声明的变量只有两种情况
  // 1. let mut a : i32;
  // 2. let mut a;
  // 因此，我们这里简单的记录变量

  const std::string& name = p_vdstmt->variable->name;

  if (p_vdstmt->var_type.has_value()) {
    // 1: let mut a : i32; 明确类型，直接构造变量
    auto p_var = make_shared<Integer>(name, false, nullopt);
    p_var->initialized = false;
    p_stable->declareVar(name, p_var);
  } else {
    // 2: let mut a; 自动类型推导 —— 类型未知
    auto p_var = make_shared<Variable>(name, false, Unknown);
    p_var->initialized = false;
    p_stable->declareVar(name, p_var);
  }
}
```

在每个block结束后，会对作用域下所有变量进行检查：

```cpp
vector<VariablePtr> SymbolTable::checkAutoTypeInference() const
{
  std::vector<VariablePtr> failed_vars;

  for (const auto& [name, p_var] : *p_cscope) {
    if (p_var->var_type == VarType::Unknown) {
      failed_vars.push_back(p_var);
    }
  }

  return failed_vars;
}
```

如果返回有值，也就是**有未确定类型的变量**，需要报错：

```cpp
bool SemanticChecker::checkBlockStmt(const BlockStmtPtr& p_bstmt)
{
  int if_cnt = 1;
  int while_cnt = 1;
  bool has_retstmt = false;

  for (const auto& p_stmt : p_bstmt->stmts) {
    switch (p_stmt->type()) {
    default:
      throw std::runtime_error{"检查到不支持的语句类型"};
    case NodeType::VarDeclStmt:
      checkVarDeclStmt(std::dynamic_pointer_cast<VarDeclStmt>(p_stmt));
      break;
    case NodeType::RetStmt:
      checkRetStmt(dynamic_pointer_cast<RetStmt>(p_stmt));
      has_retstmt = true;
      break;
    case NodeType::ExprStmt:
      checkExprStmt(dynamic_pointer_cast<ExprStmt>(p_stmt));
      break;
    case NodeType::AssignStmt:
      checkAssignStmt(dynamic_pointer_cast<AssignStmt>(p_stmt));
      break;
    case NodeType::IfStmt:
      p_stable->enterScope(format("if{}", if_cnt++));
      checkIfStmt(dynamic_pointer_cast<IfStmt>(p_stmt));
      p_stable->exitScope();
      break;
    case NodeType::WhileStmt:
      p_stable->enterScope(format("while{}", while_cnt++));
      checkWhileStmt(dynamic_pointer_cast<WhileStmt>(p_stmt));
      p_stable->exitScope();
      break;
    case NodeType::NullStmt:
      break;
    }
  }

  auto failed_vars = p_stable->checkAutoTypeInference();
  for (const auto& p_var : failed_vars) {
    p_ereporter->report(SemanticErrorType::TypeInferenceFailure,
      std::format(
        "变量 '{}' 无法通过自动类型推导确定类型",
        p_var->name),
      p_var->pos.row, p_var->pos.col, p_stable->getCurScope());
  }

  return has_retstmt;
}
```

2. 在变量使用阶段，若变量未初始化即使用，则抛出语义错误；

```cpp
VarType SemanticChecker::checkVariable(const VariablePtr& p_variable)
{
  auto opt_var = p_stable->lookupVar(p_variable->name);

  if (!opt_var.has_value()) {
    p_ereporter->report(SemanticErrorType::UndeclaredVariable,
      std::format("变量 '{}' 未声明", p_variable->name),
      p_stable->getCurScope());
    return symbol::VarType::Null;
  }

  const auto& p_var = opt_var.value();

  if (!p_var->initialized && !p_var->formal) {
    p_ereporter->report(SemanticErrorType::UninitializedVariable,
      std::format(
        "变量 '{}' 在第一次使用前未初始化",
        p_variable->name),
      p_stable->getCurScope());
  }

  return p_var->var_type;
}
```

3. 表达式检查会递归分析子表达式，并返回类型用于后续推导判断。顶层函数会对不同表达式进行分发：

```cpp
VarType SemanticChecker::checkExpr(const ExprPtr& p_expr) {
  switch (p_expr->type()) {
  default:
    throw std::runtime_error{"检查到不支持的表达式类型"};
  case NodeType::ParenthesisExpr:...
  case NodeType::CallExpr:...
  case NodeType::ComparExpr:...
  case NodeType::ArithExpr:...
  case NodeType::Factor:...
  case NodeType::Variable:...
  case NodeType::Number:...
  }
}
```

#### 7.3.4 函数调用与返回检查

1. 函数调用检查

```cpp
VarType SemanticChecker::checkCallExpr(const CallExprPtr& p_caexpr)
{
  // 检查调用表达式的实参和形参是否一致
  auto opt_func = p_stable->lookupFunc(p_caexpr->callee);

  if (!opt_func.has_value()) {
    p_ereporter->report(SemanticErrorType::UndefinedFunctionCall,
      std::format("调用了未定义的函数 '{}'", p_caexpr->callee),
      p_stable->getCurScope());
    return symbol::VarType::Null;
  }

  const auto& p_func = opt_func.value();

  if (static_cast<int>(p_caexpr->argv.size()) != p_func->argc) {
    p_ereporter->report(SemanticErrorType::ArgCountMismatch,
      std::format(
        "函数 '{}' 期望 {} 个参数，但调用提供了 {} 个",
        p_caexpr->callee, p_func->argc, p_caexpr->argv.size()),
      p_stable->getCurScope());
  }

  // 遍历所有实参与进行表达式语义检查
  for (const auto& arg : p_caexpr->argv) {
    assert(arg);
    auto rv_type = checkExpr(arg);
    if (rv_type != symbol::VarType::I32) {
        // error report
    }
  }

  return p_func->retval_type;
}
```

按照顺序依次检查函数是否定义，然后检查实参与形参的个数是否匹配，最后检查所有实参的表达式。

2. 函数返回检查

```cpp
void SemanticChecker::checkRetStmt(const RetStmtPtr& p_rstmt) {
  std::string cfunc_name = p_stable->getFuncName();
  auto opt_func = p_stable->lookupFunc(cfunc_name);
  assert(opt_func.has_value());

  const auto& p_func = opt_func.value();
  if (p_rstmt->ret_val.has_value()) {  // 有返回值表达式
    symbol::VarType ret_type = checkExpr(p_rstmt->ret_val.value());

    if (p_func->retval_type != symbol::VarType::Null) {
      // 函数有明确返回类型
      if (p_func->retval_type != ret_type) {
        // 返回值表达式类型与函数返回类型不符
        p_ereporter->report(
          SemanticErrorType::FuncReturnTypeMismatch,
          std::format(
            "函数 '{}' return 语句返回类型错误",
            cfunc_name),
          p_stable->getCurScope());
      }
    } else {
      // 函数不需要返回值，却有返回值表达式
      p_ereporter->report(
        error::SemanticErrorType::VoidFuncReturnValue,
        std::format("函数 '{}' 不需要返回值，return语句却有返回值", cfunc_name),
        p_stable->getCurScope());
    }
  } else {  // return;
      if (p_func->retval_type != symbol::VarType::Null) {
        // 有返回类型但无返回值表达式
        p_ereporter->report(
          error::SemanticErrorType::MissingReturnValue,
          std::format("函数 '{}' 需要返回值，return语句却没有返回值", cfunc_name),
          p_stable->getCurScope());
      }
  }
}
```

区分 return 语句是否有返回值表达式

- 若 return 携带返回值，则进一步检查表达式类型，并与函数返回类型比对；

- 若 return 无返回值，仅为 return;，则视函数是否应返回值判断合法性。

需要注意的是，如果函数没有返回语句，在block调用时会有一个bool型的变量**has_retstmt**作为记录，若无返回语句，视为 **return ;** ，因此如果此函数有返回类型，则需要报错：

```cpp
if (p_func->retval_type != symbol::VarType::Null)
{  // 有返回类型但无返回值表达式
  p_ereporter->report(
    error::SemanticErrorType::MissingReturnValue,
    std::format("函数 '{}' 需要返回值，函数内没有return语句", cfunc_name),
    p_stable->getCurScope());
}
```

#### 7.3.5 选择循环语句检查

根据基础规则，对**IF-ELSE**和**WHILE**语句进行检查，简化检查逻辑，仅判断表达式是否合法，由于I32可以自动推导为Bool，因此只判断为I32即可。

```cpp
void SemanticChecker::checkIfStmt(const IfStmtPtr& p_istmt) {
  checkExprStmt(std::make_shared<ExprStmt>(p_istmt->expr));
  checkBlockStmt(p_istmt->if_branch);

  assert(p_istmt->else_clauses.size() <= 1);
  if (p_istmt->else_clauses.size() == 1) {
    p_stable->enterScope("else");
    checkBlockStmt(p_istmt->else_clauses[0]->block);
    p_stable->exitScope();
  }
}
```

```cpp
void SemanticChecker::checkWhileStmt(const WhileStmtPtr& p_wstmt)
{
  checkExprStmt(std::make_shared<ExprStmt>(p_wstmt->expr));
  checkBlockStmt(p_wstmt->block);
}
```

### 7.4 错误处理机制

语义分析阶段发现的错误，统一通过 **ErrorReporter** 报告，并结合当前作用域提供精确定位信息。部分辅助错误信息使用 std::format 格式化输出，增强可读性。

### 7.5 模块特点总结

- 模块化设计良好，支持快速定位每类语义问题；

- 表达式与语句检查相互独立，递归结构清晰；

- 结合符号表实现变量作用域管理；

- 具备错误容忍能力，可在发现错误后继续分析后续节点，增强体验。

<br>

## 8 中间代码生成详细设计

### 8.1 四元式设计

#### 8.1.1 四元式操作符

本项目选用四元式作为中间代码，为此我们设计了一套四元式操作符。这套操作符能高效的按照语义规则将源代码转换为中间代码。

在 `ir_generate/ir_generate.hpp` 中定义了枚举类 `ir::OpCode` 如下：

```cpp
enum class OpCode : std::int8_t {
  Add, Sub, Mul, Div,

  Jeq, Jne, Jge,
  Jgt, Jle, Jlt,

  Eq, Neq,
  Geq, Gne,
  Leq, Lne,

  Decl, Assign,

  Label, Goto,

  Push, Pop,
  Call, Return
};
```

这里定义了六组操作符，其中第一组用于算术运算、第二组用于控制流、第三组用于比较表达式、第四组用于变量声明和赋值、第五组用于标号声明和跳转、第六组用于函数调用和返回。

规定四元式的形式为 `(op, arg1, arg2, res)`，下面给出操作符的语义动作对照表：

| 操作符 | 动作 |
|:-----:|:----:|
| Add | res = arg1 + arg2 |
| Sub | res = arg1 - arg2 |
| Mul | res = arg1 * arg2 |
| Div | res = arg1 / arg2 |
| Jeq | if arg1 == arg2 goto res |
| Jne | if arg1 != arg2 goto res |
| Jge | if arg1 >= arg2 goto res |
| Jgt | if arg1 > arg2 goto res |
| Jle | if arg1 <= arg2 goto res |
| Jlt | if arg1 < arg2 goto res |
| Eq | res = arg1 == arg2 |
| Neq | res = arg1 != arg2 |
| Geq | res = arg1 >= arg2 |
| Gne | res = arg1 > arg2 |
| Leq | res = arg1 <= arg2 |
| Lne | res = arg1 < arg2 |
| Decl | 声明一个变量 arg1 |
| Assign | res = arg1 |
| Label | 声明一个标签 arg1 |
| Goto | 跳转到 arg1 |
| Push | 为形参 arg1 构造栈帧 |
| Pop | 从栈帧中弹出一个参数赋给 arg1 |
| Call | 调用 arg1 |
| Return | 从调用函数返回 |

#### 8.1.2 四元式数据结构

同样在 `ir/ir_generate.hpp` 中定义了四元式 `struct Quad` 和操作数 `struct Operand` 如下：

```cpp
struct Operand {
  std::string name;

  Operand() = default;
  Operand(std::string name) : name(std::move(name)) {}
};

struct Quad {
  OpCode op;
  Operand arg1;
  Operand arg2;
  Operand res;
};
```

这与 8.1.1 所假设的四元式结构一致。这里简化了操作数结构，只存储了操作数的名称。

### 8.2 中间代码生成器设计

#### 8.2.1 IrGenerator 类

在 `ir/ir_generator.hpp` 中定义了 `class IrGenerator` 如下：

```cpp
class IrGenerator {
public:
  IrGenerator() = default;
  ~IrGenerator() = default;
public:
  void setSymbolTable(std::shared_ptr<SymbolTable> p_stable);
  void generateProg(const ProgPtr& p_prog);
  void printQuads(std::ofstream& out) const;
private:
  void generateFuncDecl(const FuncDeclPtr&);
  void generateFuncHeaderDecl(const FuncHeaderDeclPtr&);
  auto generateBlockStmt(const BlockStmtPtr&);
  void generateVarDeclStmt(const VarDeclStmtPtr&);
  void generateRetStmt(const RetStmtPtr&);
  void generateExprStmt(const ExprStmtPtr&);
  auto generateExpr(const ExprPtr&);
  auto generateCallExpr(const CallExprPtr&);
  auto generateComparExpr(const ComparExprPtr&);
  auto generateArithExpr(const ArithExprPtr&);
  auto generateFactor(const FactorPtr&);
  auto generateElement(const ExprPtr&);
  auto generateParenthesisExpr(const ParenthesisExprPtr&);
  auto generateNumber(const NumberPtr&);
  auto generateVariable(const VariablePtr&);
  void generateAssignStmt(const AssignStmtPtr&);
  void generateIfStmt(const IfStmtPtr&);
  void generateWhileStmt(const WhileStmtPtr&);
  [[nodiscard]] auto getVarName(const std::string&) const;
  [[nodiscard]] auto getFuncName() const -> std::string;
  void pushQuads(OpCode, const Operand&, const Operand&, const Operand&);

private:
  std::vector<Quad> quads;
  std::shared_ptr<symbol::SymbolTable> p_stable;
};
```

可以看到，中间代码生成器类中定义了一个四元式列表 `quads` 存储顺序生成的所有四元式，定义了一个 `p_stable` 指向全局共享的符号表。

这里以 `generate` 开头的一组函数是整个中间代码生成器的核心，其以深度优先、从左到右的顺序遍历语法分析器生成的 AST，并根据语义规则生成中间代码。

该类主要向外部暴露了两个接口：`generateProg` 和 `printQuads` 分别用于生成四元式和打印已生成的四元式。

#### 8.2.2 核心代码逻辑分析

下面以 `generateBlockStmt` 和 `generateIfStmt` 为例，详细说明代码逻辑。

**1. generateBlockStmt**:

```cpp
bool IrGenerator::generateBlockStmt(const BlockStmtPtr& p_bstmt)
{
  int if_cnt = 1;
  int while_cnt = 1;

  for (const auto& p_stmt : p_bstmt->stmts) {
    switch (p_stmt->type()) {
    default:
      throw std::runtime_error{"检查到不支持的语句类型"};
    case NodeType::VarDeclStmt:
      generateVarDeclStmt(std::dynamic_pointer_cast<VarDeclStmt>(p_stmt));
      break;
    case NodeType::RetStmt:
      generateRetStmt(dynamic_pointer_cast<RetStmt>(p_stmt));
      return true;
    case NodeType::ExprStmt:
      generateExprStmt(dynamic_pointer_cast<ExprStmt>(p_stmt));
      break;
    case NodeType::AssignStmt:
      generateAssignStmt(
        std::dynamic_pointer_cast<AssignStmt>(p_stmt)
      );
      break;
    case NodeType::IfStmt:
      p_stable->enterScope(std::format("if{}", if_cnt++), false);
      generateIfStmt(std::dynamic_pointer_cast<IfStmt>(p_stmt));
      p_stable->exitScope();
      break;
    case NodeType::WhileStmt:
      p_stable->enterScope(
        std::format("while{}", while_cnt++),
        false
      );
      generateWhileStmt(
        std::dynamic_pointer_cast<WhileStmt>(p_stmt)
      );
      p_stable->exitScope();
      break;
    case NodeType::NullStmt:
     break;
    }
  }
  return false;
}
```

在上述函数中，我们使用范围 for 循环遍历语句块中的每个语句，并通过 AST 节点类的 `type()` 方法进行类型分发，调用不同语句类型的处理函数。

可以看到，函数逻辑上呈现递归下降的形式，对于每个产生式，都会优先生成产生式右侧符号相关的四元式，最后再生成该产生式对应的四元式。

需要注意的是，我们维护了一个带有作用域的符号表，以实现作用域屏蔽功能，因此对于每个函数、If 语句 和 While 语句，都需要先进入作用域，并在生成结束后退出作用域。这里是通过调用符号表提供的 `enterScope` 和 `exitScope` 方法完成的。

需要注意的是，中间代码生成环节的 `enterScope` 不需要创建一个新的作用域，因此我们传入了一个额外的参数控制这一行为。

还可以看到，我们使用了两个计数器 `if_cnt` 和 `while_cnt`，这是因为同一个语句块内可能有多个 if 和 while 语句，需要通过计数器区分不同 if/while 语句的作用域。

**2. generateIfStmt**:

```cpp
void IrGenerator::generateIfStmt(const IfStmtPtr& p_istmt) {
  std::string label_true = std::format(
    "{}_true",
    replaceScopeQualifiers(p_stable->getCurScope())
  );
  std::string label_false = std::format(
    "{}_false",
    replaceScopeQualifiers(p_stable->getCurScope())
  );
  std::string label_end = std::format(
    "{}_end",
    replaceScopeQualifiers(p_stable->getCurScope())
  );

  std::string lhs;
  std::string rhs;
  OpCode op;  // 跳转到 true

  // 如果 if 语句的判断条件并非比较表达式，
  // 则使用 jne condition 0 来跳转到 if 分支
  std::string scope = p_stable->exitScope();
  if (p_istmt->expr->type() != NodeType::ComparExpr) {
    lhs = generateExpr(p_istmt->expr);
    rhs = "0";
    op = OpCode::Jne;
  } else {
    auto p_coexpr =
      std::dynamic_pointer_cast<ComparExpr>(p_istmt->expr);
    lhs = generateExpr(p_coexpr->lhs);
    rhs = generateExpr(p_coexpr->rhs);

    switch (p_coexpr->op) {
    case ComparOperator::Equal:
      op = OpCode::Jeq;
      break;
    case ComparOperator::Nequal:
      op = OpCode::Jne;
      break;
    case ComparOperator::Gequal:
      op = OpCode::Jge;
      break;
    case ComparOperator::Great:
      op = OpCode::Jgt;
      break;
    case ComparOperator::Lequal:
      op = OpCode::Jle;
      break;
    case ComparOperator::Less:
      op = OpCode::Jlt;
      break;
    }
  }
  p_stable->enterScope(scope, false);
  pushQuads(op, lhs, rhs, label_true);

  // 由于基础产生式不支持 else if 分支，所以这个 assert 成立
  assert(p_istmt->else_clauses.size() <= 1);
  bool has_else = (p_istmt->else_clauses.size() == 1);
  if (has_else) {
    pushQuads(Goto, label_false, NULL_OPERAND, NULL_OPERAND);
  } else {
    pushQuads(Goto, label_end, NULL_OPERAND, NULL_OPERAND);
  }

  pushQuads(Label, label_true, NULL_OPERAND, NULL_OPERAND);
  generateBlockStmt(p_istmt->if_branch);
  if (has_else) {
    pushQuads(Goto, label_end, NULL_OPERAND, NULL_OPERAND);
    pushQuads(Label, label_false, NULL_OPERAND, NULL_OPERAND);
    // 同样是因为没有 else if 所以下面的断言才成立
    assert(!p_istmt->else_clauses[0]->expr.has_value());
    generateBlockStmt(p_istmt->else_clauses[0]->block);
  }
  pushQuads(Label, label_end, NULL_OPERAND, NULL_OPERAND);
}
```

在开发过程中，核心的两部分在于表达式和控制流，而相对来说控制流的生成更加复杂。所以这里以 if 语句为例解释我们如何处理控制流的生成。

总的来说，对于每一个跳转分支我们都声明了一个标签，根据判断条件的计算结果跳转到不同标签处。逻辑上，先生成控制条件计算相关四元式，再生成跳转指令相关四元式，最后依次生成各跳转分支对应四元式，并在适当位置插入标签。

对于 if 语句，我们设计了三个标签 `true`, `else` 和 `end`，分别表示 if 分支、else 分支和 if 语句结束。在存在 else 分支的情况下，如果判断条件为真，则跳转到 `true`，否则 `goto else`（注意这里使用到了有条件跳转的 `J` 系列操作符和无条件跳转的 `Goto`）；不存在 else 分支的情况下，如果判断条件为真，则跳转到 `true`，否则 `goto end`。

<br>

## 9 错误报告器详细设计

### 9.1 错误类型体系设计

本项目的错误报告器采用分层分类的设计理念，根据编译过程将错误为以下三大类：

```cpp
enum class ErrorType : std::uint8_t {
  Lex,       // 词法错误
  Parse,     // 语法错误
  Semantic,  // 语义错误
};
```

错误的基类Error主要包含msg，来记录实际错误发生的实际信息。

```cpp
struct Error {
  std::string msg;  // error message

  Error() = default;
  Error(const std::string& m) : msg(m) {}
  Error(std::string&& m) : msg(std::move(m)) {}

  [[nodiscard]]
  virtual constexpr auto kind() const -> ErrorType = 0;
  virtual ~Error() = default;
};
```

#### 9.1.1 词法错误(LexError)

词法分析主要识别未知token:

```cpp
enum class LexErrorType : std::uint8_t {
  UnknownToken,  // 未知的 token
};
```

词法错误数据结构包括错误类型、位置信息和相关token:

```cpp
struct LexError : Error {
  LexErrorType type;
  std::size_t row;
  std::size_t col;
  std::string token;

  LexError(LexErrorType, const string&, size_t, size_t, string);
};
```

#### 9.1.2 语法错误(ParseError)

语法分析主要识别是否是期待Token：

```cpp
enum class ParseErrorType : std::uint8_t {
  UnexpectToken,  // 并非期望 token
};
```

语法错误数据结构与词法错误类似：

```cpp
struct ParseError : Error {
  ParseErrorType type;
  std::size_t row;
  std::size_t col;
  std::string token;

  ParseError() = delete;
  ParseError(ParseErrorType,const string&,size_t,size_t,string)
};
```

#### 9.1.3 语义错误(SemanticError)

语义分析阶段识别更丰富的错误类型：

```cpp
enum class SemanticErrorType : std::uint8_t {
  FuncReturnTypeMismatch,  // 函数返回值类型错误
  VoidFuncReturnValue,     // void函数返回了值
  MissingReturnValue,      // 非void函数未返回值
  UndefinedFunctionCall,   // 调用未定义函数
  ArgCountMismatch,        // 参数数量不匹配
  UndeclaredVariable,      // 变量未声明
  UninitializedVariable,   // 变量未初始化
  AssignToNonVariable,     // 赋值左侧非变量
  AssignToUndeclaredVar,   // 赋值给未声明变量
  TypeInferenceFailure,    // 变量无法通过自动类型推导确定类型
  TypeMismatch,            // 变量类型不匹配
};
```

语义错误数据结构增加了作用域信息：

```cpp
struct SemanticError : Error {
  SemanticErrorType type;
  std::size_t row;
  std::size_t col;
  std::string scope_name;  // 错误发生的作用域

  SemanticError(SemanticErrorType, const string&,
    size_t, size_t, string)
};
```

### 9.2 错误收集与报告机制

#### 9.2.1 错误报告器核心设计

错误报告器`ErrorReporter`采用观察者模式，以全局身份作为编译过程中各阶段的错误收集中心：

```cpp
class ErrorReporter {
public:
  explicit ErrorReporter(const std::string& t);  // 传入源代码文本

  // 错误报告接口
  void report(LexErrorType, const std::string& msg,
    std::size_t r, std::size_t c, const std::string& token,
    bool terminate = false);
  void report(const LexError& le, bool terminate = false);
  void report(ParseErrorType type, const std::string& msg,
    std::size_t r, std::size_t c, const std::string& token);
  void report(SemanticErrorType type, const std::string& msg,
    std::size_t r, std::size_t c, const std::string& scope_name);

  // 错误显示接口
  void displayLexErrs() const;
  void displayParseErrs() const;
  void displaySemanticErrs() const;

  // 错误存在性检查
  bool hasLexErr() const;
  bool hasParseErr() const;
  bool hasSemanticErr() const;

private:
  std::vector<std::string> text;           // 源代码按行存储
  std::list<LexError> lex_errs;            // 词法错误列表
  std::list<ParseError> parse_errs;        // 语法错误列表
  std::list<SemanticError> semantic_errs;  // 语义错误列表
};
```

#### 9.2.2 错误收集流程

- **词法分析阶段**：
  - 当词法分析器遇到无法识别的token，在函数的末尾return一个`UnknownToken`错误
  - 调用`report()`接口报告`UnknownToken`错误
  - 错误信息包含token内容和位置

```cpp
// 在ToyLexer::nextToken()中的错误处理
return std::unexpected(error::LexError{
  error::LexErrorType::UnknownToken,
  "识别到未知token: " + view.substr(0, 1),
  p.row, p.col,
  view.substr(0, 1)
});
```

```cpp
auto printToken(std::ofstream& out) -> bool {
  while (true) {
    if (auto token = lex->nextToken(); token.has_value()) {
      if (token->getType() == lexer::token::Type::END) {
        break;
      }
    } else {  // 未正确识别 token
      reporter->report(token.error());
    }
  }
}
```

- **语法分析阶段**:
  - report内置于Paser::expect函数中
  - 遇到非期望的token时，通过`expect()`函数报告`UnexpectToken`错误
  - 错误信息包含期望的token类型和实际遇到的token

```cpp
// 在Parser::expect()中的错误处理
void Parser::expect(token::Type type, const string& msg) {
  if (!match(type)) {
    reporter->report(
      error::ParseErrorType::UnexpectToken,
      msg,
      current.getPos().row,
      current.getPos().col,
      current.getValue()
    );
  }
}
```

- **语义分析阶段**:
  - report会在语义分析出现错误时调用
  - 错误信息包含错误类型、位置和相关符号信息

```cpp
// 在SemanticChecker中的错误处理示例
void SemanticChecker::checkRetStmt(const RetStmtPtr& p_rstmt) {
  std::string cfunc_name = p_stable->getFuncName();
  auto opt_func = p_stable->lookupFunc(cfunc_name);
  assert(opt_func.has_value());

  const auto& p_func = opt_func.value();
  if (p_rstmt->ret_val.has_value()) {  // 有返回值表达式
    VarType ret_type = checkExpr(p_rstmt->ret_val.value());

    if (p_func->retval_type != symbol::VarType::Null) {
      // 函数有明确返回类型
      if (p_func->retval_type != ret_type) {
        // 返回值表达式类型与函数返回类型不符
        p_ereporter->report(
          SemanticErrorType::FuncReturnTypeMismatch,
          std::format(
            "函数 '{}' return 语句返回类型错误",
            cfunc_name),
          p_stable->getCurScope());
      }
    } else {
      // 函数不需要返回值，却有返回值表达式
      p_ereporter->report(
        error::SemanticErrorType::VoidFuncReturnValue,
        std::format(
          "函数 '{}' 不需要返回值，return语句却有返回值",
          cfunc_name),
        p_stable->getCurScope());
    }
  } else {  // return;
      if (p_func->retval_type != symbol::VarType::Null) {
        // 有返回类型但无返回值表达式
        p_ereporter->report(
          error::SemanticErrorType::MissingReturnValue,
          std::format(
            "函数 '{}' 需要返回值，return语句却没有返回值",
            cfunc_name),
          p_stable->getCurScope());
      }
  }
}
```

### 9.3 错误可视化

错误报告器提供统一错误显示接口，采用彩色终端输出增强可读性：

```cpp
// 控制字符
static inline const std::string RESET = "\033[0m";
static inline const std::string RED = "\033[1;31m";
static inline const std::string YELLOW = "\033[1;33m";
static inline const std::string BLUE = "\033[1;34m";
static inline const std::string BOLD = "\033[1m";
```

#### 9.3.1 词法错误展示

```cpp
void ErrorReporter::displayUnknownType(const LexError& err) const {
  std::cerr << BOLD << YELLOW << "warning[UnknownToken]"
    << RESET << BOLD
    << ": 识别到未知 token '" << err.token << "'"
    << RESET << std::endl;

  // 显示错误位置
  std::cerr << BLUE << " --> " << RESET
    << "<row: " << err.row + 1
    << ", col: " << err.col + 1 << ">" << std::endl;

  // 显示错误行内容
  std::cerr << BLUE << "  |  " << std::endl
            << BLUE << " " << err.row + 1 << " | " << RESET
            << this->text[err.row] << std::endl;

  // 显示错误位置标记
  std::ostringstream oss;
  oss << " " << err.row + 1 << " | ";
  int delta = oss.str().length() + err.col - 3;
  std::cerr << BLUE << "  |" << std::string(delta, ' ')
            << "^" << RESET << std::endl << std::endl;
}
```

<img src="./lex_err.png" width=500/>

<br>

#### 9.3.2 语义错误展示

```cpp
void ErrorReporter::displaySemanticErr(const SemanticError& err) const {
  auto pair = displaySemanticErrorType(err.type);
  std::cerr << BOLD << RED << "Error[" << pair.first << "]"
    << RESET << BOLD
    << ": " << pair.second << RESET << std::endl;

  // 显示错误位置和作用域
  std::cerr << BLUE << "--> " << RESET
    << "scope: " << err.scope_name
    << " <row: " << err.row + 1
    << ", col: " << err.col + 1 << ">" << std::endl;

  // 显示错误行内容
  std::cerr << BLUE << "  |  " << std::endl
            << BLUE << " " << err.row + 1 << " | " << RESET
            << this->text[err.row] << std::endl;

  // 显示错误位置标记
  std::ostringstream oss;
  oss << " " << err.row + 1 << " | ";
  int delta = oss.str().length() + err.col - 3;
  std::cerr << BLUE << "  |" << std::string(delta, ' ')
            << "^" << RESET << std::endl << std::endl;

  // 显示详细错误信息
  std::cerr << "    Details: " << err.msg << std::endl << std::endl;
}
```

效果展示：

<img src="./semantic_err.png" width=320/>

<p>

## 10 测试与验证

### 10.1 运行环境

本项目在 Archlinux 和 WSL (Archlinux) 环境下完成开发及测试工作，具体信息如下：

<img src="./arch1.png" width=400>

<img src="./arch2.png" width=400>

### 10.2 编译器功能概述与使用方式

本项目实现了一个类 Rust 编程语言的编译器，命名为 `toy_compiler`，具备词法分析、语法分析、语义检查和中间代码生成功能，并可生成相应的中间结果（token 表、AST 图、符号表和四元式）。其支持的命令行选项如下：

**支持的命令行选项**：

| 选项 | 说明 |
|------|------|
| `-h`, `--help` | 显示帮助信息 |
| `-v`, `-V`, `--version` | 显示编译器版本信息 |
| `-i`, `--input <filename>` | 指定输入文件，必须带有后缀 |
| `-o`, `--output <filename>` | 指定输出文件（不包含后缀） |
| `-t`, `--token` | 输出词法分析结果（token） |
| `-p`, `--parse` | 输出语法分析结果（AST） |
| `-s`, `--semantic` | 进行语义检查并输出符号表 |
| `-g`, `--generate` | 生成中间代码（四元式） |

**使用示例**：

```shell
$ ./toy_compiler -t -i test.txt        # 输出词法分析结果
$ ./toy_compiler -p -i test.txt        # 输出语法分析结果（AST）
$ ./toy_compiler -t -p -i test.txt     # 同时输出 Token 和 AST
$ ./toy_compiler -s test.txt -o result # 自定义输出文件名（生成 result.symbol）
```

编译器生成的 .dot 文件可通过 Graphviz 工具进行可视化，命令如下：

```shell
$ dot -Tpng path/to/output.dot -o AST.png
```

执行该命令后即可得到 .png 格式的抽象语法树图像，便于对程序结构进行直观分析。

具体帮助信息界面截图如下：

<img src="output_ex/ex_help.png" width=400/>

编译器版本信息如下：

<img src="output_ex/ex_version.png" width=400/>

### 10.3 编译器功能测试与验证

#### 10.2.1 词法分析器

词法分析器的作用是将源代码转换为一系列有意义的记号（Token），为语法分析阶段做好准备。

以下列类Rust代码为例：

```rs
// comment
/*
 * multiline comment
 */
fn main() {
    /* */
    /*
    /* */
    */
    ; // /* */
}
```

编译器首先将所有注释进行识别与忽略，然后识别其中的词法，使用命令行：

```shell
$ ./build/toy_compiler -t -i test/test_case/1-1_2.rs
```

可以得到 `output.token`，如下：

```shell
<type: FN, value: "fn">@(5, 1)
<type: ID, value: "main">@(5, 4)
<type: LPAREN, value: "(">@(5, 8)
<type: RPAREN, value: ")">@(5, 9)
<type: LBRACE, value: "{">@(5, 11)
<type: SEMICOLON, value: ";">@(10, 5)
<type: RBRACE, value: "}">@(11, 1)
```

每一行为一个 Token，包含以下三部分信息：

- `type`：Token 的类型，如 `FN` 表示关键字 `fn`，`ID` 表示标识符，`LPAREN` 表示左括号等；
- `value`：Token 的实际文本值；
- `@(row, col)`：Token 在源文件中的行列位置，有助于后续错误定位。

从上面的输出可以看出，词法分析器成功识别了函数声明结构中的关键词、标识符、括号、分号与代码块边界，说明基本功能已正确实现。

#### 10.2.2 语法分析器

语法分析器负责将词法分析器生成的 Token 流进一步解析为抽象语法树（AST），用于表示源代码的语法结构。为了验证语法分析器的功能，继续使用上一节中的测试输入，并执行以下命令：

```shell
$ ./build/toy_compiler -p -i test/test_case/1-1_2.rs
```

会给出语法分析的结果：

<img src="output_ex/ex_parser_success.png" width=700/>

如图显示 `Parsing success`，同时可以得到 `output.dot`。

`.dot` 文件并不是语法分析器的直接产物，而是基于语法分析结果（AST）生成的可视化输出。此部分在5.3节中有较为详细的描述。

同样地，可以使用 Graphviz 工具将 .dot 文件渲染为图片：

```shell
$ dot -Tpng output.dot -o output.png
```

可以得到`output.png`，如下：

<img src="output_ex/ex2.png" width=500/>

生成的 `png` 直观展示了语法结构的层次与关系，方便开发者进行验证和调试。

下面给出一些基础部分实现的语法树部分截图，基础规则的示例类 Rust 代码与分析结果和完整实现语法树图，会在文件夹 `test_basic_example` 中给出

**1.1-1.3 基础语句及函数返回语句**：

可参考上面的示例输出png

**1.4&5 函数输入与输出**：

<img src="output_ex/ex3.png" width=600/>

**2.1 变量声明语句**：

<img src="output_ex/ex4.png" width=500/>

**2.2 赋值语句 与 3.1&2 表达式部分**：

<img src="output_ex/ex5.png" width=360/>

**2.3 变量声明赋值语句 与 3.1&2 表达式部分**：

<img src="output_ex/ex6.png" width=400/>

**3.3 函数调用语句 与 3.1&2 表达式部分**：

<img src="output_ex/ex7.png" width=400/>

**4.1&2 选择语句**：

<img src="output_ex/ex8.png" width=650/>

**5.1 while循环语句**：

<img src="output_ex/ex9.png" width=500/>

我们的语法分析器实现到了 **拓展** 部分（即9.2），但 AST 到 dot 的转化目前只实现了基础部分。

因此，以下列代码为例的规则均可以通过词法与语法分析器的分析，得到 `Parsing success`，但暂时没有 dot 的生成。

```rs
fn f5_8_9(mut x:[i32;3],mut y:(i32,i32,i32)) -> i32 {
    let a : i32 = 1;
    if a > 1 {
        return 2;
    } else if a > 0 {
        return 1;
    } else {
        return 0;
    }
    let mut n : i32;
    while n>0 {
        n = n-1;
    }
    for mut i in 1..n+1 {
        n=n-1;
        }
    loop {
        continue;
    }
    let mut b:i32=x[0];
    x[0] = 1;
    let mut c:i32=y.0;
    a.0=x[0]+x[0];
    a.1=a.0;
    return x[1]>a.1;
}
fn f6() -> i32 {
    let mut a : i32 = 1;
    let mut b : &mut i32 = &mut a;
    let mut c : i32 = *b;
    c = *b * 2;
    c = 2 * *b;
    c = *b + 2;
    c = 2 + *b * 2 + *b + 2;
    2 + *b;
    *b * 2 * 3 + 2 + 3 + *b;
    *b = 2;
    let x : i32 = 1;
    let y : & i32 = &x;
    let z : i32 = *y;
    return x+y+z;
}
fn f7_1(mut x: i32, mut y : i32) {
    let mut z = {
        let mut t = x * x + x;
        t = t + x * y;
        t + x * y;
        t
    };
}
fn f7_2(mut x : i32, mut y : i32) -> i32 {
    let mut t = x * x + x;
    t = t + x*y;
    t
}
fn f7_3(mut a : i32) {
    let mut b = if a > 0 {
        1
    } else{
        0
    };
}
fn f7_4() {
    let mut a = loop {
        break 2;
    };
}
fn f9(){
    let mut b: (&i32, &i32);
    let mut c: ((&i32, &i32), &i32);
    let mut d: &mut (i32, i32, i32);
    b = (2>5,a.0);
    c = (1,);
    let e: (i32,);
}
fn main(){
    let mut a : i32;
    let mut a_array:[i32;3];
    let mut b:[&i32;3];
    let mut c:[[&i32;3];3];
    let mut d:&mut[[i32;3];3];
    a_array = [1,2,3];
    let a_tuple:(i32,i32,i32);
    a_tuple=(1,2,f3());
    f1(f2(a_array,a_tuple),666 + 999 / 1 > (2 * 2));
    return ;
}
```

#### 10.2.3 语义检查器

语义检查器按照深度优先、从左到右的顺序遍历语法分析过程中生成的 AST 并检查是否满足事先定义的语义规则。使用 `-s` 选项以执行语义检查，具体命令如下：

```shell
$ ./build/toy_compiler -p -i test/test_case_2/test_semantic.rs
```

运行结果如下：

<img src="output_ex/semantic.png" width=500>

下面给出测试文件 `test_semantic.rs`：

```rs
fn foo(mut a : i32, mut b : i32) -> i32
{
    let mut a;
    a=b;
    return a+foo(a,b);
}
fn foo2(mut c: i32)
{
    let mut x;
    if c > 1 {
        x=c;
    }
    while c>1{
        let mut c;
        c=5;
        if c>10{
        }
        else{
            c=c+1;
        }
    }
}
fn main(mut a :i32)
{
    let mut b : i32;
    a=1;
    b=666+999*(2<4);
    a=foo(0,0);
    foo2(foo(a,b));
}
```

可以看到符号搜集的结果和预期一致。

下面测试代码中有语义错误的情况，首先给出测试用例 `test_error.rs`：

```rs
fn foo(mut a: i32,mut b:i32)->i32
{
    let mut a;
    a=b;
    return ;
}
fn foo2(mut c: i32)
{
    let mut x;

    if c > 1 {
        x=c;
        c=c*2;
    }
    while c>1{
        let mut c;
        c=5;
        if c>10{
            let mut d;
        }
        else{
            let mut e;
            c=c+1;
        }
    }
    return a+foo(a,b);
}
fn main(mut a :i32)
{
    let mut b : i32;
    a=b;
    b=666+999*(2<4);
    b=c;
    1;
    (((a)));
    b=a*2;
    function();
    a=foo(0);
    foo(0,0);
    foo2(foo(a));
}
```

观察可以发现，上述代码有如下错误：

1. foo 函数有返回值，但 return 语句返回值为空；
2. foo2 函数中 while 语句的 if 语句的 true 分支中，变量 d 无法自动推导类型；
3. foo2 函数中 while 语句的 if 语句的 else 分支中，变量 e 无法自动推导类型；
4. foo2 函数的 return 语句中，返回值使用了未声明的变量 a 和 b；
5. foo2 函数返回值为空，但是 return 语句有返回值；
6. main 函数中使用了为初始化的变量 b；
7. main 函数中使用了未声明的变量 c；
8. main 函数中调用了未声明的函数 function；
9. main 函数中 foo 函数调用时参数个数不匹配。

下面运行 `./build/toy_compiler -s -i ./test/test_case_2/test_error.rs` 命令，测试错误检测效果，结果如下：

<img src="output_ex/semantic_error_1.png" width=350>

<p>

<img src="output_ex/semantic_error_2.png" width=320>

从输出结果可以看到，所有不符合语义规则的错误均被正确识别到了。

#### 10.2.4 中间代码生成器

使用 10.2.3 节中的测试用例 `test_semantic.rs`，运行如下命令可得输出文件 `output.ir`：

```shell
$ ./build/toy_compiler -g -i test/test_case_2/test_semantic.rs
```

得到的四元式序列如下：

```shell
(label, foo, -, -)
(pop, -, -, global::foo::a)
(pop, -, -, global::foo::b)
(decl, global::foo::a, i32, -)
(=, global::foo::b, -, global::foo::a)
(push, global::foo::a, -, -)
(push, global::foo::b, -, -)
(call, foo, -, t0)
(+, global::foo::a, t0, t1)
(return, t1, -, -)
(label, foo2, -, -)
(pop, -, -, global::foo2::c)
(decl, global::foo2::x, i32, -)
(j>, global::foo2::c, 1, global_foo2_if1_true)
(goto, global_foo2_if1_end, -, -)
(label, global_foo2_if1_true, -, -)
(=, global::foo2::if1::c, -, global::foo2::if1::x)
(label, global_foo2_if1_end, -, -)
(label, global_foo2_while1_start, -, -)
(j<=, global::foo2::c, 1, global_foo2_while1_end)
(decl, global::foo2::while1::c, i32, -)
(=, 5, -, global::foo2::while1::c)
(j>, global::foo2::while1::c, 10, global_foo2_while1_if1_true)
(goto, global_foo2_while1_if1_false, -, -)
(label, global_foo2_while1_if1_true, -, -)
(goto, global_foo2_while1_if1_end, -, -)
(label, global_foo2_while1_if1_false, -, -)
(+, global::foo2::while1::if1::c, 1, t2)
(=, t2, -, global::foo2::while1::if1::c)
(label, global_foo2_while1_if1_end, -, -)
(goto, global_foo2_while1_start, -, -)
(label, global_foo2_while1_end, -, -)
(return, -, -, -)
(label, main, -, -)
(pop, -, -, global::main::a)
(decl, global::main::b, i32, -)
(=, 1, -, global::main::a)
(<, 2, 4, t3)
(*, 999, t3, t4)
(+, 666, t4, t5)
(=, t5, -, global::main::b)
(push, 0, -, -)
(push, 0, -, -)
(call, foo, -, t6)
(=, t6, -, global::main::a)
(push, global::main::a, -, -)
(push, global::main::b, -, -)
(call, foo, -, t7)
(push, t7, -, -)
(call, foo2, -, -)
(return, -, -, -)
```

检查发现，这与预期结果完全一致。

<br>

## 11 总结与展望

本项目实现了一个简化类 Rust 语言的编译器前端，涵盖了从词法分析、语法分析、语义检查到中间代码生成的完整流程。通过团队成员的协作，我们不仅实现了各模块的核心功能，还额外完成了 AST 可视化与语义错误诊断，提升了可维护性与可视化能力。

**项目成果回顾**：

- 构建了基于 LL(2) 的递归下降语法分析器；

- 支持丰富的语法结构：如 if 表达式、函数调用、数组与元组；

- 实现语义分析，包括变量作用域管理与未初始化变量检查；

- 提供语法树可视化功能，生成 DOT 图并渲染为 PNG 图；

- 基于四元式格式生成中间代码，具备基本可扩展性；

- 融合 clang-tidy、clang-format 和 git hooks 实现轻量级 CI 流程。

**不足与未来工作**：

尽管项目已经实现了预期的主要功能，但仍存在以下不足和优化空间：

- 语义分析不够完备：未完全支持所有 Rust 类型系统特性，如类型推导、生命周期；

- 中间代码生成待扩展：目前仅支持基本四元式生成，尚未引入优化与控制流图；

- 错误恢复能力弱：一旦发生语法错误，编译流程将中断，后续可引入 panic 模式或容错机制；

- 未集成后端模块：未覆盖目标代码生成与机器指令映射，完整编译链尚未闭环；

- 可测试性有待增强：缺乏系统性单元测试与覆盖率分析。

后续可进一步尝试集成 LLVM 后端、引入 SSA 表示、实现寄存器分配与死代码消除等优化策略，真正构建一个具备教学与实验双重价值的教学编译器。

<br>

## 12 参考文献与资料

### 12.1 网站类资料

| 名称  | 链接 | 说明 |
|------|-----|------|
| 南京大学编译原理公开课 | [https://space.bilibili.com/479141149/lists/2312309?type=season](https://space.bilibili.com/479141149/lists/2312309?type=season) | 词法分析器的设计 |
| Rust 官方书籍（The Rust Programming Language） | [https://doc.rust-lang.org/book/title-page.html](https://doc.rust-lang.org/book/title-page.html) | 了解 Rust 语言，理解词法和语法规则 |
| Graphviz DOT 语言参考文档 | [https://graphviz.org/doc/info/lang.html](https://graphviz.org/doc/info/lang.html) | 用于 AST 可视化的 DOT 图语言文档 |
| Rust 源码中的词法分析实现 | [https://github.com/rust-lang/rust/blob/master/compiler/rustc_lexer/src/lib.rs](https://github.com/rust-lang/rust/blob/master/compiler/rustc_lexer/src/lib.rs) | Rust 编译器源码中的 lexer 模块参考 |
| C++ 标准文档 | [https://en.cppreference.com/](https://en.cppreference.com/) | C++ 库函数用法参考 |

### 12.2 书籍资料

| 书名 | 简要说明 |
|------|----------|
| 《A Tour of C++》 | C++ 作者 Stroustrup 的简明教程，覆盖最新的 C++ 语法以及最佳实践。 |
| 《C++ Templates: The Complete Guide, 2nd Edition》 | 深入理解模板编程，为 AST 与 DOT 输出的泛型模板使用提供理论支持。 |
| 《正则表达式必知必会（修订版）》 | 协助构建词法分析器中的正则表达式匹配规则。 |
| 《Effective Modern C++》 | 优化 C++11/14 的语法使用，提升编译器模块的效率与鲁棒性。 |
| 《modern-cpp-tutorial 中文版》 | 用于理解现代 C++ 特性，尤其是智能指针、可变参数模板等语法要点。 |
