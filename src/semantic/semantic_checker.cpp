/**
 * @file semantic_checker.cpp
 * @brief Implements semantic analysis and type checking for AST nodes in the toy compiler.
 *
 * This file contains the implementation of the SemanticChecker class and its associated
 * visit methods for various AST node types. The semantic checker is responsible for:
 *   - Ensuring variables are declared and initialized before use.
 *   - Enforcing type correctness for expressions, assignments, function calls, and control flow.
 *   - Checking for correct usage of control flow constructs (if, while, for, loop, break, continue).
 *   - Handling type inference for variables and expressions.
 *   - Reporting semantic errors with precise diagnostic information.
 *
 * Each visit method implements the semantic rules for a specific AST node type, as described
 * in the comments within each function. The checker uses a context object to track scope,
 * variable mutability, initialization state, and function/loop context.
 *
 * Error reporting is performed via the reporter object, which provides detailed messages
 * including error type, description, source position, and scope information.
 *
 * @note The file assumes that AST nodes and type system utilities are defined elsewhere.
 * @author anxi
 */
#include <ranges>
#include <cassert>

#include "panic.hpp"
#include "err_report.hpp"
#include "type_factory.hpp"
#include "break_checker.hpp"
#include "return_checker.hpp"
#include "semantic_checker.hpp"

