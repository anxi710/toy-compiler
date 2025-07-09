# 产生式

**0.1 变量声明内部**:

- InnerVarDecl -> "mut" "\<ID\>"

**0.2 类型**:

- Type -> "i32" | "()" | "bool"
  - Tips: () 是基础类型 unit type

**0.3 可赋值元素**:

- AssignElem -> Variable
- Variable -> "\<ID\>"

**1.1 基础程序**:

- Prog -> (Decl)*
- Decl -> FuncDecl
- Expr -> StmtBlockExpr
- FuncDecl -> FuncHeaderDecl StmtBlockExpr
- FuncHeaderDecl -> "fn" "\<ID\>" "(" Args ")"
- Args -> $\epsilon$
- StmtBlockStmt -> _{_ (Stmt)* _}_
- Stmt -> $\epsilon$

**1.2 语句（前置规则 1.1）**:

- Stmt -> EmptyStmt
- EmptyStmt -> ";"

**1.3 返回语句（前置规则 1.2）**:

- Expr -> RetExpr
- RetExpr -> "return"

**1.4 函数输入（前置规则 0.1, 0.2, 1.1）**:

- Args -> Arg ("," Arg)*
- Arg -> InnerVarDecl ":" Type

**1.5 函数输出（前置规则 0.2, 1.1, 1.3, 3.1）**:

- FuncHeaderDecl -> "fn" "\<ID\>" "(" Args ")" "->" Type
- RetExpr -> "return" Expr

**2.1 变量声明语句（前置规则 0.1, 0.2, 1.2）**:

- Stmt -> VarDeclStmt
- VarDeclStmt -> "let" InnerVarDecl (":" Type)? ";"

**2.2 赋值语句（前置规则 0.3, 1.2, 3.1）**:

- Stmt -> ExprStmt
- ExprStmt -> Expr ( ";" )?
- Expr -> AssignExpr
- AssignExpr -> AssignElem "=" Expr

**2.3 变量声明赋值语句（前置规则 0.1, 0.2, 1.2, 3.1）**:

- VarDeclStmt -> "let" InnerVarDecl (":" Type)? ( "=" Expr)? ";"

**3.1 基本表达式（前置规则 0.3）**:

- Expr -> CmpExpr
- CmpExpr -> AriExpr
- AriExpr -> Item
- Item -> Factor
- Factor -> Element
- Element -> "\<NUM\>" | AssignElem | BracketExpr
- BracketExpr -> "(" Expr ")"

**3.2 表达式增加计算和比较（前置规则 3.1）**:

- CmpExpr -> CmpExpr ["<" | "<=" | ">" | ">=" | "==" | "!="] AriExpr
- AriExpr -> AriExpr [+ | -] Item
- Item -> Item [* | /] Factor

**3.3 函数调用（前置规则 3.1）**:

- Element -> CallExpr
- CallExpr -> "\<ID\>" "(" ArgList ")"
- ArgList -> Expr ("," Expr)* | $\epsilon$

**4.1 选择结构（前置规则 1.2, 3.1）**:

- Expr -> IfExpr
- IfExpr -> "if" Expr StmtBlockExpr ElseClause
- ElseClause -> "else" StmtBlockExpr | $\epsilon$

**4.2 增加 else if（前置规则 4.1）**:

- ElseClause -> "else" "if" Expr StmtBlockExpr ElseClause

**5.1 while 循环结构（前置规则 1.2, 3.1）**:

- Expr -> LoopExpr
- LoopExpr -> WhileLoopExpr
- WhileLoopExpr -> "while" Expr StmtBlockExpr

**5.2 for 循环结构（前置规则 5.1）**:

- LoopExpr -> ForLoopExpr
- ForLoopExpr -> "for" InnerVarDecl "in" Iterable
- Iterable -> Interval
- Interval -> Expr ".." Expr

**5.3 loop 循环结构（前置规则 5.1）**:

- LoopExpr -> "loop" StmtBlockExpr

**5.4 增加 break 和 continue（前置规则 5.1）**:

- Expr -> BreakExpr | ContinueExpr
- BreakExpr -> "break"
- ContinueExpr -> "continue"

**6.1 声明不可变变量（前置规则 0.2）**:

- InnerVarDecl -> "\<ID\>"

**6.2 借用和引用（前置规则 3.1, 6.1）**:

- Factor -> "&" ("mut")? Factor
- Type -> "&" ("mut")? Type

