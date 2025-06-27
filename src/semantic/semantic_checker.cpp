#include <cassert>

#include "error/err_type.hpp"
#include "semantic_checker.hpp"

using namespace par::ast;

namespace sem {

/**
 * @brief  设置语义检查器使用的错误报告器指针
 * @param  p_ereporter 错误报告器智能指针
 */
void
SemanticChecker::setErrReporter(std::shared_ptr<err::ErrReporter> p_ereporter)
{
  this->p_ereporter = std::move(p_ereporter);
}

/**
 * @brief  设置语义检查器使用的符号表指针
 * @param  p_stable 符号表智能指针
 */
void
SemanticChecker::setSymbolTable(std::shared_ptr<sym::SymbolTable> p_stable)
{
  this->p_stable = std::move(p_stable);
}

/**
 * @brief  对程序入口节点执行语义检查，递归检查所有函数声明
 * @param  p_prog 程序根节点指针，包含所有顶层声明
 */
void
SemanticChecker::checkProg(const ProgPtr &p_prog)
{
  for (const auto &p_decl : p_prog->decls)
  {
    // 最顶层的产生式为 Prog -> (FuncDecl)*
    auto p_fdecl = std::dynamic_pointer_cast<FuncDecl>(p_decl);
    assert(p_fdecl); // 在 parser 的实现中，只能解析
                     // FuncDecl，碰到非函数声明会直接终止解析！
    checkFuncDecl(p_fdecl);
  }
}

/**
 * @brief  执行函数声明的语义检查，包括参数和函数体
 * @param  p_fdecl 函数声明节点指针
 */
void
SemanticChecker::checkFuncDecl(const FuncDeclPtr &p_fdecl)
{
  p_stable->enterScope(p_fdecl->header->name); // 注意不要在没进入作用域时就开始声明变量！

  checkFuncHeaderDecl(p_fdecl->header);

  if (!checkBlockStmt(p_fdecl->body)) {
    // 函数内无return语句
    std::string cfunc_name = p_stable->getFuncName();
    auto opt_func = p_stable->lookupFunc(cfunc_name);
    assert(opt_func.has_value());

    const auto &p_func = opt_func.value();
    if (p_func->retval_type != sym::VarType::Null) {
      // 有返回类型但无返回值表达式
      p_ereporter->report(
        err::SemErrType::MissingReturnValue,
        std::format("函数 '{}' 需要返回值，但没返回", cfunc_name),
        p_func->pos.row,
        p_func->pos.col,
        p_stable->getCurScope()
      );
    }
  }

  p_stable->exitScope();
}

/**
 * @brief  检查函数头部声明的语义，处理函数名、返回类型和形参
 * @param  p_fhdecl 函数头部声明节点指针
 */
void
SemanticChecker::checkFuncHeaderDecl(const FuncHeaderDeclPtr &p_fhdecl)
{
  int argc = p_fhdecl->argv.size();
  for (const auto &arg : p_fhdecl->argv) {
    std::string name = arg->variable->name;
    // 形参的初始值在函数调用时才会被赋予
    auto p_fparam = std::make_shared<sym::Integer>(name, true, std::nullopt);
    p_fparam->setPos(arg->pos);
    p_stable->declareVar(name, p_fparam);
  }

  auto rt = p_fhdecl->retval_type.has_value() ? sym::VarType::I32 : sym::VarType::Null;

  auto p_func = std::make_shared<sym::Function>(p_fhdecl->name, argc, rt);
  p_func->setPos(p_fhdecl->pos);

  p_stable->declareFunc(p_fhdecl->name, p_func);
}

/**
 * @brief  检查代码块语义是否合法，并建立对应的作用域
 * @param  p_bstmt 代码块语句节点指针
 * @return 是否有返回语句
 */
bool
SemanticChecker::checkBlockStmt(const BlockStmtPtr &p_bstmt)
{
  int if_cnt = 1;
  int while_cnt = 1;
  bool has_retstmt = false;

  for (const auto &p_stmt : p_bstmt->stmts) {
    switch (p_stmt->type()) {
    default:
      throw std::runtime_error{"检查到不支持的语句类型"};
    case NodeType::VarDeclStmt:
      checkVarDeclStmt(std::dynamic_pointer_cast<VarDeclStmt>(p_stmt));
      break;
    case NodeType::RetStmt:
      checkRetStmt(std::dynamic_pointer_cast<RetStmt>(p_stmt));
      has_retstmt = true;
      break;
    case NodeType::ExprStmt:
      checkExprStmt(std::dynamic_pointer_cast<ExprStmt>(p_stmt));
      break;
    case NodeType::AssignStmt:
      checkAssignStmt(std::dynamic_pointer_cast<AssignStmt>(p_stmt));
      break;
    case NodeType::IfStmt:
      p_stable->enterScope(std::format("if{}", if_cnt++));
      checkIfStmt(std::dynamic_pointer_cast<IfStmt>(p_stmt));
      p_stable->exitScope();
      break;
    case NodeType::WhileStmt:
      p_stable->enterScope(std::format("while{}", while_cnt++));
      checkWhileStmt(std::dynamic_pointer_cast<WhileStmt>(p_stmt));
      p_stable->exitScope();
      break;
    case NodeType::NullStmt:
      break;
    }
  }

  auto failed_vars = p_stable->checkAutoTypeInference(); // 语句块后检查变量是否有类型
  for (const auto &p_var : failed_vars) {
    p_ereporter->report(
      err::SemErrType::TypeInferenceFailure,
      std::format("变量 '{}' 无法通过自动类型推导确定类型", p_var->name),
      p_var->pos.row,
      p_var->pos.col,
      p_stable->getCurScope()
    );
  }

  return has_retstmt;
}

/**
 * @brief  检查变量声明语句的语义，处理变量定义及初始化
 * @param  p_vdstmt 变量声明语句节点指针
 */
void
SemanticChecker::checkVarDeclStmt(const VarDeclStmtPtr &p_vdstmt)
{
  // 需要实现的产生式中，所有变量声明语句声明的变量只有两种情况
  // 1. let mut a : i32;
  // 2. let mut a;
  // 因此，我们这里简单的记录变量
  // 需要注意的是，由于存在自动类型推导，如果没有指定类型，理论上只能声明一个
  // Variable 符号，而不能直接声明一个 Variable 的子类
  // 简单起见，这里将所有变量声明为一个 i32 的变量

  const std::string &name = p_vdstmt->variable->name;

  sym::VariablePtr p_var;
  if (p_vdstmt->var_type.has_value()) {
    // 1: let mut a : i32; 明确类型，直接构造变量
    p_var = std::make_shared<sym::Integer>(name, false, std::nullopt);
  } else {
    // 2: let mut a; 自动类型推导 —— 类型未知
    p_var = std::make_shared<sym::Variable>(name, false, sym::VarType::Unknown);
  }
  p_var->initialized = false;
  p_var->setPos(p_vdstmt->pos);
  p_stable->declareVar(name, p_var);
}

/**
 * @brief  检查 return 语句的语义，验证返回值类型是否匹配
 * @param  p_rstmt 返回语句节点指针
 */
void
SemanticChecker::checkRetStmt(const RetStmtPtr &p_rstmt)
{
  std::string cfunc_name = p_stable->getFuncName();
  auto opt_func = p_stable->lookupFunc(cfunc_name);
  assert(opt_func.has_value());

  const auto &p_func = opt_func.value();
  if (p_rstmt->ret_val.has_value()) {
    // 有返回值表达式
    sym::VarType ret_type = checkExpr(p_rstmt->ret_val.value());

    if (p_func->retval_type != sym::VarType::Null) {
      // 函数有明确返回类型
      if (p_func->retval_type != ret_type) {
        // 返回值表达式类型与函数返回类型不符
        p_ereporter->report(
          err::SemErrType::FuncReturnTypeMismatch,
          std::format("函数 '{}' return 语句返回类型错误", cfunc_name),
          p_rstmt->pos.row,
          p_rstmt->pos.col,
          p_stable->getCurScope()
        );
      }
    } else {
      // 函数不需要返回值，却有返回值表达式
      p_ereporter->report(
        err::SemErrType::VoidFuncReturnValue,
        std::format("函数 '{}' 不需要返回值，return 语句却有返回值", cfunc_name),
        p_rstmt->pos.row,
        p_rstmt->pos.col,
        p_stable->getCurScope()
      );
    }
  } else {
    // return;
    if (p_func->retval_type != sym::VarType::Null) {
      // 有返回类型但无返回值表达式
      p_ereporter->report(
        err::SemErrType::MissingReturnValue,
        std::format("函数 '{}' 需要返回值，return语句却没有返回值", cfunc_name),
        p_rstmt->pos.row,
        p_rstmt->pos.col,
        p_stable->getCurScope()
      );
    }
  }
}

/**
 * @brief  检查表达式语句的语义，并推导其结果类型
 * @param  p_estmt 表达式语句节点指针
 * @return 表达式的类型
 */
sym::VarType
SemanticChecker::checkExprStmt(const ExprStmtPtr &p_estmt)
{
  switch (p_estmt->expr->type()) {
  default:
    return checkExpr(p_estmt->expr);
  case NodeType::CallExpr:
    return checkCallExpr(std::dynamic_pointer_cast<CallExpr>(p_estmt->expr));
  }
}

/**
 * @brief  语义检查表达式节点，推导并返回表达式的类型
 * @param  p_expr 表达式节点指针
 * @return 表达式的类型
 */
sym::VarType
SemanticChecker::checkExpr(const ExprPtr &p_expr)
{
  switch (p_expr->type()) {
  default:
    throw std::runtime_error{"检查到不支持的表达式类型"};
  case NodeType::ParenthesisExpr: {
    auto paren_expr = std::dynamic_pointer_cast<ParenthesisExpr>(p_expr);
    return checkExpr(paren_expr->expr);
  }
  case NodeType::CallExpr:
    return checkCallExpr(std::dynamic_pointer_cast<CallExpr>(p_expr));
  case NodeType::ComparExpr:
    return checkComparExpr(std::dynamic_pointer_cast<ComparExpr>(p_expr));
  case NodeType::ArithExpr:
    return checkArithExpr(std::dynamic_pointer_cast<ArithExpr>(p_expr));
  case NodeType::Factor:
    return checkFactor(std::dynamic_pointer_cast<Factor>(p_expr));
  case NodeType::Variable:
    return checkVariable(std::dynamic_pointer_cast<Variable>(p_expr));
  case NodeType::Number:
    return checkNumber(std::dynamic_pointer_cast<Number>(p_expr));
  }
}

/**
 * @brief  检查函数调用表达式的语义，验证参数和返回类型
 * @param  p_caexpr 函数调用表达式节点指针
 * @return 调用表达式的返回类型
 */
sym::VarType
SemanticChecker::checkCallExpr(const CallExprPtr &p_caexpr)
{
  // 检查调用表达式的实参和形参是否一致
  auto opt_func = p_stable->lookupFunc(p_caexpr->callee);

  if (!opt_func.has_value()) {
    p_ereporter->report(
      err::SemErrType::UndefinedFunctionCall,
      std::format("调用了未定义的函数 '{}'", p_caexpr->callee),
      p_caexpr->pos.row,
      p_caexpr->pos.col,
      p_stable->getCurScope()
    );
    return sym::VarType::Null;
  }

  const auto &p_func = opt_func.value();

  if (static_cast<int>(p_caexpr->argv.size()) != p_func->argc) {
    p_ereporter->report(
      err::SemErrType::ArgCountMismatch,
      std::format(
        "函数 '{}' 期望 {} 个参数，但调用提供了 {} 个",
        p_caexpr->callee, p_func->argc, p_caexpr->argv.size()
      ),
      p_caexpr->pos.row,
      p_caexpr->pos.col,
      p_stable->getCurScope());
  }

  // 遍历所有实参与进行表达式语义检查
  for (const auto &arg : p_caexpr->argv) {
    assert(arg);
    auto rv_type = checkExpr(arg);
    if (rv_type != sym::VarType::I32) {
      // error report
    }
  }

  return p_func->retval_type;
}

/**
 * @brief  检查比较表达式的语义，确保操作数类型兼容并返回结果类型
 * @param  p_coexpr 比较表达式节点指针
 * @return 表达式的类型
 */
sym::VarType
SemanticChecker::checkComparExpr(const ComparExprPtr &p_coexpr)
{
  // 递归检查子树中所涉及的符号是否类型匹配，是否有值能够使用
  // 在这里我们只需要简单的检查，变量是否是第一次使用，如果是第一次使用，其是否有初值
  checkExprStmt(std::make_shared<ExprStmt>(p_coexpr->lhs));
  checkExprStmt(std::make_shared<ExprStmt>(p_coexpr->rhs));

  // 这里只是一个简化的实现，理论上应该是一个 Bool 类型的值
  return sym::VarType::I32;
}

/**
 * @brief  检查算术表达式的语义，验证操作数类型并推导表达式类型
 * @param  p_aexpr 算术表达式节点指针
 * @return 表达式的类型
 */
sym::VarType
SemanticChecker::checkArithExpr(const ArithExprPtr &p_aexpr)
{
  checkExprStmt(std::make_shared<ExprStmt>(p_aexpr->lhs));
  checkExprStmt(std::make_shared<ExprStmt>(p_aexpr->rhs));

  return sym::VarType::I32;
}

/**
 * @brief  检查因子节点的语义，推导因子的类型
 * @param  p_factor 因子节点指针
 * @return 因子的类型
 */
sym::VarType
SemanticChecker::checkFactor(const FactorPtr &p_factor)
{
  auto expr = p_factor->element;

  switch (expr->type()) {
  default:
    throw std::runtime_error{"不支持的因子类型"};
  case NodeType::Number:
    return checkNumber(std::dynamic_pointer_cast<Number>(p_factor->element));
  case NodeType::Variable:
    return checkVariable(
        std::dynamic_pointer_cast<Variable>(p_factor->element));
  case NodeType::CallExpr:
    return checkCallExpr(std::dynamic_pointer_cast<CallExpr>(p_factor->element));
  }
}

/**
 * @brief  检查变量节点的语义，获取变量的类型信息
 * @param  p_variable 变量节点指针
 * @return 变量的类型
 */
sym::VarType
SemanticChecker::checkVariable(const VariablePtr &p_variable)
{
  auto opt_var = p_stable->lookupVar(p_variable->name);

  if (!opt_var.has_value()) {
    p_ereporter->report(
      err::SemErrType::UndeclaredVariable,
      std::format("变量 '{}' 未声明", p_variable->name),
      p_variable->pos.row,
      p_variable->pos.col,
      p_stable->getCurScope()
    );
    return sym::VarType::Unknown;
  }

  const auto &p_var = opt_var.value();

  if (!p_var->initialized && !p_var->formal) {
    p_ereporter->report(
      err::SemErrType::UninitializedVariable,
      std::format("变量 '{}' 在第一次使用前未初始化", p_variable->name),
      p_variable->pos.row,
      p_variable->pos.col,
      p_stable->getCurScope()
    );
  }

  return p_var->var_type;
}

/**
 * @brief  检查数字节点的语义，返回固定的整型类型
 * @param  p_number 数字节点指针
 * @return 数字的类型（固定为 I32）
 */
sym::VarType
SemanticChecker::checkNumber(const NumberPtr &p_number)
{
  return sym::VarType::I32; // 异常简化的实现
}

/**
 * @brief  检查赋值语句的语义，验证左值和右值合法性及类型匹配
 * @param  p_astmt 赋值语句节点指针
 */
void
SemanticChecker::checkAssignStmt(const AssignStmtPtr &p_astmt)
{
  auto lhs_var = std::dynamic_pointer_cast<Variable>(p_astmt->lvalue);

  if (!lhs_var) {
    p_ereporter->report(
      err::SemErrType::AssignToNonVariable,
      std::format("赋值语句左侧不是变量"),
      p_astmt->lvalue->pos.row,
      p_astmt->lvalue->pos.col,
      p_stable->getCurScope()
    );
  }

  auto opt_var = p_stable->lookupVar(lhs_var->name);

  if (!opt_var.has_value()) {
    p_ereporter->report(
      err::SemErrType::AssignToUndeclaredVar,
      std::format("赋值语句左侧变量 '{}' 未声明", lhs_var->name),
      p_astmt->lvalue->pos.row,
      p_astmt->lvalue->pos.col,
      p_stable->getCurScope()
    );
  }

  const auto &p_var = opt_var.value();

  // 先检查右侧表达式是否合法
  auto expr_stmt = std::make_shared<ExprStmt>(p_astmt->expr);
  sym::VarType rhs_type = checkExpr(expr_stmt->expr);

  // 自动类型推导
  if (p_var->var_type == sym::VarType::Unknown) {
    p_var->var_type = rhs_type;
  } else if (rhs_type != sym::VarType::Unknown && p_var->var_type != rhs_type) {
    p_ereporter->report(
      err::SemErrType::TypeMismatch,
      std::format("变量 '{}' 的类型不匹配", lhs_var->name),
      p_astmt->lvalue->pos.row,
      p_astmt->lvalue->pos.col,
      p_stable->getCurScope()
    );
  }
  // 右侧合法后，标记左侧变量为“已初始化”
  p_var->initialized = true;
}

/**
 * @brief  检查 if 语句的语义，验证条件表达式及分支语句合法性
 * @param  p_istmt if 语句节点指针
 */
void
SemanticChecker::checkIfStmt(const IfStmtPtr &p_istmt)
{
  checkExprStmt(std::make_shared<ExprStmt>(p_istmt->expr));
  checkBlockStmt(p_istmt->if_branch);

  assert(p_istmt->else_clauses.size() <= 1);
  if (p_istmt->else_clauses.size() == 1) {
    p_stable->enterScope("else");
    checkBlockStmt(p_istmt->else_clauses[0]->block);
    p_stable->exitScope();
  }
}

/**
 * @brief  检查 while 语句的语义，验证循环条件表达式及循环体合法性
 * @param  p_wstmt while 语句节点指针
 */
void
SemanticChecker::checkWhileStmt(const WhileStmtPtr &p_wstmt)
{
  checkExprStmt(std::make_shared<ExprStmt>(p_wstmt->expr));
  checkBlockStmt(p_wstmt->block);
}

} // namespace semantic