namespace sem {

// NOTE: 各函数执行相应 AST 结点的语义检查逻辑
//       具体的检查目标在函数内部的注释中有明确说明
//       因此不再编写函数整体的注释

void
SemanticChecker::checkInit(ast::Expr &expr)
{
  if (expr.symbol == nullptr) {
    return;
  }

  if (!expr.symbol->init) {
    reporter.report(
      err::SemErrType::UNINITIALIZED_VAR,
      "变量在初始化之前无法使用！",
      expr.pos,
      ctx.getCurScopeName()
    );
  }
}

void
SemanticChecker::checkCondition(ast::Expr &cond, util::Position pos)
{
  if (cond.type != ast::Type{type::TypeFactory::BOOL_TYPE}) {
    reporter.report(
      err::SemErrType::TYPE_MISMATCH,
      std::format(
        "{} 无法用于逻辑判断（只有 bool 可以用于逻辑判断）",
        cond.type.str()
      ),
      pos,
      ctx.getCurScopeName()
    );
  } // end if
}

static ast::Type
inferType(const ast::Type &type)
{
  // 注意：这里不能使用 ast::Type 封装的 type::TypePtr 的比较逻辑
  // 因为这里判断的是裸指针是否相等！
  if (type.type == type::TypeFactory::ANY_TYPE) {
    // ANY_TYPE 在进行推导时要被推导为 unit type!!!
    return ast::Type{type::TypeFactory::UNIT_TYPE};
  }
  return type;
}

void
SemanticChecker::visit(ast::FuncDecl &fdecl)
{
  // 检查函数体中是否有 return 表达式
  if (!fdecl.body->has_ret
    && fdecl.header->type.type != type::TypeFactory::UNIT_TYPE
    && fdecl.body->type.type == type::TypeFactory::UNIT_TYPE)
  {
    // 如果函数体中没有 return（在函数体中语句的层面上）且函数体的最后一个语句不是默认返回的表达式
    reporter.report(
      err::SemErrType::MISSING_RETVAL,
      std::format(
        "函数 {} 需要一个类型为 {} 的返回值",
        fdecl.header->name,
        fdecl.header->type.str()
      ),
      fdecl.pos,
      ctx.getCurScopeName()
    );
  }

  ctx.resetTempCnt();
}

void
SemanticChecker::visit(ast::FuncHeaderDecl &fhdecl)
{
  // 设置返回值类型
  ctx.setRetValType(fhdecl.type.type);
}

void
SemanticChecker::visit(ast::Arg &arg)
{
  // 声明形参
  ctx.declareArg(arg.name, arg.mut, arg.type.type, arg.pos);
}

void
SemanticChecker::visit(ast::StmtBlockExpr &sbexpr)
{
  // 检查语句块中是否有 return 表达式
  ReturnChecker rchecker;
  sbexpr.accept(rchecker);
  sbexpr.has_ret = rchecker.has_ret;
  sbexpr.res_mut = false;

  if (sbexpr.stmts.empty()) {
    sbexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  } else {
    sbexpr.type = inferType(sbexpr.stmts.back()->type);
  }

  // 检查是否有未推导出类型的变量
  auto failed_vars = ctx.checkAutoTypeInfer();
  for (const auto &var : failed_vars) {
    reporter.report(
      err::SemErrType::TYPE_INFER_FAILURE,
      std::format("变量 '{}' 无法通过自动推导确定类型", var->name),
      var->pos,
      ctx.getCurScopeName()
    );
  }
}

void
SemanticChecker::visit(ast::EmptyStmt &estmt)
{
  estmt.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::VarDeclStmt &vdstmt)
{
  auto rval = vdstmt.rval.value_or(nullptr);
  if (vdstmt.vartype.type != type::TypeFactory::UNKNOWN_TYPE) {
    // let (mut)? <ID> : Type ;
    if (rval && vdstmt.vartype != rval->type) {
      // let (mut)? <ID> : Type = Expr ;
      reporter.report(
        err::SemErrType::TYPE_MISMATCH,
        std::format(
          "表达式的类型 {} 和变量 {} 声明的类型 {} 不一致",
          rval->type.str(),
          vdstmt.name,
          vdstmt.vartype.str()
        ),
        rval->pos,
        ctx.getCurScopeName()
      );
    }
  } else if (rval) {
    // let (mut)? <ID> = Expr ;
    // Type Inference
    vdstmt.vartype = inferType(rval->type);
  }

  bool init = vdstmt.rval.has_value();
  ctx.declareVar(vdstmt.name, vdstmt.mut, init, vdstmt.vartype.type, vdstmt.pos);

  if (rval && rval->is_var) {
    // 如果用于初始化的表达式是一个变量，则去检查该变量是否已经初始化
    checkInit(*rval);
  }

  vdstmt.type = ast::Type{type::TypeFactory::UNIT_TYPE}; // 变量声明语句使用的是副作用！！！
}

void
SemanticChecker::visit(ast::ExprStmt &estmt)
{
  if (estmt.expr->is_var) {
    // 即使表达式语句中的表达式是一个变量（如：a; ），也需要检查该变量是否已经初始化
    checkInit(*estmt.expr);
  }

  if (estmt.is_last) {
    // 如果是语句块中的最后一个 expression statement
    // 则这个表达式语句一定是用作表达式
    //   Tips: 这里的命名在语义上可能不是那么合理，可以查看 parser
    //   的具体设置逻辑，可以发现只有在没有 semi-colon 且下一个 token
    //   是 } 时，才会设置 is_last 为 true
    estmt.type = inferType(estmt.expr->type);
    return;
  }

  if (estmt.expr->used_as_stmt && estmt.expr->type != ast::Type{type::TypeFactory::UNIT_TYPE}) {
    // 只有控制流对应的 IfExpr, LoopExpr, WhileLoopExpr, ForLoopExpr
    // 可以直接用作表达式！！！
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "用作语句的表达式必须为 ()，但其为 {}",
        estmt.expr->type.str()
      ),
      estmt.pos,
      ctx.getCurScopeName()
    );
  }

  estmt.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::RetExpr &rexpr)
{
  auto curfuncname = ctx.getCurFuncName();
  auto curfunctype = ctx.getCurFuncType();
  if (auto retval = rexpr.retval.value_or(nullptr); retval) {
    // return Expr
    if (retval->is_var) {
      // 检查该变量是否已经初始化
      checkInit(*retval);
    }

    if (curfunctype != retval->type.type) {
      // 返回值表达式的类型与函数返回值类型不匹配
      reporter.report(
        err::SemErrType::RETTYPE_MISMATCH,
        std::format("返回类型 {} 与函数 {} 声明的 {} 不一致",
          retval->type.str(),
          curfuncname,
          curfunctype->str()
        ),
        rexpr.pos, ctx.getCurScopeName()
      );
    }
  } else {
    // return
    if (!type::typeEquals(curfunctype, type::TypeFactory::UNIT_TYPE)) {
      // 函数返回类型非空但是返回语句无返回值
      reporter.report(
        err::SemErrType::MISSING_RETVAL,
        std::format("函数 '{}' 需要返回值，return 却没有返回", curfuncname),
        rexpr.pos, ctx.getCurScopeName()
      );
    }
  }

  // return expression 使用的是副作用
  // 因此该表达式本身的类型是 unit type
  rexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  rexpr.res_mut = false;
}

