#include <ranges>
#include <cassert>

#include "err_type.hpp"
#include "varref_checker.hpp"
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
  VarRefChecker vrchecker{ctx};
  expr.accept(vrchecker);
  if (!vrchecker.init) {
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

void
SemanticChecker::visit(ast::FuncDecl &fdecl)
{
  // 检查函数体中是否有 return 表达式
  // 如果没有且返回类型是 unit type，则自动生成一个 return ;
  if (!fdecl.body->has_ret) {
    if (fdecl.header->type == ast::Type{type::TypeFactory::UNIT_TYPE}) {
      // 对于 unit type 而言，可以自动生成一个没带返回值的 return 语句
      auto retexpr = std::make_shared<ast::RetExpr>(std::nullopt);
      retexpr->used_as_stmt = true;
      retexpr->type = ast::Type{type::TypeFactory::UNIT_TYPE};
      auto retstmt = std::make_shared<ast::ExprStmt>(retexpr);
      retstmt->type = ast::Type{type::TypeFactory::UNIT_TYPE};
      fdecl.body->stmts.push_back(std::move(retstmt));
    } else if (fdecl.body->type == ast::Type{type::TypeFactory::UNIT_TYPE}){
      // 注意没有 return 语句，但是最后一行是表达式的情况
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
  }
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
    // 注意检查是否为空！
    sbexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  } else {
    // 注意 back() 需要先检查 empty() 再使用！
    if (sbexpr.stmts.back()->type.type == type::TypeFactory::ANY_TYPE) {
      // 注意：这里不能使用 ast::Type 封装的 type::TypePtr 的比较逻辑
      // 因为这里判断的是裸指针是否相等！
      sbexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE}; // ANY_TYPE 在进行推导时要被推导为 unit type!!!
    } else {
      sbexpr.type = sbexpr.stmts.back()->type;
    }
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
  if (vdstmt.vartype.type != type::TypeFactory::UNKNOWN_TYPE) {
    // let (mut)? <ID> : Type ;
    if (vdstmt.value.has_value() &&
        vdstmt.vartype != vdstmt.value.value()->type
    ) {
      // let (mut)? <ID> : Type = Expr ;
      reporter.report(
        err::SemErrType::TYPE_MISMATCH,
        std::format(
          "表达式的类型 {} 和变量 {} 声明的类型 {} 不一致",
          vdstmt.value.value()->type.str(),
          vdstmt.name,
          vdstmt.vartype.str()
        ),
        vdstmt.value.value()->pos,
        ctx.getCurScopeName()
      );
    }
  } else {
    // let (mut)? <ID> ;
    if (vdstmt.value.has_value()) {
      // let (mut)? <ID> = Expr ;
      // Type Inference
      if (vdstmt.value.value()->type.type == type::TypeFactory::ANY_TYPE) {
        vdstmt.vartype = ast::Type{type::TypeFactory::UNIT_TYPE};
      } else {
        vdstmt.vartype = vdstmt.value.value()->type;
      }
    }
  }

  bool init = vdstmt.value.has_value();
  ctx.declareVal(vdstmt.name, vdstmt.mut, init, vdstmt.vartype.type, vdstmt.pos);

  if (vdstmt.value.has_value()) {
    if (auto value = vdstmt.value.value(); value->is_var) {
      // 如果用于初始化的表达式是一个变量，则去检查该变量是否已经初始化
      checkInit(*value);
    }
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
    // 在这里需要注意 ANY_TYPE 的推导问题（ANY_TYPE 作为一个推导类型变量存在，
    // 其不是一个实际的类型）
    if (estmt.expr->type.type == type::TypeFactory::ANY_TYPE) {
      estmt.type = ast::Type{type::TypeFactory::UNIT_TYPE};
    } else {
      estmt.type = estmt.expr->type;
    }
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
  if (rexpr.retval.has_value()) {
    // return Expr
    ast::ExprPtr retval = rexpr.retval.value();
    if (retval->is_var) {
      // 检查该变量是否已经初始化
      checkInit(*retval);
    }

    if (ctx.curfunc->type != retval->type.type) {
      // 返回值表达式的类型与函数返回值类型不匹配
      reporter.report(
        err::SemErrType::RETTYPE_MISMATCH,
        std::format("返回类型 {} 与函数 {} 声明的 {} 不一致",
          retval->type.str(),
          ctx.curfunc->name,
          ctx.curfunc->type->str()
        ),
        rexpr.pos, ctx.getCurScopeName()
      );
    }
  } else {
    // return
    if (!type::typeEquals(ctx.curfunc->type, type::TypeFactory::UNIT_TYPE)) {
      // 函数返回类型非空但是返回语句无返回值
      reporter.report(
        err::SemErrType::MISSING_RETVAL,
        std::format("函数 '{}' 需要返回值，return 却没有返回", ctx.curfunc->name),
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

  if (bexpr.value.has_value()) {
    if (auto value = bexpr.value.value(); value->is_var) {
      checkInit(*value); // 检查该变量是否已经初始化
    }

    if (auto loopctx = ctx.getLoopCtx();
      loopctx.has_value()
    && loopctx.value().kind != SemanticContext::Scope::Kind::LOOP) {
      // break Expr
      reporter.report(
        err::SemErrType::BREAK_CTX_ERROR,
        "含有值的 break 表达式只能在 loop 上下文中",
        bexpr.pos,
        ctx.getCurScopeName()
      );
      return;
    }
  }

  // break 表达式只能出现在循环上下文中
  if (!ctx.inLoopCtx()) {
    reporter.report(
      err::SemErrType::BREAK_CTX_ERROR,
      "break 不在 loop/while/for 上下文中",
      bexpr.pos,
      ctx.getCurScopeName()
    );
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
  auto opt_var = ctx.lookupVal(var.name);
  type::TypePtr type;
  if (opt_var.has_value()) {
    type = opt_var.value()->type;
    var.res_mut = opt_var.value()->mut;
    var.symbol = opt_var.value();
  } else {
    reporter.report(
      err::SemErrType::UNDECLARED_VAR,
      std::format("变量 {} 未声明", var.name),
      var.pos,
      ctx.getCurScopeName()
    );

    // 避免报错过多
    type = type::TypeFactory::ANY_TYPE;
    var.res_mut = true;
  }

  var.type = ast::Type{type};
  var.is_var = true;
}

void
SemanticChecker::visit(ast::ArrAcc &aacc)
{
  // 检查数组访问对象是否初始化
  if (aacc.value->is_var) {
    checkInit(*aacc.value);
  }

  // 检查数组访问对象是否是一个数组类型，并检查索引是否是一个 i32
  type::TypePtr arr_type = aacc.value->type.type;
  type::TypePtr elem_type;
  if (!type::TypeFactory::isArray(arr_type)) {
    // 对象不是 array type
    reporter.report(
      err::SemErrType::TYPE_MISMATCH,
      std::format(
        "数组访问对象是 {} 类型不是数组类型",
        aacc.value->type.str()
      ),
      aacc.value->pos,
      ctx.getCurScopeName()
    );
    elem_type = type::TypeFactory::UNKNOWN_TYPE;
    aacc.res_mut = true; // 避免报错过多
  } else {
    // NOTE: 访问越界是运行时的问题！！！
    if (aacc.idx->type != ast::Type{type::TypeFactory::INT_TYPE}) {
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
      elem_type = type::TypeFactory::UNKNOWN_TYPE;
      aacc.res_mut = true; // 避免报错过多
    } else {
      // everything is normal~
      elem_type = arr_type->getElemType();
      aacc.res_mut = aacc.value->res_mut;
    }
  }

  aacc.type = ast::Type{elem_type};
}

void
SemanticChecker::visit(ast::TupAcc &tacc)
{
  // 检查元组访问对象是否初始化
  if (tacc.value->is_var) {
    checkInit(*tacc.value);
  }

  // 检查元组访问对象是否是一个元组类型，并检查元组访问是否越界
  //   Tips: 元组访问越界可以在编译期确定，
  //   因为元组访问是通过给定 number 进行的，
  //   而不支持动态计算索引
  type::TypePtr tuple_type = tacc.value->type.type;
  type::TypePtr elem_type;
  if (!type::TypeFactory::isTuple(tuple_type)) {
    // 元组访问对象不是元组类型
    reporter.report(
      err::SemErrType::TYPE_MISMATCH,
      std::format(
        "元组访问对象是 {} 类型不是元组类型",
        tacc.value->type.str()
      ),
      tacc.pos,
      ctx.getCurScopeName()
    );
    elem_type = type::TypeFactory::UNKNOWN_TYPE;
    tacc.res_mut = true; // 避免报错过多
  } else {
    if (tacc.idx < 0 || tacc.idx > tuple_type->size()) {
      // 元组访问越界
      reporter.report(
        err::SemErrType::OUTOFBOUNDS_ACCESS,
        std::format(
          "合法的元组访问区间为 0 ~ {}，但试图访问的索引为 {}",
          tuple_type->size() - 1,
          tacc.idx
        ),
        tacc.pos,
        ctx.getCurScopeName()
      );
      elem_type = type::TypeFactory::UNKNOWN_TYPE;
      tacc.res_mut = true; // 避免报错过多
    } else {
      elem_type = tuple_type->getElemType(tacc.idx);
      tacc.res_mut = tacc.value->res_mut;
    }
  }

  tacc.type = ast::Type{elem_type};
}

void
SemanticChecker::visit(ast::AssignElem &aelem)
{
  if (aelem.kind == ast::AssignElem::Kind::VARIABLE) {
    aelem.is_var = true;
    aelem.symbol = aelem.value->symbol;
  } else {
    aelem.is_var = false;
  }

  aelem.type = aelem.value->type;
  aelem.res_mut = aelem.value->res_mut;
}

void
SemanticChecker::visit(ast::AssignExpr &aexpr)
{
  // Rust 中赋值表达式的值并不是左值！而是 ()
  aexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};

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
  }

  if (aexpr.rval->is_var) {
    // 检查变量是否初始化
    checkInit(*aexpr.rval);
  }

  // NOTE: res_mut 表示了表达式的计算结果是否可变
  //       这里只有变量及其数组或元组访问是可能可变的
  //       其余表达式的计算记过均是不可变的！
  if (aexpr.lval->is_var) {
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
  } else {
    if (!aexpr.lval->res_mut) {
      reporter.report(
        err::SemErrType::ASSIGN_IMMUTABLE,
        "赋值表达式的左值不可变",
        aexpr.lval->pos,
        ctx.getCurScopeName()
      );
      return;
    }
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

  if (cexpr.lhs->type != ast::Type{type::TypeFactory::INT_TYPE} ||
    cexpr.rhs->type != ast::Type{type::TypeFactory::INT_TYPE}) {
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

  if (aexpr.lhs->type != ast::Type{type::TypeFactory::INT_TYPE} ||
    aexpr.rhs->type != ast::Type{type::TypeFactory::INT_TYPE}) {
    reporter.report(
      err::SemErrType::UNCOMPUTABLE_TYPES,
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
    auto arr_type = ctx.types.getArray(0, type::TypeFactory::UNKNOWN_TYPE);
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
  aelems.type = ast::Type{ctx.types.getArray(aelems.elems.size(), type.type)};
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
  telems.type = ast::Type{ctx.types.getTuple(etypes)};
}

void
SemanticChecker::visit(ast::BracketExpr &bexpr)
{
  if (bexpr.expr.has_value()) {
    auto expr = bexpr.expr.value();
    if (expr->is_var) {
      checkInit(*expr); // 检查变量是否初始化
    }

    bexpr.res_mut = expr->res_mut;

    if (expr->type.type == type::TypeFactory::ANY_TYPE) {
      bexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
    } else {
      bexpr.type = expr->type;
    }
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
      return;
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

  ast::Type type = ast::Type{type::TypeFactory::UNKNOWN_TYPE};
  if (iexpr.body->has_ret) {
    if (iexpr.elses.empty() || iexpr.elses.back()->cond.has_value()) {
      type = ast::Type{type::TypeFactory::UNIT_TYPE};
    } else {
      for (const auto &eclause : iexpr.elses) {
        if (!eclause->body->has_ret) {
          type = eclause->body->type;
          break;
        }
      } // end for
      if (type == ast::Type{type::TypeFactory::UNKNOWN_TYPE}) {
        // 如果所有分支都存在一个 return，则将其设置为 AnyType
        // 因此可以尽最大可能的减少误报
        type = ast::Type{type::TypeFactory::ANY_TYPE};
      }
    } // end if
  } else {
    type = iexpr.body->type;
  } // end if

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
  if (eclause.cond.has_value()) {
    // else if condition StmtBlockExpr
    auto cond = eclause.cond.value();
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
SemanticChecker::visit(ast::Interval &interval)
{
  if (interval.start->is_var) {
    checkInit(*interval.start); // 检查变量是否初始化
  }
  if (interval.end->is_var) {
    checkInit(*interval.end); // 检查变量是否初始化
  }

  // 区间端点应该是 i32 类型的值
  if (interval.start->type.type != type::TypeFactory::INT_TYPE) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "区间起始值期望类型为 i32，但实际为 {}",
        interval.start->type.str()
      ),
      interval.start->pos,
      ctx.getCurScopeName()
    );
  } else if (interval.end->type.type != type::TypeFactory::INT_TYPE) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "区间终止值望类型为 i32，但实际为 {}",
        interval.end->type.str()
      ),
      interval.end->pos,
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
  type::TypePtr type = iter.value->type.type;
  if (!type::TypeFactory::isArray(type)) {
    reporter.report(
      err::SemErrType::UNEXPECTED_EXPR_TYPE,
      std::format(
        "{} 不可迭代",
        type->str()
      ),
      iter.pos,
      ctx.getCurScopeName()
    );
  } // end if

  if (type == type::TypeFactory::ANY_TYPE) {
    // 虽然 AnyType 不是 iterable 的
    // 但是仍需避免其因为类型推导而暴露出去
    iter.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  } else {
    iter.type = iter.value->type;
  }
}

void
SemanticChecker::visit(ast::LoopExpr&lexpr)
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

  lexpr.res_mut = false;
}

} // namespace sem
