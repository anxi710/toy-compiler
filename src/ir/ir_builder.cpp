/**
 * @file ir_builder.cpp
 * @brief Implementation of the IRBuilder class for
 *        generating intermediate representation (IR) code from AST nodes.
 *
 * This file contains the implementation of the IRBuilder class,
 * which traverses the abstract syntax tree (AST)
 * and generates a flat vector of IR quads for each node.
 * The IRBuilder uses modern C++ features such as ranges,
 * concepts, and perfect forwarding to efficiently concatenate
 * and transform IR code sequences.
 *
 * Key Features:
 * - Provides utility functions for concatenating multiple IR code vectors
 *   and extracting IR code from AST node ranges.
 * - Implements visit methods for various AST node types,
 *   handling code generation for functions, statements, expressions,
 *   control flow constructs (if, while, for, loop),
 *   and data structures (arrays, tuples).
 * - Handles desugaring of language constructs,
 *   such as implicit returns and block expressions.
 * - Ensures type safety and efficient IR code construction
 *   using C++20 concepts and ranges.
 */
#include <ranges>
#include <vector>

#include "ast.hpp"
#include "panic.hpp"
#include "ir_builder.hpp"
#include "quad_factory.hpp"
#include "semantic_context.hpp"

namespace ir {

IRBuilder::IRBuilder(sem::SemanticContext &ctx) : ctx(ctx) {}

/**
 * @brief 拼接多个 IR 代码序列为一个扁平的 IR 代码向量
 *
 * 本函数接受任意个 std::vector<IRQuadPtr> 类型的参数（支持左值、右值、临时对象），
 * 并将它们按顺序拼接为一个新的 vector，返回该扁平化的结果。
 *
 * @tparam Vecs 任意多个 std::vector<IRQuadPtr> 类型的参数（可以为左值或右值）
 * @param  vecs 要拼接的 IR 代码序列容器列表
 * @return 拼接后的 std::vector<IRQuadPtr>
 *
 * @note
 * - 使用了变参模板 + concepts + decay，确保传入的每个参数类型为 std::vector<IRQuadPtr>。
 * - 利用 ranges::views::concat 进行懒惰拼接，并使用 ranges::to 生成最终 vector。
 * - 相比手动 insert，更简洁、类型安全，支持完美转发。
 *
 * 示例：
 *   auto result = concatIrcode(code1, std::move(code2), std::vector<IRQuadPtr>{...});
 */
template <typename... Vecs>
  // decay 可以将 Vecs 的所有 const, &, volatile 等修饰符剥去
  // 这样可以让本函数接受左值、右值、const、临时变量等不同类型的 vector 参数，
  // 提高了接口的灵活性和泛用性，便于在不同上下文中高效拼接 IR 代码序列。
requires (std::same_as<std::decay_t<Vecs>, std::vector<IRQuadPtr>> && ...)
static std::vector<IRQuadPtr>
concatIrcode(Vecs&&... vecs)
{
  // 变长参数模板中如何对参数进行展开是一个非常关键的问题，
  // 这里使用折叠表达式（左折叠）对变长参数进行展开
  return std::views::concat(std::forward<Vecs>(vecs)...)
    | std::ranges::to<std::vector>();
}

/**
 * @brief Concept to check if a type provides IR code generation.
 *
 * This concept is satisfied if the given type `T` has a member `ircode`
 * accessible via pointer dereference (`node -> ircode`), and the type of
 * `ircode` is convertible to `std::vector<IRQuadPtr>`.
 *
 * @tparam T The type to check for IR code generation capability.
 */
template <typename T>
concept HasIRCode = requires(const T &node) {
  { node->ircode } -> std::convertible_to<std::vector<IRQuadPtr>>;
};

/**
 * @brief Extracts and concatenates IR code from a range of AST nodes.
 *
 * This function takes a range of AST nodes, each of which must have an `ircode` member
 * (as required by the `HasIRCode` concept). It transforms the range by extracting the
 * `ircode` from each node, flattens the resulting range of IR code sequences, and
 * collects all IR quads into a single `std::vector<IRQuadPtr>`.
 *
 * @tparam ASTNodeRange A range type whose value type satisfies the `HasIRCode` concept.
 * @param nodes The range of AST nodes to process.
 * @return std::vector<IRQuadPtr> A vector containing all IR quads from all nodes, concatenated.
 */
template <std::ranges::range ASTNodeRange>
  requires HasIRCode<std::ranges::range_value_t<ASTNodeRange>>
static std::vector<IRQuadPtr>
extractIrcodeAndConcat(const ASTNodeRange &nodes) {
  return nodes
    | std::views::transform([](const auto &node) {
        return node->ircode;
      })                              // transform
    | std::views::join                // flatten
    | std::ranges::to<std::vector>(); // collect
}

/**
 * @brief Visits a prog AST node and generates its IR code.
 *
 * @param prog Reference to the prog AST node to visit.
 */
void
IRBuilder::visit(ast::Prog &prog)
{
  prog.ircode = extractIrcodeAndConcat(prog.decls);
}

/**
 * @brief Visits a function declaration AST node and generates its IR code.
 *
 * This method processes an `ast::FuncDecl` node,
 * handling the generation of IR code for the function.
 *
 * - If the function's return type is `unit` and there is no
 *   explicit return statement in the body, it automatically
 *   inserts a return instruction at the end.
 * - If the function's return type is not `unit`,
 *   it desugars the implicit return of the last expression
 *   in the function body by generating a return instruction for its value.
 * - The final IR code for the function is constructed by
 *   concatenating the IR code from the function header,
 *   the function body, and any generated return instructions.
 *
 * @param fdecl Reference to the function declaration AST node to visit.
 */
void
IRBuilder::visit(ast::FuncDecl &fdecl)
{
  std::vector<IRQuadPtr> retcode;
  if (!fdecl.body->has_ret
    && fdecl.header->type.type == type::TypeFactory::UNIT_TYPE)
  {
    // 如果函数体内没有返回语句，且返回类型为 ()，则自动生成一条 return 语句
    // NOTE: 在 semantic checker 中，语句块中有返回语句指的是语句串中有返回语句，
    //       或有路径覆盖的 if 语句/loop 语句。这个意义上来讲，并不是递归检查到
    //       一条返回语句就认为有返回语句，而是路径覆盖意义上的有返回语句！
    if (fdecl.body->ircode.empty()
      || fdecl.body->ircode.back()->op != IROp::RETURN)
    {
      // 函数体可能为空！
      retcode.push_back(
        QuadFactory::makeRet(fdecl.header->name)
      );
    }
  }

  if (fdecl.body->type.type != type::TypeFactory::UNIT_TYPE) {
    // 将默认返回最后一个表达式的值的语法糖 desugar!
    auto laststmt = fdecl.body->stmts.back();

    // 如果函数体的类型不为 unit，则最后一个语句一定是一个表达式
    auto exprstmt = std::static_pointer_cast<ast::ExprStmt>(laststmt);
    CHECK(
      exprstmt != nullptr,
      "the last statement isn't an expression"
    );

    retcode.push_back(
      QuadFactory::makeRet(exprstmt->expr->symbol, fdecl.header->name)
    );
  }

  fdecl.ircode = concatIrcode(
    fdecl.header->ircode,
    fdecl.body->ircode,
    retcode
  );
}

/**
 * @brief Visits a function header declaration AST node
 *        and generates its IR code.
 *
 * This method creates an IR quad representing
 * the function declaration using the function's name,
 * and appends it to the IR code list of
 * the given function header declaration node.
 *
 * @param fhdecl Reference to the function header declaration AST node to process.
 */
void
IRBuilder::visit(ast::FuncHeaderDecl &fhdecl)
{
  auto quad = QuadFactory::makeFunc(fhdecl.name);
  fhdecl.ircode.push_back(quad);
}

/**
 * @brief Visits a statement block expression node and processes its IR code.
 *
 * This method handles the case where the statement block expression (`StmtBlockExpr`)
 * is not of unit type. In such cases, it ensures that the block is not empty and that
 * the last statement is an expression statement. It then propagates the symbol from
 * the last expression to the block expression itself.
 *
 * Finally, it extracts and concatenates the IR code from all statements in the block,
 * assigning the result to the block expression's `ircode` field.
 *
 * @param sbexpr The statement block expression AST node to visit.
 */
void
IRBuilder::visit(ast::StmtBlockExpr &sbexpr)
{
  if (sbexpr.type.type != type::TypeFactory::UNIT_TYPE) {
    // 如果语句块表达式的类型不为 unit type
    // 则意味着这个语句块最后一个语句是一个表达式
    CHECK(
      !sbexpr.stmts.empty(),
      "the statement block is empty, but it's type isn't unit type"
    );
    auto laststmt = sbexpr.stmts.back();
    auto exprstmt = std::static_pointer_cast<ast::ExprStmt>(laststmt);
    CHECK(
      exprstmt != nullptr,
      "the last statement isn't an expression"
    );

    sbexpr.symbol = exprstmt->expr->symbol;
  }

  sbexpr.ircode = extractIrcodeAndConcat(sbexpr.stmts);
}

/**
 * @brief Visits a variable declaration statement and generates corresponding IR code.
 *
 * This method handles variable declarations of the form:
 *   let (mut)? <ID> (: Type)? = Expr;
 * If the variable declaration includes an initializer expression (`rval`), it:
 *   - Looks up the variable in the current context.
 *   - Asserts that the variable has been declared.
 *   - Creates an assignment quad (intermediate representation) from the initializer's symbol to the variable.
 *   - Appends the generatedo the variable declaration statement AST node.
 */
void
IRBuilder::visit(ast::VarDeclStmt &vdstmt)
{
  // 只有 let (mut)? <ID> (: Type)? = Expr ; 需要生成四元式
  if (auto rval = vdstmt.rval.value_or(nullptr); rval) {
    auto var = ctx.lookupVal(vdstmt.name).value_or(nullptr);
    ASSERT_MSG(var != nullptr, "variable didn't declared!");

    auto quad = QuadFactory::makeAssign(rval->symbol, var);
    vdstmt.ircode = rval->ircode;
    vdstmt.ircode.push_back(quad);
  }
}

void
IRBuilder::visit(ast::ExprStmt &estmt)
{
  estmt.ircode = estmt.expr->ircode;
}

void
IRBuilder::visit(ast::RetExpr &rexpr)
{
  std::vector<IRQuadPtr> code;
  if (auto retval = rexpr.retval.value_or(nullptr); retval) {
    code = retval->ircode;
    code.push_back(
      QuadFactory::makeRet(retval->symbol, ctx.getCurFuncName())
    );
  } else {
    code.push_back(
      QuadFactory::makeRet(ctx.getCurFuncName())
    );
  }
  rexpr.ircode = std::move(code);
}

void
IRBuilder::visit(ast::BreakExpr &bexpr)
{
  // 在 IR 层面上，直接将 BreakExpr desugar 为 optional<assign> 和 goto
  std::vector<IRQuadPtr> codes;
  auto retval = bexpr.value.value_or(nullptr);
  if (retval) {
    auto dst = bexpr.dst.value_or(nullptr);
    ASSERT_MSG(dst.get(), "dst didn't exist (break expression)");
    codes.push_back(QuadFactory::makeAssign(retval->symbol, dst));
  }

  auto *loopctx = ctx.getLoopCtx().value_or(nullptr);
  ASSERT_MSG(loopctx, "break not in loop context");

  std::string curfuncname = ctx.getCurFuncName();
  std::string prefix = std::format("{}_{}", curfuncname, loopctx->name);
  codes.push_back(
    QuadFactory::makeGoto(std::format("{}_end", prefix))
  );

  if (retval) {
    bexpr.ircode = concatIrcode(retval->ircode, std::move(codes));
  } else {
    bexpr.ircode = std::move(codes);
  }
}

void
IRBuilder::visit(ast::ContinueExpr &cexpr)
{
  auto *loopctx = ctx.getLoopCtx().value_or(nullptr);
  ASSERT_MSG(loopctx, "continue not in loop context");

  std::string curfuncname = ctx.getCurFuncName();
  std::string prefix = std::format("{}_{}", curfuncname, loopctx->name);
  cexpr.ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_start", prefix))
  );
}