void
SemanticChecker::visit(ast::BreakExpr &bexpr)
{
  // break 表达式认为其使用的是副作用
  // 因此设置其类型为 unit type
  // 而非 break 返回的表达式的类型！！！
  bexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  bexpr.res_mut = false;

  // break 表达式只能出现在循环上下文中
  if (!ctx.inLoopCtx()) {
    reporter.report(
      err::SemErrType::BREAK_CTX_ERROR,
      "break 不在 loop/while/for 上下文中",
      bexpr.pos,
      ctx.getCurScopeName()
    );

    return;
  }

  if (auto value = bexpr.value.value_or(nullptr); value) {
    // break Expr
    if (value->is_var) {
      checkInit(*value); // 检查该变量是否已经初始化
    }

    auto *loopctx = ctx.getLoopCtx().value_or(nullptr);
    ASSERT_MSG(loopctx, "must in loop context");

    if (loopctx->kind != SemanticContext::Scope::Kind::LOOP) {
      reporter.report(
        err::SemErrType::BREAK_CTX_ERROR,
        "含有值的 break 表达式只能在 loop 上下文中",
        bexpr.pos,
        ctx.getCurScopeName()
      );
      return;
    }

    // 如果 loop ctx 中没有记录 loop 表达式对应的临时变量
    // 则生成一个临时变量并在 loop ctx 中记录
    if (!loopctx->val.has_value()) {
      loopctx->val = ctx.produceTemp(bexpr.pos, value->type.type);
    }
    bexpr.dst = loopctx->val.value();
  }
}

void
SemanticChecker::visit(ast::ContinueExpr &cexpr)
{
  // continue 只能出现在循环上下文中
  if (!ctx.inLoopCtx()) {
    reporter.report(
      err::SemErrType::CONTINUE_CTX_ERROR,
      "continue 不在 loop/while/for 上下文中",
      cexpr.pos,
      ctx.getCurScopeName()
    );
  }

  cexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  cexpr.res_mut = false;
}

void
SemanticChecker::visit(ast::Variable &var)
{
  // NOTE: 已经检查了变量是否声明！！！
  // NOTE: 区分对变量的访问还是对变量的赋值！！！

  // 检查变量是否声明
  if (auto val = ctx.lookupVal(var.name).value_or(nullptr); val) {
    var.type = ast::Type{val->type};
    var.res_mut = val->mut;
    var.symbol = val;
  } else {
    reporter.report(
      err::SemErrType::UNDECLARED_VAR,
      std::format("变量 {} 未声明", var.name),
      var.pos,
      ctx.getCurScopeName()
    );

    // 避免报错过多
    var.type = ast::Type{type::TypeFactory::ANY_TYPE};
    var.res_mut = true;
  }

  var.is_var = true;
}

void
SemanticChecker::visit(ast::ArrAcc &aacc)
{
  // 检查数组访问对象是否初始化
  if (aacc.base->is_var) {
    checkInit(*aacc.base);
  }

  // 检查数组访问对象是否是一个数组类型，并检查索引是否是一个 i32
  type::TypePtr arr_type = aacc.base->type.type;
  if (!type::TypeFactory::isArray(arr_type)) {
    // 对象不是 array type
    reporter.report(
      err::SemErrType::TYPE_MISMATCH,
      std::format(
        "数组访问对象是 {} 类型不是数组类型",
        aacc.base->type.str()
      ),
      aacc.base->pos,
      ctx.getCurScopeName()
    );

    aacc.type = ast::Type{type::TypeFactory::ANY_TYPE};
    aacc.res_mut = true; // 避免报错过多
  } else if (aacc.idx->type != ast::Type{type::TypeFactory::INT_TYPE}) {
    // NOTE: 数组访问越界是运行时的问题！！！

    // 索引必须是一个整数！
    reporter.report(
      err::SemErrType::TYPE_MISMATCH,
      std::format(
        "用于访问的数组下标不是一个 i32 类型的值，而是 {}",
        aacc.idx->type.str()
      ),
      aacc.pos,
      ctx.getCurScopeName()
    );

    aacc.type = ast::Type{type::TypeFactory::ANY_TYPE};
    aacc.res_mut = true; // 避免报错过多
  } else {
    // everything is normal~
    aacc.type = ast::Type{arr_type->getElemType()};
    aacc.res_mut = aacc.base->res_mut;
  }
}

