#include <ranges>
#include <vector>

#include "panic.hpp"
#include "ir_builder.hpp"
#include "quad_factory.hpp"

namespace ir {

template <typename... Vecs>
  requires (std::same_as<std::decay_t<Vecs>, std::vector<IRQuadPtr>> && ...)
static std::vector<IRQuadPtr>
concatIrcode(Vecs&&... vecs)
{
  return std::views::concat(std::forward<Vecs>(vecs)...)
    | std::ranges::to<std::vector>();
}

template <typename T>
concept HasIRCode = requires(const T &node) {
  { node -> ircode } -> std::convertible_to<std::vector<IRQuadPtr>>;
};

template <std::ranges::range ASTNodeRange>
  requires HasIRCode<std::ranges::range_value_t<ASTNodeRange>>
static std::vector<IRQuadPtr>
extractIrcodeAndConcat(const ASTNodeRange &nodes) {
  return nodes
    | std::views::transform([](const auto &node) {
        return node->ircode;
      })
    | std::views::join  // flatten
    | std::ranges::to<std::vector>();
}

void
IRBuilder::visit(ast::Prog &prog)
{
  prog.ircode = extractIrcodeAndConcat(prog.decls);
}

void
IRBuilder::visit(ast::FuncDecl &fdecl)
{
  std::vector<IRQuadPtr> retcode;
  if (!fdecl.body->has_ret && fdecl.header->type.type == type::TypeFactory::UNIT_TYPE) {
    // 不管函数体内是否有返回语句，对于空返回值的函数，都需要根据最后一条语句是否是返回语句
    // 额外生成一条默认返回语句！
    if (fdecl.body->ircode.empty() || fdecl.body->ircode.back()->op != IROp::RETURN) {
      retcode.push_back(QuadFactory::makeRet(fdecl.header->name));
    }
  }

  if (fdecl.body->type.type != type::TypeFactory::UNIT_TYPE) {
    // 将默认返回最后一个表达式的值的语法糖 desugar!
    auto retval_stmt = fdecl.body->stmts.back();
    auto expr_stmt = std::static_pointer_cast<ast::ExprStmt>(retval_stmt);
    CHECK(expr_stmt != nullptr, "the last expression statement in statement block isn't exist");

    retcode.push_back(QuadFactory::makeRet(expr_stmt->expr->symbol, fdecl.header->name));
  }

  fdecl.ircode = concatIrcode(
    fdecl.header->ircode,
    fdecl.body->ircode,
    retcode
  );
}

void
IRBuilder::visit(ast::FuncHeaderDecl &fhdecl)
{
  auto quad = QuadFactory::makeFunc(fhdecl.name);
  fhdecl.ircode.push_back(quad);
}

void
IRBuilder::visit(ast::Arg &arg)
{
  // nothing to do
}

void
IRBuilder::visit(ast::StmtBlockExpr &sbexpr)
{
  if (sbexpr.type.type != type::TypeFactory::UNIT_TYPE) {
    // 如果语句块表达式的类型不为 unit type
    // 则意味着这个语句块最后一个语句是一个表达式语句，且没有分号，
    // 即是一个函数表达式语句块
    auto retval_stmt = sbexpr.stmts.back();
    auto expr_stmt = std::static_pointer_cast<ast::ExprStmt>(retval_stmt);
    CHECK(expr_stmt != nullptr, "the last expression statement in statement block isn't exist");

    sbexpr.symbol = expr_stmt->expr->symbol;
  }
  sbexpr.ircode = extractIrcodeAndConcat(sbexpr.stmts);
}

void
IRBuilder::visit(ast::VarDeclStmt &vdstmt)
{
  if (auto value = vdstmt.value.value_or(nullptr); value) {
    // 只有 let (mut)? <ID> (: Type)? = Expr ; 需要生成四元式
    auto var = ctx.lookupVal(vdstmt.name).value_or(nullptr);
    ASSERT_MSG(var != nullptr, "variable didn't declared!");

    auto quad = QuadFactory::makeAssign(value->symbol, var);
    vdstmt.ircode = value->ircode;
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

  auto loopctx = ctx.getLoopCtx().value_or(nullptr);
  ASSERT_MSG(loopctx, "break not in loop context");

  codes.push_back(
    QuadFactory::makeGoto(std::format("{}_end", loopctx->name))
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
  auto loopctx = ctx.getLoopCtx().value_or(nullptr);
  ASSERT_MSG(loopctx, "break not in loop context");
  cexpr.ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_start", loopctx->name))
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
  aelem.ircode = aelem.value->ircode;
  aelem.symbol = aelem.value->symbol;
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
    aacc.value->symbol,
    aacc.idx->symbol,
    temp
  );

  aacc.ircode = concatIrcode(
    aacc.value->ircode,
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
    tacc.value->symbol,
    tacc.idx->symbol,
    temp
  );

  tacc.ircode = concatIrcode(
    tacc.value->ircode,
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

  auto param_code = cexpr.argv
    | std::views::transform([](const auto &arg) {
        return QuadFactory::makeParam(arg->symbol);
      })
    | std::ranges::to<std::vector>();

  auto temp = ctx.produceTemp(cexpr.pos, cexpr.type.type);
  cexpr.symbol = temp;

  auto quad = QuadFactory::makeCall(cexpr.callee, temp);

  cexpr.ircode = concatIrcode(
    std::move(codes),
    std::move(param_code),
    std::vector{quad}
  );
}

static void
makeCondAndInsert(std::vector<IRQuadPtr> &ircode, const ast::ExprPtr &cond, std::string label)
{
  auto cond_code = cond->ircode;
  cond_code.push_back(QuadFactory::makeBnez(cond->symbol, std::move(label)));

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
  std::string curctxname = ctx.getCurCtxName();
  makeCondAndInsert(
    iexpr.body->ircode,
    iexpr.cond,
    std::format("{}_end", curctxname)
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
    QuadFactory::makeGoto(std::format("{}_final", curctxname))
  );

  insertLabels(iexpr.body->ircode, curctxname);

  auto else_codes = extractIrcodeAndConcat(iexpr.elses);
  iexpr.ircode = concatIrcode(iexpr.body->ircode, else_codes);
  iexpr.ircode.push_back(
    QuadFactory::makeLabel(std::format("{}_final", curctxname))
  );
}

void
IRBuilder::visit(ast::ElseClause &eclause)
{
  std::string curctxname = ctx.getCurCtxName();
  if (const auto &cond = eclause.cond.value_or(nullptr); cond) {
    makeCondAndInsert(
      eclause.body->ircode,
      cond,
      std::format("{}_end", curctxname)
    );
  }

  auto opt_ifscope = ctx.getIfScope();
  ASSERT_MSG(opt_ifscope.has_value(), "not in if context");
  auto ifscope = opt_ifscope.value();

  if (eclause.body->type.type != type::TypeFactory::UNIT_TYPE) {
    auto opt_ifval = ifscope.val;
    ASSERT_MSG(opt_ifval.has_value(), "if expression don't have temp value");
    auto ifval = opt_ifval.value();

    eclause.body->ircode.push_back(
      QuadFactory::makeAssign(eclause.body->symbol, ifval)
    );
  }
  eclause.body->ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_final", ifscope.name))
  );

  pushbackLabel(eclause.body->ircode, curctxname);

  eclause.ircode = eclause.body->ircode;
}

void
IRBuilder::visit(ast::WhileLoopExpr&wlexpr)
{
  auto cond_code = wlexpr.cond->ircode;
  std::string curctxname = ctx.getCurCtxName();
  cond_code.push_back(
    QuadFactory::makeBnez(
      wlexpr.cond->symbol,
      std::format("{}_end", curctxname)
    )
  );

  wlexpr.ircode = concatIrcode(cond_code, wlexpr.body->ircode);
  wlexpr.ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_start", curctxname))
  );
  insertLabels(wlexpr.ircode, curctxname);
}

void
IRBuilder::visit(ast::ForLoopExpr &flexpr)
{
  flexpr.ircode = concatIrcode(flexpr.iter->ircode, flexpr.body->ircode);

  auto curforscope = ctx.getCurScope();
  CHECK(curforscope.val.has_value(), "for loop iterator didn't declared");

  flexpr.ircode.push_back(
    QuadFactory::makeGoto(std::format("{}_start", curforscope.name))
  );
  pushbackLabel(flexpr.ircode, curforscope.name);
}

void
IRBuilder::visit(ast::Interval &interval)
{
  auto codes = concatIrcode(
    interval.start->ircode,
    interval.end->ircode
  );

  auto curforscope = ctx.getCurScope();
  CHECK(curforscope.val.has_value(), "for loop iterator didn't declared");
  auto iter = curforscope.val.value();

  auto one = ctx.declareConst(1, interval.pos);
  codes.push_back(
    QuadFactory::makeOperation(
      IROp::SUB,
      interval.start->symbol,
      one,
      iter
    )
  );

  codes.push_back(
    QuadFactory::makeLabel(std::format("{}_start", curforscope.name))
  );

  auto temp = ctx.produceTemp(interval.pos, type::TypeFactory::INT_TYPE);
  codes.push_back(
    QuadFactory::makeOperation(IROp::ADD, iter, one, temp)
  );
  codes.push_back(
    QuadFactory::makeAssign(temp, iter)
  );

  codes.push_back(
    QuadFactory::makeBlt(
      iter,
      interval.end->symbol,
      std::format("{}_end", curforscope.name)
    )
  );

  interval.ircode = std::move(codes);
}

void
IRBuilder::visit(ast::IterableVal &iter)
{
  auto codes = iter.value->ircode;

  auto curforscope = ctx.getCurScope();
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
    QuadFactory::makeLabel(std::format("{}_start", curforscope.name))
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
    QuadFactory::makeBlt(
      temp2,
      size,
      std::format("{}_end", curforscope.name)
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

  std::string label = ctx.getCurCtxName();

  codes.push_back(
    QuadFactory::makeGoto(std::format("{}_start", label))
  );
  insertLabels(codes, label);

  lexpr.ircode = std::move(codes);
}

} // namespace ir