void
IRBuilder::visit(ast::AssignExpr &aexpr)
{
  auto quad = QuadFactory::makeAssign(
    aexpr.rval->symbol,
    aexpr.lval->symbol
  );
  aexpr.ircode = concatIrcode(
    aexpr.lval->ircode,
    aexpr.rval->ircode,
    std::vector{quad}
  );
}

void
IRBuilder::visit(ast::AssignElem &aelem)
{
  aelem.ircode = aelem.base->ircode;
  aelem.symbol = aelem.base->symbol;
}

void
IRBuilder::visit(ast::Variable &var)
{
  // nothing to do
}

void
IRBuilder::visit(ast::ArrAcc &aacc)
{
  sym::TempPtr temp = ctx.produceTemp(aacc.pos, aacc.type.type);
  aacc.symbol = temp;

  auto quad = QuadFactory::makeAcc(
    IROp::INDEX,
    aacc.base->symbol,
    aacc.idx->symbol,
    temp
  );

  aacc.ircode = concatIrcode(
    aacc.base->ircode,
    aacc.idx->ircode,
    std::vector{quad}
  );
}

void
IRBuilder::visit(ast::TupAcc &tacc)
{
  sym::TempPtr temp = ctx.produceTemp(tacc.pos, tacc.type.type);
  tacc.symbol = temp;

  auto quad = QuadFactory::makeAcc(
    IROp::DOT,
    tacc.base->symbol,
    tacc.idx->symbol,
    temp
  );

  tacc.ircode = concatIrcode(
    tacc.base->ircode,
    tacc.idx->ircode,
    std::vector{quad}
  );
}