void
SemanticChecker::visit(ast::TupAcc &tacc)
{
  // 检查元组访问对象是否初始化
  if (tacc.base->is_var) {
    checkInit(*tacc.base);
  }

  // 检查元组访问对象是否是一个元组类型，并检查元组访问是否越界
  //   Tips: 元组访问越界可以在编译期确定，
  //   因为元组访问是通过给定 number 进行的，
  //   而不支持动态计算索引
  type::TypePtr tuple_type = tacc.base->type.type;
  if (!type::TypeFactory::isTuple(tuple_type)) {
    // 元组访问对象不是元组类型
    reporter.report(
      err::SemErrType::TYPE_MISMATCH,
      std::format(
        "元组访问对象是 {} 类型不是元组类型",
        tacc.base->type.str()
      ),
      tacc.pos,
      ctx.getCurScopeName()
    );
    tacc.type = ast::Type{type::TypeFactory::ANY_TYPE};
    tacc.res_mut = true; // 避免报错过多
  } else if (tacc.idx->value < 0 || tacc.idx->value > tuple_type->size()) {
    // 元组访问越界
    reporter.report(
      err::SemErrType::OUT_OF_BOUNDS_ACCESS,
      std::format(
        "合法的元组访问区间为 0 ~ {}，但试图访问的索引为 {}",
        tuple_type->size() - 1,
        tacc.idx->value
      ),
      tacc.pos,
      ctx.getCurScopeName()
    );
    tacc.type = ast::Type{type::TypeFactory::ANY_TYPE};
    tacc.res_mut = true; // 避免报错过多
  } else {
    tacc.type = ast::Type{tuple_type->getElemType(tacc.idx->value)};
    tacc.res_mut = tacc.base->res_mut;
  }
}

void
SemanticChecker::visit(ast::AssignElem &aelem)
{
  if (aelem.kind == ast::AssignElem::Kind::VARIABLE) {
    aelem.is_var = true;
    aelem.symbol = aelem.base->symbol;
  } else {
    aelem.is_var = false;
  }

  aelem.type = aelem.base->type;
  aelem.res_mut = aelem.base->res_mut;
}

void
SemanticChecker::visit(ast::AssignExpr &aexpr)
{
  // Rust 中赋值表达式的值并不是左值！而是 ()
  aexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};

  // NOTE: res_mut 表示了表达式的计算结果是否可变
  //       这里只有变量及其数组或元组访问是可能可变的
  //       其余表达式的计算记过均是不可变的！
  if (aexpr.lval->is_var) {
    // 赋值表达式中并不需要检查左值变量是否初始化
    // 赋值是给变量初始化的一种手段！
    if (aexpr.lval->type.type == type::TypeFactory::UNKNOWN_TYPE) {
      // type inference
      aexpr.lval->type.type = aexpr.rval->type.type;
      if (aexpr.lval->symbol != nullptr) {
        aexpr.lval->symbol->type = aexpr.rval->type.type;
      }
    }

    if (aexpr.lval->symbol != nullptr) {
      if (aexpr.lval->symbol->init && !aexpr.lval->symbol->mut) {
        // 对于 immutable 的变量而言，
        // 允许未初始化情况下的第一次赋值，将该次赋值视为初始化
        // 后面的赋值则视为错误！
        reporter.report(
          err::SemErrType::ASSIGN_IMMUTABLE,
          "赋值表达式的左值不可变",
          aexpr.lval->pos,
          ctx.getCurScopeName()
        );
        return;
      }
      aexpr.lval->symbol->init = true;
    }
  } else if (!aexpr.lval->res_mut) {
    reporter.report(
      err::SemErrType::ASSIGN_IMMUTABLE,
      "赋值表达式的左值不可变",
      aexpr.lval->pos,
      ctx.getCurScopeName()
    );
    return;
  }

  if (aexpr.rval->is_var) {
    // 检查变量是否初始化
    checkInit(*aexpr.rval);
  }

  if (aexpr.lval->type != aexpr.rval->type) {
    reporter.report(
      err::SemErrType::ASSIGN_MISMATCH,
      std::format(
        "左值类型为 {}，右值类型为 {}",
        aexpr.lval->type.str(),
        aexpr.rval->type.str()
      ),
      aexpr.pos,
      ctx.getCurScopeName()
    );
    return;
  }
}