`&mut` - 可变引用；`&` - 不可变引用；`*` - 解引用

- AssignElem -> *Variable

**7.1 函数表达式块（前置规则 1.2, 3.1）**:

**7.2 函数表达式块作为函数体**:

**7.3 选择表达式（前置规则 7.1）**:

**7.4 循环表达式（前置规则 7.1）**:

- BreakExpr -> "break" Expr

**8.1 数组（前置规则 0.2, 3.1）**:

- Type -> ArrayType
- ArrayType -> "[" Type ";" "\<NUM\>" "]"
- Factor -> ArrayElems
- ArrayElems -> "[" Expr ("," Expr)* "]" | $\epsilon$

**8.2 数组元素（前置规则 8.1）**:

- AssignElem -> ELement "[" Expr "]"
- Iterable -> Element

**9.1 元组（前置规则 0.2, 3.1）**:

- Type -> TupleType
- TupleType -> "(" Type "," TypeList ")"
- TypeList -> $\epsilon$ | $Type ("," Type)*
- Factor -> TupleAssign
- TupleAssign -> "(" (Expr "," TupleElems)? ")"
- TupleElems -> $\epsilon$ | Expr ("," Expr)"

**9.2 元组元素（前置规则 8.1）**:

- AssignElem -> "\<ID\>" "." "\<NUM\>"

**实际实现的产生式**：

- InnerVarDecl -> ("mut")? "\<ID\>"

- Type -> "i32" | "bool" | ArrayType | "()" | TupleType
- ArrayType -> "[" Type ";" "\<NUM\>" "]"
- TupleType -> "(" Type "," TypeList ")"
- TypeList -> $\epsilon$ | Type ("," Type)*

- Prog -> (FuncDecl)*

- FuncDecl -> FuncHeaderDecl StmtBlockExpr

- FuncHeaderDecl -> "fn" "\<ID\>" "(" Args ")" ("->" Type)?
- Args -> $\epsilon$ | Arg ("," Arg)*
- Arg -> InnerVarDecl ":" Type

- StmtBlockExpr -> "{" (Stmt)* "}"

- Stmt -> EmptyStmt | VarDeclStmt | ExprStmt

- EmptyStmt -> ";"
- RetExpr -> "return" (Expr)?
- VarDeclStmt -> "let" InnerVarDecl (":" Type)? ("=" Expr)? ";"

- ExprStmt -> Expr ( ";" )?

- Expr -> RetExpr | BreakExpr | ContinueExpr | AssignExpr | CmpExpr | IfExpr | LoopExpr | WhileLoopExpr | ForLoopExpr | StmtBlockExpr

- AssignExpr -> AssignElem "=" Expr
- AssignElem -> Variable | ArrayAccess | TupleAccess
- ArrayAccess -> Value "[" Expr "]"
- TupleAccess -> Value "." "\<NUM\>"
- Value -> BracketExpr | CallExpr | Variable
- Variable -> "\<ID\>"
- CallExpr -> "\<ID\>" "(" ArgList ")"
- ArgList -> Expr ("," Expr)* | $\epsilon$

- CmpExpr -> (CmpExpr ["<" | "<=" | ">" | ">=" | "==" | "!="])* AddExpr
- AddExpr -> (AddExpr [+ | -])* MulExpr
- MulExpr -> (MulExpr [\* | /])* Factor

- Factor -> Element | ArrayElems | TupleAssign
- ArrayElems -> "[" (Expr)? ("," Expr)* "]"
- TupleElems -> "(" (Expr "," TupleElem)? ")"
- TupleElem -> $\epsilon$ | Expr ("," Expr)*

- Element -> "\<NUM\>" | AssignElem | Value

- BracketExpr -> "(" Expr ")"

- IfExpr -> "if" Expr StmtBlockExpr ElseClause
- ElseClause -> "else" "if" Expr StmtBlockExpr ElseClause | "else" StmtBlockExpr | $\epsilon$

- LoopExpr -> WhileLoopExpr | ForLoopExpr | "loop" StmtBlockExpr
- WhileLoopExpr -> "while" Expr StmtBlockExpr

- ForLoopExpr -> "for" InnerVarDecl "in" Iterable StmtBlockExpr
- Iterable -> Interval | IterableVal
- Interval -> Expr ".." Expr
- IterableVal -> Expr

- BreakStmt -> "break" (Expr)? ";"
- Continue -> "continue" ";"