static IROp
cmpOp2IROp(ast::CmpOper op)
{
  switch(op) {
    case ast::CmpOper::EQ:  return IROp::EQ;
    case ast::CmpOper::NEQ: return IROp::NEQ;
    case ast::CmpOper::GT:  return IROp::GT;
    case ast::CmpOper::GEQ: return IROp::GEQ;
    case ast::CmpOper::LT:  return IROp::LT;
    case ast::CmpOper::LEQ: return IROp::LEQ;
  }
}

void
IRBuilder::visit(ast::CmpExpr &cexpr)
{
  sym::TempPtr temp = ctx.produceTemp(cexpr.pos, cexpr.type.type);
  cexpr.symbol = temp;

  auto quad = QuadFactory::makeOperation(
    cmpOp2IROp(cexpr.op),
    cexpr.lhs->symbol,
    cexpr.rhs->symbol,
    temp
  );

  cexpr.ircode = concatIrcode(
    cexpr.lhs->ircode,
    cexpr.rhs->ircode,
    std::vector{quad}
  );
}

static IROp
ariOp2IROp(ast::AriOper op)
{
  switch(op) {
    case ast::AriOper::ADD: return IROp::ADD;
    case ast::AriOper::SUB: return IROp::SUB;
    case ast::AriOper::MUL: return IROp::MUL;
    case ast::AriOper::DIV: return IROp::DIV;
  }
}