void
SemanticChecker::visit(ast::CmpExpr &cexpr)
{
  if (cexpr.lhs->is_var) {
    checkInit(*cexpr.lhs); // 检查变量是否初始化
  }
  if (cexpr.rhs->is_var) {
    checkInit(*cexpr.rhs); // 检查变量是否初始化
  }

  cexpr.res_mut = false;
  cexpr.type = ast::Type{type::TypeFactory::BOOL_TYPE};

  if (cexpr.lhs->type != ast::Type{type::TypeFactory::INT_TYPE}
    || cexpr.rhs->type != ast::Type{type::TypeFactory::INT_TYPE})
  {
    reporter.report(
      err::SemErrType::INCOMPARABLE_TYPES,
      std::format(
        "{} 和 {} 不可比较",
        cexpr.lhs->type.str(),
        cexpr.rhs->type.str()
      ),
      cexpr.pos,
      ctx.getCurScopeName()
    );
  }
}

void
SemanticChecker::visit(ast::AriExpr &aexpr)
{
  if (aexpr.lhs->is_var) {
    checkInit(*aexpr.lhs); // 检查变量是否初始化
  }
  if (aexpr.rhs->is_var) {
    checkInit(*aexpr.rhs); // 检查变量是否初始化
  }

  aexpr.res_mut = false;
  aexpr.type = ast::Type{type::TypeFactory::INT_TYPE};

  if (aexpr.lhs->type != ast::Type{type::TypeFactory::INT_TYPE}
    || aexpr.rhs->type != ast::Type{type::TypeFactory::INT_TYPE})
  {
    reporter.report(
      err::SemErrType::NON_COMPUTABLE_TYPES,
      std::format(
        "{} 和 {} 不可进行算术运算",
        aexpr.lhs->type.str(),
        aexpr.rhs->type.str()
      ),
      aexpr.pos,
      ctx.getCurScopeName()
    );
  }
}

void
SemanticChecker::visit(ast::ArrElems &aelems)
{
  for (const auto &elem : aelems.elems) {
    if (elem->is_var) {
      checkInit(*elem); // 检查变量是否初始化
    }
  }

  if (aelems.elems.size() == 0) {
    auto arr_type = ctx.produceArrType(0, type::TypeFactory::UNKNOWN_TYPE);
    aelems.type = ast::Type{arr_type};
    return;
  }

  ast::Type type = aelems.elems[0]->type;
  for (const auto &elem : aelems.elems) {
    if (elem->type != type) {
      reporter.report(
        err::SemErrType::TYPE_MISMATCH,
        std::format(
          "数组期望的元素类型为 {}，但是该元素类型为 {}",
          type.str(),
          elem->type.str()
        ),
        elem->pos,
        ctx.getCurScopeName()
      );
    }
  }

  aelems.res_mut = false;
  aelems.type = ast::Type{ctx.produceArrType(aelems.elems.size(), type.type)};
}

void
SemanticChecker::visit(ast::TupElems &telems)
{
  for (const auto &elem : telems.elems) {
    if (elem->is_var) {
      checkInit(*elem); // 检查变量是否初始化
    }
  }

  auto etypes = telems.elems
    | std::views::transform([](const auto &elem) {
        return elem->type.type;
      })
    | std::ranges::to<std::vector<type::TypePtr>>();

  telems.res_mut = false;
  telems.type = ast::Type{ctx.produceTupType(etypes)};
}

void
SemanticChecker::visit(ast::BracketExpr &bexpr)
{
  auto expr = bexpr.expr.value_or(nullptr);
  if (expr) {
    if (expr->is_var) {
      checkInit(*expr); // 检查变量是否初始化
    }

    bexpr.res_mut = expr->res_mut;
    bexpr.type = inferType(expr->type);
  } else {
    bexpr.res_mut = false;
    bexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  }
}

void
SemanticChecker::visit(ast::Number &num)
{
  num.symbol = ctx.declareConst(num.value, num.pos);
  num.res_mut = false;
  num.type = ast::Type{type::TypeFactory::INT_TYPE};
}

