#include <iostream>
#include <ranges>
#include <cassert>

#include "err_type.hpp"
#include "type_factory.hpp"
#include "return_checker.hpp"
#include "semantic_checker.hpp"

namespace sem {

void
SemanticChecker::visit(ast::Prog &prog)
{
}

void
SemanticChecker::visit(ast::FuncDecl &fdecl)
{
  if (!fdecl.body->has_ret) {
    if (fdecl.header->type == type::TypeFactory::UNIT_TYPE) {
      auto retexpr = std::make_shared<ast::RetExpr>(std::nullopt);
      retexpr->used_as_stmt = true;
      retexpr->type = ast::Type{type::TypeFactory::UNIT_TYPE};
      auto retstmt = std::make_shared<ast::ExprStmt>(retexpr);
      retstmt->type = ast::Type{type::TypeFactory::UNIT_TYPE};
      fdecl.body->stmts.push_back(std::move(retstmt));
    } else {
      //TODO: 函数有明确的返回类型，但是没有返回一个值
      std::cerr << "函数有明确的返回类型，但是没有返回一个值" << std::endl;
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
  ctx.declareArg(arg.name, arg.mut, arg.type.type, arg.pos);
}

void
SemanticChecker::visit(ast::StmtBlockExpr &sbexpr)
{
  ReturnChecker rchecker;
  sbexpr.accept(rchecker);
  sbexpr.has_ret = rchecker.has_ret;

  if (sbexpr.stmts.empty()) {
    sbexpr.type = type::TypeFactory::UNIT_TYPE;
  } else {
    sbexpr.type = (*sbexpr.stmts.rbegin())->type;
  }

  //TODO: 检查是否所有变量都被正确推导出了类型
}

void
SemanticChecker::visit(ast::VarDeclStmt &vdstmt)
{
  if (vdstmt.vartype != type::TypeFactory::UNKNOWN_TYPE) {
    // let (mut)? <ID> : Type ;
    if (vdstmt.value.has_value() &&
        vdstmt.vartype != vdstmt.value.value()->type
    ) {
      // let (mut)? <ID> : Type = Expr ;
      //TODO: 表达式的值的类型和变量指定的类型不一致
      std::cerr << "表达式的值的类型和变量指定的类型不一致" << std::endl;
    }
  } else {
    // let (mut)? <ID> ;
    if (vdstmt.value.has_value()) {
      // let (mut)? <ID> = Expr ;
      // Type Inference
      vdstmt.vartype = vdstmt.value.value()->type;
    }
  }

  bool init = vdstmt.value.has_value();
  ctx.declareVar(vdstmt.name, vdstmt.mut, init, vdstmt.vartype.type, vdstmt.pos);
  vdstmt.type = type::TypeFactory::UNIT_TYPE; // 变量声明语句使用的是副作用！！！
}

void
SemanticChecker::visit(ast::ExprStmt &estmt)
{
  if (estmt.expr->used_as_stmt) {
    estmt.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  } else {
    estmt.type = estmt.expr->type;
  }
}

void
SemanticChecker::visit(ast::RetExpr &rexpr)
{
  if (rexpr.retval.has_value()) {
    // 有返回值表达式
    ast::ExprPtr retval = rexpr.retval.value();
    if (ctx.curfunc->type != type::TypeFactory::UNIT_TYPE) {
      // 函数返回类型非空
      if (ctx.curfunc->type != retval->type) {
        // 返回值表达式的类型与函数返回值类型不匹配
        reporter.report(
          err::SemErrType::RETTYPE_MISMATCH,
          std::format("返回类型与函数 '{}' 声明时不匹配", ctx.curfunc->name),
          retval->pos, ctx.symtab.getCurScopeName()
        );
      }
    } else {
      // 函数返回值为空，却有返回值表达式
      reporter.report(
        err::SemErrType::VOID_FUNC_RET_VAL,
        std::format("函数 '{}' 返回值为空，return 语句却有返回值", ctx.curfunc->name),
        retval->pos, ctx.symtab.getCurScopeName()
      );
    }
  } else {
    // return ;
    if (!type::typeEquals(ctx.curfunc->type, type::TypeFactory::UNIT_TYPE)) {
      // 函数返回类型非空但是返回语句无返回值
      reporter.report(
        err::SemErrType::MISSING_RETVAL,
        std::format("函数 '{}' 需要返回值，return 语句却没有返回", ctx.curfunc->name),
        rexpr.pos, ctx.symtab.getCurScopeName()
      );
    }
  }

  // return expression 使用的是副作用
  // 因此该表达式本身的类型是 unit type
  rexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::BreakExpr &bexpr)
{
  if (!ctx.inLoopCtx()) {
    //TODO: 当前不在循环上下文中
    std::cerr << "当前不在循环上下文中" << std::endl;
  }

  if (bexpr.value.has_value()) {
    if (ctx.loopctx.kind != SemanticContext::Scope::Kind::LOOP) {
      //TODO: break 含有返回值但是没在 loop 中
      std::cerr << "break 含有返回值但是没在 loop 中" << std::endl;

      bexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
    } else {
      bexpr.type = bexpr.value.value()->type;
    }
  } else {
    bexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  }
}

void
SemanticChecker::visit(ast::ContinueExpr &cexpr)
{
  if (!ctx.inLoopCtx()) {
    //TODO: 当前不在循环上下文中
  }

  cexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::Variable &var)
{
  auto opt_var = ctx.lookupVar(var.name);
  type::TypePtr type;
  if (!opt_var.has_value()) {
    //TODO: 变量未定义

    type = type::TypeFactory::UNKNOWN_TYPE;
    var.res_mut = true;
  } else {
    type = opt_var.value()->type;
    var.res_mut = opt_var.value()->mut;
  }
  var.type = ast::Type{type};
}

void
SemanticChecker::visit(ast::ArrayAccess &aacc)
{
  type::TypePtr arr_type = aacc.value->type.type;
  type::TypePtr elem_type;
  if (!type::TypeFactory::isArray(arr_type)) {
    //TODO: 数组访问对象不是数组类型

    elem_type = type::TypeFactory::UNKNOWN_TYPE;
    aacc.res_mut = true;
  } else {
    //TODO: 访问越界是运行时的问题！！！
    if (aacc.idx->type != type::TypeFactory::INT_TYPE) {
      //TODO: 用于访问的数组下标不是一个整数

      elem_type = type::TypeFactory::UNKNOWN_TYPE;
      aacc.res_mut = true;
    } else {
      elem_type = arr_type->getElemType();
      aacc.res_mut = aacc.value->res_mut;
    }
  }

  aacc.type = ast::Type{elem_type};
}

void
SemanticChecker::visit(ast::TupleAccess &tacc)
{
  type::TypePtr tuple_type = tacc.value->type.type;
  type::TypePtr elem_type;
  if (!type::TypeFactory::isTuple(tuple_type)) {
    //TODO: 元组访问对象不是元组类型

    elem_type = type::TypeFactory::UNKNOWN_TYPE;
    tacc.res_mut = true;
  } else {
    if (tacc.idx < 0 || tacc.idx > tuple_type->size()) {
      //TODO: 元组访问越界！

      elem_type = type::TypeFactory::UNKNOWN_TYPE;
      tacc.res_mut = true;
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
  aelem.type = aelem.value->type;
  aelem.res_mut = aelem.value->res_mut;
}

void
SemanticChecker::visit(ast::AssignExpr &aexpr)
{
  if (!aexpr.lval->res_mut) {
    //TODO: 左值不可变

    return;
  }

  if (aexpr.lval->type != aexpr.rval->type) {
    //TODO: 左值和右值类型不匹配
  }

  // Rust 中赋值表达式的返回值并不是左值！
  aexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::CmpExpr &cexpr)
{
  if (cexpr.lhs->type != type::TypeFactory::INT_TYPE ||
    cexpr.rhs->type != type::TypeFactory::INT_TYPE) {
    //TODO: 只有两个整数可以相互比较！

    return;
  }

  cexpr.res_mut = false;
  cexpr.type = ast::Type{type::TypeFactory::BOOL_TYPE};
}

void
SemanticChecker::visit(ast::AriExpr &aexpr)
{
  if (aexpr.lhs->type != type::TypeFactory::INT_TYPE ||
    aexpr.rhs->type != type::TypeFactory::INT_TYPE) {
    //TODO: 只有两个整数可以进行算术运算！

    return;
  }

  aexpr.res_mut = false;
  aexpr.type = ast::Type{type::TypeFactory::INT_TYPE};
}

void
SemanticChecker::visit(ast::ArrayElems &aelems)
{
  if (aelems.elems.size() == 0) {
    auto arr_type = ctx.types.getArray(0, type::TypeFactory::UNKNOWN_TYPE);
    aelems.type = ast::Type{arr_type};
    return;
  }

  ast::Type type = aelems.elems[0]->type;
  for (const auto &elem : aelems.elems) {
    if (elem->type != type) {
      //TODO: 数组元素中的各元素的类型必须相等！

      aelems.type = ast::Type{type::TypeFactory::UNKNOWN_TYPE};
      return;
    }
  }

  aelems.res_mut = false;
  aelems.type = ast::Type{ctx.types.getArray(aelems.elems.size(), type.type)};
}

void
SemanticChecker::visit(ast::TupleElems &telems)
{
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
    bexpr.res_mut = bexpr.expr.value()->res_mut;
    bexpr.type = bexpr.expr.value()->type;
  } else {
    bexpr.res_mut = false;
    bexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  }

}

void
SemanticChecker::visit(ast::Number &num)
{
  num.res_mut = false;
  num.type = ast::Type{type::TypeFactory::INT_TYPE};
}

void
SemanticChecker::visit(ast::CallExpr &cexpr)
{
  auto opt_func = ctx.lookupFunc(cexpr.callee);
  if (opt_func.has_value()) {
    //TODO: 调用了未声明的函数

    return;
  }

  const auto &func = opt_func.value();
  if (func->argv.size() != cexpr.argv.size()) {
    //TODO: 调用参数个数不匹配

    return;
  }

  // 检查参数类型是否匹配
  auto aparam_types = cexpr.argv
    | std::views::transform([](const auto &arg) {
        return arg->type;
      })
    | std::ranges::to<std::vector<ast::Type>>();

  auto fparam_types = func->argv
    | std::views::transform([](const auto &arg) {
        return arg->type;
      })
    | std::ranges::to<std::vector<type::TypePtr>>();

  for (std::size_t i = 0; i < aparam_types.size(); ++i) {
    if (aparam_types[i] != fparam_types[i]) {
      //TODO: 第 i 个调用参数类型不匹配

      return;
    }
  }

  cexpr.res_mut = false;
  cexpr.type = ast::Type{func->type};
}

void
SemanticChecker::visit(ast::IfExpr &iexpr)
{
  if (iexpr.cond->type != type::TypeFactory::BOOL_TYPE) {
    //TODO: condtion 值的类型无法用于逻辑判断
    // condtion 在解析时并不保证是一个 CmpExpr
    // 因此需要再次判断
  }

  ast::Type type = type::TypeFactory::UNKNOWN_TYPE;
  if (iexpr.body->has_ret) {
    for (const auto &eclause : iexpr.elses) {
      if (!eclause->body->has_ret) {
        type = eclause->body->type;
        break;
      }
    }
    if (type == type::TypeFactory::UNKNOWN_TYPE) {
      type = type::TypeFactory::ANY_TYPE;
    }
  } else {
    type = iexpr.body->type;
  }

  for (const auto &eclause : iexpr.elses) {
    if (!eclause->body->has_ret && eclause->body->type != type) {
      //TODO: else 分支的类型与推导出的类型不一致
    }
  }

  if (type != type::TypeFactory::UNIT_TYPE &&
      iexpr.elses.back()->cond.has_value()) {
    //TODO: 推断出的类型不是 unit，但是没有 else 分支
  }

  iexpr.type = type;
}

void
SemanticChecker::visit(ast::ElseClause &eclause)
{
  if (eclause.cond.has_value()) {
    // else if
    auto cond = eclause.cond.value();
    if (cond->type != type::TypeFactory::BOOL_TYPE) {
      //TODO: condition 值的类型无法用于逻辑判断
    }
  }
}

void
SemanticChecker::visit(ast::WhileLoopExpr &wlexpr)
{
  if (wlexpr.cond->type != type::TypeFactory::BOOL_TYPE) {
    //TODO: condition 值的类型无法用于条件判断
  }

  if (wlexpr.body->type != type::TypeFactory::UNIT_TYPE) {
    //TODO: while 循环期望语句块表达式的类型是 unit type
  }

  wlexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::ForLoopExpr &flexpr)
{
  if (flexpr.body->type != type::TypeFactory::UNIT_TYPE) {
    //TODO: for 循环期望语句块表达式的类型是 unit type
  }
  flexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::Interval &interval)
{
  if (interval.start->type != type::TypeFactory::INT_TYPE) {
    //TODO:
  } else if (interval.end->type != type::TypeFactory::INT_TYPE) {
    //TODO:
  }

}

void
SemanticChecker::visit(ast::IterableVal &iter)
{
  type::TypePtr type = iter.value->type.type;
  if (!type::TypeFactory::isArray(type)) {
    //TODO: 只有 Array 是可以迭代的！
  }
  iter.type = iter.value->type;
}

void
SemanticChecker::visit(ast::LoopExpr&lexpr)
{
}

} // namespace sem