void
IRBuilder::visit(ast::AriExpr &aexpr)
{
  sym::TempPtr temp = ctx.produceTemp(aexpr.pos, aexpr.type.type);
  aexpr.symbol = temp;

  auto quad = QuadFactory::makeOperation(
    ariOp2IROp(aexpr.op),
    aexpr.lhs->symbol,
    aexpr.rhs->symbol,
    temp
  );

  aexpr.ircode = concatIrcode(
    aexpr.lhs->ircode,
    aexpr.rhs->ircode,
    std::vector{quad}
  );
}

static auto
extractSymbol(const std::vector<ast::ExprPtr> &elems)
{
  return elems
    | std::views::transform([](const auto &elem) {
        return Operand{elem->symbol};
      })
    | std::ranges::to<std::vector>();
}

void
IRBuilder::visit(ast::ArrElems &aelems)
{
  auto codes = extractIrcodeAndConcat(aelems.elems);
  auto elems = extractSymbol(aelems.elems);

  sym::TempPtr temp = ctx.produceTemp(aelems.pos, aelems.type.type);
  aelems.symbol = temp;

  auto quad = QuadFactory::makeElems(IROp::MAKE_ARR, elems, temp);
  codes.push_back(quad);

  aelems.ircode = std::move(codes);
}

void
IRBuilder::visit(ast::TupElems &telems)
{
  auto codes = extractIrcodeAndConcat(telems.elems);
  auto elems = extractSymbol(telems.elems);

  sym::TempPtr temp = ctx.produceTemp(telems.pos, telems.type.type);
  telems.symbol = temp;

  auto quad = QuadFactory::makeElems(IROp::MAKE_TUP, elems, temp);
  codes.push_back(quad);

  telems.ircode = std::move(codes);
}