void
SemanticChecker::visit(ast::CallExpr &cexpr)
{
  cexpr.res_mut = false; // 避免过早截断导致未赋值

  for (const auto &arg : cexpr.argv) {
    if (arg->is_var) {
      checkInit(*arg); // 检查变量是否初始化
    }
  }

  auto opt_func = ctx.lookupFunc(cexpr.callee);
  if (!opt_func.has_value()) {
    reporter.report(
      err::SemErrType::CALL_UNDECLARED_FUNC,
      std::format("调用了未声明的函数 {}", cexpr.callee),
      cexpr.pos,
      ctx.getCurScopeName()
    );
    return; // return!
  } // end if

  const auto &func = opt_func.value();
  cexpr.type = ast::Type{func->type};

  if (func->argv.size() != cexpr.argv.size()) {
    // 实形参个数不匹配
    reporter.report(
      err::SemErrType::ARG_CNT_MISMATCH,
      std::format(
        "形参个数 {} 和实参个数 {} 不匹配",
        func->argv.size(),
        cexpr.argv.size()
      ),
      cexpr.pos,
      ctx.getCurScopeName()
    );
    return; // return!
  } // end if

  // 检查参数类型是否匹配
  auto aparam_types = cexpr.argv
    | std::views::transform([](const auto &arg) {
        return arg->type.type;
      })
    | std::ranges::to<std::vector<type::TypePtr>>();

  auto fparam_types = func->argv
    | std::views::transform([](const auto &arg) {
        return arg->type;
      })
    | std::ranges::to<std::vector<type::TypePtr>>();

  // 依次检查各实形参类型是否匹配
  for (std::size_t i = 0; i < aparam_types.size(); ++i) {
    if (aparam_types[i] != fparam_types[i]) {
      // 使用 type::TypePtr 裸指针进行相等判断，避免 Any Type 泄露导致判断错误
      reporter.report(
        err::SemErrType::ARG_CNT_MISMATCH,
        std::format(
          "第 {} 个参数形参类型为 {}，但实参类型为 {}",
          i + 1,
          fparam_types[i]->str(),
          aparam_types[i]->str()
        ),
        cexpr.pos,
        ctx.getCurScopeName()
      );
    } // end if
  } // end for
}

void
SemanticChecker::visit(ast::IfExpr &iexpr)
{
  if (iexpr.cond->is_var) {
    checkInit(*iexpr.cond); // 检查变量是否初始化
  }

  // condition 在解析时并不保证是一个 CmpExpr，因此需要再次判断
  checkCondition(*iexpr.cond, iexpr.pos);

  ast::Type type;
  if (!iexpr.body->has_ret) {
    type = iexpr.body->type;
  } else if (iexpr.elses.empty() || iexpr.elses.back()->cond.has_value()) {
    type = ast::Type{type::TypeFactory::UNIT_TYPE};
  } else if (
    auto it = std::ranges::find_if(
      iexpr.elses,
      [](const auto &eclause) { return !eclause->body->has_ret; }
    );
    it != iexpr.elses.end()
  ) {
    type = (*it)->body->type;
  } else {
    // 如果所有分支都存在一个 return，则将其设置为 AnyType
    // 因此可以尽最大可能的减少误报
    type = ast::Type{type::TypeFactory::ANY_TYPE};
  }

  // 检查 else if 和 else 分支的类型是否和推导的类型一致
  for (const auto &eclause : iexpr.elses) {
    if (!eclause->body->has_ret && eclause->body->type != type) {
      // 如果分支不含 return 且该分支的类型与推导的类型不一致
      // 则需要报错！
      reporter.report(
        err::SemErrType::TYPE_MISMATCH,
        std::format(
          "else {} 分支期望的类型为 {}，但实际类型为 {}",
          eclause == iexpr.elses.back() ? "" : "if",
          type.str(),
          eclause->body->type.str()
        ),
        eclause->pos,
        ctx.getCurScopeName()
      );
    } // end if
  } // end for

  if (type != ast::Type{type::TypeFactory::UNIT_TYPE}
    && (iexpr.elses.empty() || iexpr.elses.back()->cond.has_value())
  ) { // 如果 type 不是 ()，则需要每一个分支都有返回值！
    reporter.report(
      err::SemErrType::MISSING_ELSE,
      std::format(
        "if 表达式被推导为 {}，但缺少 else 分支",
        type.str()
      ),
      iexpr.pos,
      ctx.getCurScopeName()
    );
  }

  iexpr.res_mut = false;
  iexpr.type = type;
}

void
SemanticChecker::visit(ast::ElseClause &eclause)
{
  // 只需检查 condition 的类型是否是 bool 类型
  // 在 if 表达式中会直接使用 eclause.body 来进行判断
  if (auto cond = eclause.cond.value_or(nullptr); cond) {
    // else if condition StmtBlockExpr
    if (cond->is_var) {
      checkInit(*cond); // 检查变量是否初始化
    }

    checkCondition(*cond, eclause.pos);
  } // end if
}

