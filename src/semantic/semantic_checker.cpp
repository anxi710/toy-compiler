#include <ranges>
#include <cassert>

#include "err_type.hpp"
#include "type_factory.hpp"
#include "semantic_checker.hpp"

namespace sem {

void
SemanticChecker::visit(ast::Prog &prog)
{

}

void
SemanticChecker::visit(ast::FuncDecl &fdecl)
{
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
  ctx.declareArg(arg.name, arg.mut, arg.type.type);
}

void
SemanticChecker::visit(ast::StmtBlockExpr &sbexpr)
{
}

void
SemanticChecker::visit(ast::VarDeclStmt &vdstmt)
{
  type::TypePtr type = vdstmt.type.type;
  if (type != type::TypeFactory::UNKNOWN_TYPE) {
    // let (mut)? <ID> : Type ;
    if (vdstmt.value.has_value() &&
        vdstmt.type != vdstmt.value.value()->type
    ) {
      // let (mut)? <ID> : Type = Expr ;
      //TODO: Error
    }
  } else {
    // let (mut)? <ID> ;
    if (vdstmt.value.has_value()) {
      // let (mut)? <ID> = Expr ;
      // Type Inference
      vdstmt.type = vdstmt.value.value()->type;
    }
  }

  bool init = vdstmt.value.has_value();
  ctx.declareVar(vdstmt.name, vdstmt.mut, init, type);
}

void
SemanticChecker::visit(ast::ExprStmt &estmt)
{
}

void
SemanticChecker::visit(ast::RetExpr &rexpr)
{
  if (rexpr.retval.has_value()) {
    // 有返回值表达式
    ast::ExprPtr retval = rexpr.retval.value();
    ast::Type rv_type = retval->type;
    if (ctx.curfunc->type != type::TypeFactory::UNIT_TYPE) {
      // 函数返回类型非空
      if (ctx.curfunc->type != rv_type.type) {
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
    if (ctx.curfunc->type != type::TypeFactory::UNIT_TYPE) {
      // 函数返回类型非空但是返回语句无返回值
      reporter.report(
        err::SemErrType::MISSING_RETVAL,
        std::format("函数 '{}' 需要返回值，return 语句却没有返回", ctx.curfunc->name),
        rexpr.pos, ctx.symtab.getCurScopeName()
      );
    }
  }
}

void
SemanticChecker::visit(ast::AssignExpr &aexpr)
{
  // auto opt_var = ctx.lookupVar(aexpr.lval->name);
  // if (!opt_var.has_value()) {
  //   //TODO: 变量未定义
  //
  //   return;
  // }

  // const auto &var = opt_var.value();
  // if (!var->mut) {
  //   //TODO: 左值不可变
  //
  //   return;
  // }

  if (aexpr.lval->type != aexpr.rval->type) {
    //TODO: 左值和右值类型不匹配
  }

  // Rust 中赋值表达式的返回值并不是左值！
  aexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
}

void
SemanticChecker::visit(ast::AssignElem &aelem)
{
}

void
SemanticChecker::visit(ast::Variable &var)
{
  auto opt_var = ctx.lookupVar(var.name);
  type::TypePtr type;
  if (!opt_var.has_value()) {
    //TODO: 变量未定义

    type = type::TypeFactory::UNKNOWN_TYPE;
  } else {
    type = opt_var.value()->type;
  }
  var.type = ast::Type{type};
}

void
SemanticChecker::visit(ast::ArrayAccess &aacc)
{
  type::TypePtr arr_type = aacc.value->type.type;
  type::TypePtr idx_type = aacc.idx->type.type;
  type::TypePtr elem_type;
  if (!type::TypeFactory::isArray(arr_type)) {
    //TODO: 数组访问对象不是数组类型

    elem_type = type::TypeFactory::UNKNOWN_TYPE;
  } else {
    //TODO: 访问越界是运行时的问题！！！
    if (idx_type != type::TypeFactory::INT_TYPE) {
      //TODO: 用于访问的数组下标不是一个整数

      elem_type = type::TypeFactory::UNKNOWN_TYPE;
    } else {
      elem_type = arr_type->getElemType();
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
    //TODO: 数组访问对象不是元组类型

    elem_type = type::TypeFactory::UNKNOWN_TYPE;
  } else {
    if (tacc.idx < 0 || tacc.idx > tuple_type->size()) {
      //TODO: 元组访问越界！

      elem_type = type::TypeFactory::UNKNOWN_TYPE;
    } else {
      elem_type = tuple_type->getElemType(tacc.idx);
    }
  }
  tacc.type = ast::Type{elem_type};
}

void
SemanticChecker::visit(ast::CmpExpr &cexpr)
{
  if (cexpr.lhs->type.type != type::TypeFactory::INT_TYPE ||
    cexpr.rhs->type.type != type::TypeFactory::INT_TYPE) {
    //TODO: 只有两个整数可以相互比较！

    return;
  }

  cexpr.type = ast::Type{type::TypeFactory::BOOL_TYPE};
}

void
SemanticChecker::visit(ast::AriExpr &aexpr)
{
  if (aexpr.lhs->type.type != type::TypeFactory::INT_TYPE ||
    aexpr.rhs->type.type != type::TypeFactory::INT_TYPE) {
    //TODO: 只有两个整数可以进行算术运算！

    return;
  }

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

      return;
    }
  }

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

  telems.type = ast::Type{ctx.types.getTuple(etypes)};
}

void
SemanticChecker::visit(ast::BracketExpr &bexpr)
{
  if (bexpr.expr.has_value()) {
    bexpr.type = bexpr.expr.value()->type;
  } else {
    bexpr.type = ast::Type{type::TypeFactory::UNIT_TYPE};
  }
}

void
SemanticChecker::visit(ast::Number &num)
{
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
        return arg->type.type;
      })
    | std::ranges::to<std::vector<type::TypePtr>>();

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
}

void
SemanticChecker::visit(ast::IfExpr &iexpr)
{
  if (iexpr.cond->type.type != type::TypeFactory::INT_TYPE) {
    //TODO: condtion 值的类型无法用于逻辑判断
    // condtion 在解析时并不保证是一个 CmpExpr
    // 因此需要再次判断
  }
}

void
SemanticChecker::visit(ast::ElseClause &eclause)
{
}

void
SemanticChecker::visit(ast::WhileLoopExpr&wlexpr)
{
}

void
SemanticChecker::visit(ast::ForLoopExpr &flexpr)
{
}

void
SemanticChecker::visit(ast::LoopExpr&lexpr)
{
}

void
SemanticChecker::visit(ast::BreakExpr &bstmt)
{
}

void
SemanticChecker::visit(ast::ContinueExpr &cstmt)
{
}

void
SemanticChecker::visit(ast::EmptyStmt &estmt)
{
}

} // namespace sem