void
IRBuilder::visit(ast::Number &num)
{
  // nothing to do
}

void
IRBuilder::visit(ast::BracketExpr &bexpr)
{
  if (auto expr = bexpr.expr.value_or(nullptr); expr) {
    bexpr.symbol = expr->symbol;
    bexpr.ircode = expr->ircode;
  }
}

void
IRBuilder::visit(ast::CallExpr &cexpr)
{
  auto codes = extractIrcodeAndConcat(cexpr.argv);

  auto params = cexpr.argv
    | std::views::transform([](const auto &arg) {
        return Operand{arg->symbol};
      })
    | std::ranges::to<std::vector>();

  auto temp = ctx.produceTemp(cexpr.pos, cexpr.type.type);
  cexpr.symbol = temp;

  auto quad = QuadFactory::makeCall(cexpr.callee, params, temp);

  cexpr.ircode = concatIrcode(
    std::move(codes),
    std::vector{quad}
  );
}

static void
makeCondAndInsert(std::vector<IRQuadPtr> &ircode, const ast::ExprPtr &cond, std::string label)
{
  auto cond_code = cond->ircode;
  cond_code.push_back(QuadFactory::makeBeqz(cond->symbol, std::move(label)));

  ircode.insert(
    ircode.begin(),
    std::make_move_iterator(cond_code.begin()),
    std::make_move_iterator(cond_code.end())
  );
}

static void
pushbackLabel(std::vector<IRQuadPtr> &ircode, const std::string &base)
{
  ircode.push_back(
    QuadFactory::makeLabel(std::format("{}_end", base))
  );
}

static void
insertLabels(std::vector<IRQuadPtr> &ircode, const std::string &base)
{
  ircode.insert(
    ircode.begin(),
    QuadFactory::makeLabel(std::format("{}_start", base))
  );
  ircode.push_back(
    QuadFactory::makeLabel(std::format("{}_end", base))
  );
}