void
SemanticChecker::visit(ast::WhileLoopExpr &wlexpr)
{
  // 1. 检查 condition 是否是 bool 类型；
  // 2. 检查 while 循环体的类型是否为 ()；
  //   Tips: break 在 BreakExpr 中就已经检查了，不需要额外检查
  if (wlexpr.cond->is_var) {
    checkInit(*wlexpr.cond); // 检查变量是否初始化
  }

  checkCondition(*wlexpr.cond, wlexpr.pos);

  if (wlexpr.body->type != ast::Type{type::TypeFactory::UNIT_TYPE}) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "while 期望语句块类型为 ()，但实际为 {}",
        wlexpr.body->type.str()
      ),
      wlexpr.pos,
      ctx.getCurScopeName()
    );
  } // end if

  wlexpr.res_mut = false;
  wlexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::ForLoopExpr &flexpr)
{
  // 检查 while 循环体的类型是否为 ()；
  //   Tips: break 在 BreakExpr 中就已经检查了，不需要额外检查
  //         Iterable 也在相应 visit 函数中做了检查！
  if (flexpr.body->type != ast::Type{type::TypeFactory::UNIT_TYPE}) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "for 期望语句块类型为 ()，但实际为 {}",
        flexpr.body->type.str()
      ),
      flexpr.pos,
      ctx.getCurScopeName()
    );
  } // end if

  flexpr.res_mut = false;
  flexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::RangeExpr &range_expr)
{
  if (range_expr.start->is_var) {
    checkInit(*range_expr.start); // 检查变量是否初始化
  }
  if (range_expr.end->is_var) {
    checkInit(*range_expr.end); // 检查变量是否初始化
  }

  // 范围表达式端点应该是 i32 类型的值
  if (range_expr.start->type.type != type::TypeFactory::INT_TYPE) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "范围表达式起始值期望类型为 i32，但实际为 {}",
        range_expr.start->type.str()
      ),
      range_expr.start->pos,
      ctx.getCurScopeName()
    );
  }
  if (range_expr.end->type.type != type::TypeFactory::INT_TYPE) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "范围表达式终止值望类型为 i32，但实际为 {}",
        range_expr.end->type.str()
      ),
      range_expr.end->pos,
      ctx.getCurScopeName()
    );
  }
}

void
SemanticChecker::visit(ast::IterableVal &iter)
{
  if (iter.value->is_var) {
    checkInit(*iter.value); // 检查变量是否初始化
  }

  // 现阶段只有 ArrayType 是可以迭代的
  ast::Type type = iter.value->type;
  if (!type::TypeFactory::isArray(type.type)) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "{} 不可迭代",
        type.str()
      ),
      iter.pos,
      ctx.getCurScopeName()
    );
  } // end if

  // 虽然 AnyType 不是 iterable 的
  // 但是仍需避免其因为类型推导而暴露出去
  iter.symbol = iter.value->symbol;
  iter.type = inferType(type);
}

void
SemanticChecker::visit(ast::LoopExpr &lexpr)
{
  // loop expression 的类型由其循环体内的 break 语句决定
  // 其循环体的类型应该是 unit type
  BreakChecker bchecker{ctx, reporter};
  lexpr.body->accept(bchecker);
  if (bchecker.has_break) {
    // Break Checker 返回的类型是 break 表达式返回的值的类型
    // 因此可以将 break 表达式本身的类型安全的设置为 unit type!
    lexpr.type = ast::Type{bchecker.type};
  } else {
    lexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  }

  if (lexpr.body->type != ast::Type{type::TypeFactory::UNIT_TYPE}) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "for 期望语句块类型为 ()，但实际为 {}",
        lexpr.body->type.str()
      ),
      lexpr.pos,
      ctx.getCurScopeName()
    );
  } // end if

  if (lexpr.type.type != type::TypeFactory::UNIT_TYPE) {
    auto *loopctx = ctx.getLoopCtx().value_or(nullptr);
    ASSERT_MSG(loopctx, "don't in loop ctx");
    ASSERT_MSG(loopctx->val.has_value(), "didn't allocate temp value");
    lexpr.symbol = loopctx->val.value();
  }
  lexpr.res_mut = false;
}

} // namespace sem