void
IRBuilder::visit(ast::IfExpr &iexpr)
{
  std::string curfuncname = ctx.getCurFuncName();
  std::string curctxname = ctx.getCurCtxName();
  std::string prefix = std::format("{}_{}", curfuncname, curctxname);
  makeCondAndInsert(
    iexpr.body->ircode,
    iexpr.cond,
    std::format("{}_end", prefix)
  );

  if (iexpr.body->type.type != type::TypeFactory::UNIT_TYPE) {
    iexpr.body->ircode.push_back(
      QuadFactory::makeAssign(
        iexpr.body->symbol,
        iexpr.symbol
      )
    );
  }
  iexpr.body->ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_final", prefix))
  );

  insertLabels(iexpr.body->ircode, prefix);

  auto else_codes = extractIrcodeAndConcat(iexpr.elses);
  iexpr.ircode = concatIrcode(iexpr.body->ircode, else_codes);
  iexpr.ircode.push_back(
    QuadFactory::makeLabel(std::format("{}_final", prefix))
  );
}

void
IRBuilder::visit(ast::ElseClause &eclause)
{
  std::string curfuncname = ctx.getCurFuncName();
  std::string curctxname = ctx.getCurCtxName();
  std::string prefix = std::format("{}_{}", curfuncname, curctxname);
  if (const auto &cond = eclause.cond.value_or(nullptr); cond) {
    makeCondAndInsert(
      eclause.body->ircode,
      cond,
      std::format("{}_end", prefix)
    );
  }

  auto opt_ifscope = ctx.getIfScope();
  ASSERT_MSG(opt_ifscope.has_value(), "not in if context");
  auto ifscope = opt_ifscope.value();

  if (eclause.body->type.type != type::TypeFactory::UNIT_TYPE) {
    auto opt_ifval = ifscope.val;
    ASSERT_MSG(opt_ifval.has_value(), "if expression don't have temp value");
    const auto &ifval = opt_ifval.value();

    eclause.body->ircode.push_back(
      QuadFactory::makeAssign(eclause.body->symbol, ifval)
    );
  }
  eclause.body->ircode.push_back(
    QuadFactory::makeGoto(
      std::format("{}_{}_final", curfuncname, ifscope.name)
    )
  );

  pushbackLabel(eclause.body->ircode, prefix);

  eclause.ircode = eclause.body->ircode;
}

void
IRBuilder::visit(ast::WhileLoopExpr&wlexpr)
{
  auto cond_code = wlexpr.cond->ircode;
  std::string curfuncname = ctx.getCurFuncName();
  std::string curctxname = ctx.getCurCtxName();
  std::string prefix = std::format("{}_{}", curfuncname, curctxname);
  cond_code.push_back(
    QuadFactory::makeBeqz(
      wlexpr.cond->symbol,
      std::format("{}_end", prefix)
    )
  );

  wlexpr.ircode = concatIrcode(cond_code, wlexpr.body->ircode);
  wlexpr.ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_start", prefix))
  );
  insertLabels(wlexpr.ircode, prefix);
}

void
IRBuilder::visit(ast::ForLoopExpr &flexpr)
{
  flexpr.ircode = concatIrcode(flexpr.iterexpr->ircode, flexpr.body->ircode);

  auto curforscope = ctx.getCurScope();
  CHECK(curforscope.val.has_value(), "for loop iterator didn't declared");

  std::string curfuncname = ctx.getCurFuncName();
  std::string prefix = std::format("{}_{}", curfuncname, curforscope.name);
  flexpr.ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_start", prefix))
  );
  pushbackLabel(flexpr.ircode, prefix);
}

/**
 * @brief Generates intermediate representation (IR) code for an range expression AST node,
 *        typically used in for-loop constructs.
 *
 * This method constructs the IR code sequence for iterating over an range expression,
 * handling initialization, loop variable increment, and loop termination condition.
 * It assumes that the loop iterator has already been declared in the current scope.
 *
 * The generated IR performs the following steps:
 * 1. Concatenates the IR code for the range expression's start and end expressions.
 * 2. Initializes the loop iterator to (start - 1).
 * 3. Emits a label marking the start of the loop.
 * 4. Increments the iterator by 1 and assigns it back.
 * 5. Emits a conditional branch to the loop end
 *    if the iterator is less than the range expression's end.
 * 6. Stores the resulting IR code in the range exprssion node.
 *
 * @param range_expr Reference to the AST range expression node to process.
 */
void
IRBuilder::visit(ast::RangeExpr &range_expr)
{
  auto codes = concatIrcode(
    range_expr.start->ircode,
    range_expr.end->ircode
  );

  auto curforscope = ctx.getCurScope();
  std::string curfuncname = ctx.getCurFuncName();
  std::string prefix = std::format("{}_{}", curfuncname, curforscope.name);
  CHECK(curforscope.val.has_value(), "for loop iterator didn't declared");
  auto iter = curforscope.val.value();

  auto one = ctx.declareConst(1, range_expr.pos);
  codes.push_back(
    QuadFactory::makeOperation(
      IROp::SUB,
      range_expr.start->symbol,
      one,
      iter
    )
  );

  codes.push_back(
    QuadFactory::makeLabel(std::format("{}_start", prefix))
  );

  auto temp = ctx.produceTemp(range_expr.pos, type::TypeFactory::INT_TYPE);
  codes.push_back(
    QuadFactory::makeOperation(IROp::ADD, iter, one, temp)
  );
  codes.push_back(
    QuadFactory::makeAssign(temp, iter)
  );

  codes.push_back(
    QuadFactory::makeBge(
      iter,
      range_expr.end->symbol,
      std::format("{}_end", prefix)
    )
  );

  range_expr.ircode = std::move(codes);
}

/**
 * @brief Generates intermediate representation (IR) code for an iterable value in a for loop.
 *
 * This method processes an AST node representing an iterable value (`ast::IterableVal`)
 * and generates the corresponding IR code for iterating over it. It handles the setup
 * of loop variables, index incrementation, bounds checking, and element access.
 *
 * The generated IR code includes:
 * - Initialization of the loop index to -1.
 * - Loop start label.
 * - Incrementing the loop index.
 * - Bounds checking to determine loop termination.
 * - Accessing the current element in the iterable.
 *
 * @param iter Reference to the AST node representing the iterable value.
 */
void
IRBuilder::visit(ast::IterableVal &iter)
{
  auto codes = iter.value->ircode;

  auto curforscope = ctx.getCurScope();
  std::string curfuncname = ctx.getCurFuncName();
  std::string prefix = std::format("{}_{}", curfuncname, curforscope.name);

  CHECK(curforscope.val.has_value(), "for loop iterator didn't declared");
  auto for_it = curforscope.val.value();

  auto one = ctx.declareConst(1, iter.pos);
  auto negone = ctx.declareConst(-1, iter.pos);
  auto size = ctx.declareConst(iter.type.type->size(), iter.pos);

  auto temp1 = ctx.produceTemp(iter.pos, type::TypeFactory::INT_TYPE);
  codes.push_back(
    QuadFactory::makeAssign(negone, temp1)
  );

  codes.push_back(
    QuadFactory::makeLabel(std::format("{}_start", prefix))
  );

  auto temp2 = ctx.produceTemp(iter.pos, type::TypeFactory::INT_TYPE);
  codes.push_back(
    QuadFactory::makeOperation(
      IROp::ADD,
      temp1,
      one,
      temp2
    )
  );

  codes.push_back(
    QuadFactory::makeBge(
      temp2,
      size,
      std::format("{}_end", prefix)
    )
  );

  codes.push_back(
    QuadFactory::makeOperation(
      IROp::INDEX,
      iter.symbol,
      temp2,
      for_it
    )
  );

  iter.ircode = std::move(codes);
}

void
IRBuilder::visit(ast::LoopExpr&lexpr)
{
  auto codes = lexpr.body->ircode;

  std::string curctxname = ctx.getCurCtxName();
  std::string curfuncname = ctx.getCurFuncName();
  std::string prefix = std::format("{}_{}", curfuncname, curctxname);

  codes.push_back(
    QuadFactory::makeGoto(std::format("{}_start", prefix))
  );
  insertLabels(codes, prefix);

  lexpr.ircode = std::move(codes);
}

} // namespace ir
