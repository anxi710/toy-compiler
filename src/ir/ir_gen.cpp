#include "ir_gen.hpp"

#include <cassert>
#include <regex>

using namespace par::ast;

namespace ir {

static const Operand NULL_OPERAND{std::string{"-"}};

void
IrGenerator::setSymbolTable(std::shared_ptr<sym::SymbolTable> p_stable)
{
  this->p_stable = std::move(p_stable);
}

std::string
IrGenerator::getVarName(const std::string &var_name) const
{
  return std::format("{}::{}", p_stable->getCurScope(), var_name);
}

std::string
IrGenerator::getFuncName() const
{
  std::regex re{R"(^global::(\w*))"};
  std::smatch match;
  std::regex_search(p_stable->getCurScope(), match, re);
  return match[1];
}

void
IrGenerator::pushQuads(OpCode op, const Operand &arg1, const Operand &arg2,
                       const Operand &res)
{
  quads.emplace_back(op, arg1, arg2, res);
}

static std::string
replaceScopeQualifiers(const std::string &str)
{
  std::regex re{"::"};
  return std::regex_replace(str, re, "_");
}

void
IrGenerator::generateProg(const ProgPtr &p_prog)
{
  for (const auto &p_decl : p_prog->decls)
  {
    auto p_fdecl = std::dynamic_pointer_cast<FuncDecl>(p_decl);
    assert(p_fdecl);
    generateFuncDecl(p_fdecl);
  }
}

void
IrGenerator::generateFuncDecl(const FuncDeclPtr &p_fdecl)
{
  p_stable->enterScope(p_fdecl->header->name, false);

  generateFuncHeaderDecl(std::dynamic_pointer_cast<FuncHeaderDecl>(p_fdecl->header));

  bool has_ret = generateBlockStmt(std::dynamic_pointer_cast<BlockStmt>(p_fdecl->body));
  if (!has_ret) {
    pushQuads(OpCode::Return, NULL_OPERAND, NULL_OPERAND, NULL_OPERAND);
  }

  p_stable->exitScope();
}

void
IrGenerator::generateFuncHeaderDecl(const FuncHeaderDeclPtr &p_fhdecl)
{
  // 函数声明对应的四元式分为两部分
  // 1. 函数名对应的标号；
  // 2. 构建参数
  pushQuads(OpCode::Label, p_fhdecl->name, NULL_OPERAND, NULL_OPERAND);

  for (const auto &arg : p_fhdecl->argv) {
    pushQuads(OpCode::Pop, NULL_OPERAND, NULL_OPERAND, getVarName(arg->variable->name));
  }
}

bool
IrGenerator::generateBlockStmt(const BlockStmtPtr &p_bstmt)
{
  int if_cnt = 1;
  int while_cnt = 1;

  for (const auto &p_stmt : p_bstmt->stmts) {
    switch (p_stmt->type()) {
    default:
      throw std::runtime_error{"检查到不支持的语句类型"};
    case NodeType::VarDeclStmt:
      generateVarDeclStmt(std::dynamic_pointer_cast<VarDeclStmt>(p_stmt));
      break;
    case NodeType::RetStmt:
      generateRetStmt(std::dynamic_pointer_cast<RetStmt>(p_stmt));
      return true;
      // break;
    case NodeType::ExprStmt:
      generateExprStmt(std::dynamic_pointer_cast<ExprStmt>(p_stmt));
      break;
    case NodeType::AssignStmt:
      generateAssignStmt(std::dynamic_pointer_cast<AssignStmt>(p_stmt));
      break;
    case NodeType::IfStmt:
      p_stable->enterScope(std::format("if{}", if_cnt++), false);
      generateIfStmt(std::dynamic_pointer_cast<IfStmt>(p_stmt));
      p_stable->exitScope();
      break;
    case NodeType::WhileStmt:
      p_stable->enterScope(std::format("while{}", while_cnt++), false);
      generateWhileStmt(std::dynamic_pointer_cast<WhileStmt>(p_stmt));
      p_stable->exitScope();
      break;
    case NodeType::NullStmt:
      break;
    }
  }

  return false;
}

void
IrGenerator::generateVarDeclStmt(const VarDeclStmtPtr &p_vdstmt)
{
  // 变量声明语句只有两种形式
  // 1. let mut a;
  // 2. let mut a : i32;
  // 但是我们在声明变量时需要有明确的类型，这里假设在语义检查阶段已经完成了自动类型推导
  // 因此，变量声明语句对应的四元式就是明确且唯一的

  // 由于暂时只有 i32 类型，因此这里直接将所有变量声明为 i32 的
  // 这样假设的前提是所有异常都已经被检查出
  pushQuads(OpCode::Decl, getVarName(p_vdstmt->variable->name),
            std::string{"i32"}, NULL_OPERAND);
}

void
IrGenerator::generateRetStmt(const RetStmtPtr &p_rstmt)
{
  auto p_func = p_stable->lookupFunc(getFuncName());
  assert(p_func.has_value());

  std::string name;
  if (p_func.value()->retval_type == sym::VarType::Null) {
    assert(!p_rstmt->ret_val.has_value());
    name = "-";
  } else {
    name = generateExpr(p_rstmt->ret_val.value());
  }

  pushQuads(OpCode::Return, name, NULL_OPERAND, NULL_OPERAND);
}

void
IrGenerator::generateExprStmt(const ExprStmtPtr &p_estmt)
{
  // generateExpr(p_estmt->expr);

  // 对于表达式语句，只有调用表达式会需要生成
  // 其他表达式由于不可能产生副作用，因此可以不生成
  if (p_estmt->expr->type() == NodeType::CallExpr) {
    generateCallExpr(std::dynamic_pointer_cast<CallExpr>(p_estmt->expr));
  }
}

/**
 *
 * @return 返回表达式结果所存储的临时变量名
 */
std::string
IrGenerator::generateExpr(const ExprPtr &p_expr)
{
  switch (p_expr->type()) {
  default:
    throw std::runtime_error{"检查到不支持的表达式类型"};
  case NodeType::CallExpr:
    return generateCallExpr(std::dynamic_pointer_cast<CallExpr>(p_expr));
  case NodeType::ComparExpr:
    return generateComparExpr(std::dynamic_pointer_cast<ComparExpr>(p_expr));
  case NodeType::ArithExpr:
    return generateArithExpr(std::dynamic_pointer_cast<ArithExpr>(p_expr));
  case NodeType::Factor:
    return generateFactor(std::dynamic_pointer_cast<Factor>(p_expr));
  case NodeType::ParenthesisExpr:
  case NodeType::Number:
  case NodeType::Variable:
    return generateElement(p_expr);
  }
}

std::string
IrGenerator::generateCallExpr(const CallExprPtr &p_caexpr)
{
  // Step1. 获取函数符号指针
  std::string func_name = p_caexpr->callee;
  auto p_func = p_stable->lookupFunc(func_name);
  assert(p_func.has_value());

  // Step2. 检查函数是否有返回值
  std::string rv_name{"-"};
  if (p_func.value()->retval_type != sym::VarType::Null) {
    rv_name = p_stable->getTempValName();
  }

  // Step3. 为函数构造形参
  std::vector<std::string> argv;
  for (const auto &p_expr : p_caexpr->argv) {
    argv.push_back(generateExpr(p_expr));
  }

  for (const auto &arg : argv) {
    pushQuads(OpCode::Push, arg, NULL_OPERAND, NULL_OPERAND);
  }

  pushQuads(OpCode::Call, func_name, NULL_OPERAND, rv_name);

  return rv_name;
}

std::string
IrGenerator::generateComparExpr(const ComparExprPtr &p_coexpr)
{
  // 调用到该函数的情况都不是比较表达式作为控制条件的情况
  std::string lhs = generateExpr(p_coexpr->lhs);
  std::string rhs = generateExpr(p_coexpr->rhs);

  std::string rv_name = p_stable->getTempValName();
  OpCode op;
  switch (p_coexpr->op) {
  case ComparOperator::Equal:
    op = OpCode::Eq;
    break;
  case ComparOperator::Nequal:
    op = OpCode::Neq;
    break;
  case ComparOperator::Gequal:
    op = OpCode::Geq;
    break;
  case ComparOperator::Great:
    op = OpCode::Gne;
    break;
  case ComparOperator::Lequal:
    op = OpCode::Leq;
    break;
  case ComparOperator::Less:
    op = OpCode::Lne;
    break;
  }

  pushQuads(op, lhs, rhs, rv_name);
  return rv_name;
}

std::string
IrGenerator::generateArithExpr(const ArithExprPtr &p_aexpr)
{
  // 调用到该函数的情况都不是比较表达式作为控制条件的情况
  std::string lhs = generateExpr(p_aexpr->lhs);
  std::string rhs = generateExpr(p_aexpr->rhs);

  std::string rv_name = p_stable->getTempValName();
  OpCode op;
  switch (p_aexpr->op) {
  case ArithOperator::Add:
    op = OpCode::Add;
    break;
  case ArithOperator::Sub:
    op = OpCode::Sub;
    break;
  case ArithOperator::Mul:
    op = OpCode::Mul;
    break;
  case ArithOperator::Div:
    op = OpCode::Div;
    break;
  }

  pushQuads(op, lhs, rhs, rv_name);
  return rv_name;
}

void
IrGenerator::generateAssignStmt(const AssignStmtPtr &p_astmt)
{
  std::string rvalue_name = generateExpr(p_astmt->expr);
  auto p_lvalue = std::dynamic_pointer_cast<Variable>(p_astmt->lvalue);
  assert(p_lvalue);
  std::string lvalue_name = getVarName(p_lvalue->name);

  pushQuads(OpCode::Assign, rvalue_name, NULL_OPERAND, lvalue_name);
}

std::string
IrGenerator::generateFactor(const FactorPtr &p_factor)
{
  return generateElement(p_factor->element);
}

std::string
IrGenerator::generateElement(const ExprPtr &p_element)
{
  switch (p_element->type()) {
  default:
    throw std::runtime_error{"检查到不支持的 Element 类型"};
  case NodeType::ParenthesisExpr:
    return generateParenthesisExpr(std::dynamic_pointer_cast<ParenthesisExpr>(p_element));
  case NodeType::Number:
    return generateNumber(std::dynamic_pointer_cast<Number>(p_element));
  case NodeType::Variable:
    return generateVariable(std::dynamic_pointer_cast<Variable>(p_element));
  case NodeType::CallExpr:
    return generateCallExpr(std::dynamic_pointer_cast<CallExpr>(p_element));
  }
}

std::string
IrGenerator::generateParenthesisExpr(const ParenthesisExprPtr &p_pexpr)
{
  return generateExpr(p_pexpr->expr);
}

std::string
IrGenerator::generateNumber(const NumberPtr &p_number)
{
  // auto tv_name = p_stable->getTempValName();
  // pushQuads(OpCode::Assign, std::format("{}", p_number->value), NULL_OPERAND,
  // tv_name); return tv_name;
  return std::to_string(p_number->value);
}

std::string
IrGenerator::generateVariable(const VariablePtr &p_variable)
{
  return getVarName(p_variable->name);
}

// 假定标号支持前向声明
void
IrGenerator::generateIfStmt(const IfStmtPtr &p_istmt)
{
  std::string label_true =
      std::format("{}_true", replaceScopeQualifiers(p_stable->getCurScope()));
  std::string label_false =
      std::format("{}_false", replaceScopeQualifiers(p_stable->getCurScope()));
  std::string label_end =
      std::format("{}_end", replaceScopeQualifiers(p_stable->getCurScope()));

  std::string lhs;
  std::string rhs;
  OpCode op; // 跳转到 true

  // 如果 if 语句的判断条件并非比较表达式，则使用 jne condition 0 来跳转到 if
  // 分支
  std::string scope = p_stable->exitScope();
  if (p_istmt->expr->type() != NodeType::ComparExpr) {
    lhs = generateExpr(p_istmt->expr);
    rhs = "0";
    op = OpCode::Jne;
  } else {
    auto p_coexpr = std::dynamic_pointer_cast<ComparExpr>(p_istmt->expr);
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
    pushQuads(OpCode::Goto, label_false, NULL_OPERAND, NULL_OPERAND);
  } else {
    pushQuads(OpCode::Goto, label_end, NULL_OPERAND, NULL_OPERAND);
  }

  pushQuads(OpCode::Label, label_true, NULL_OPERAND, NULL_OPERAND);
  generateBlockStmt(p_istmt->if_branch);

  if (has_else) {
    pushQuads(OpCode::Goto, label_end, NULL_OPERAND, NULL_OPERAND);
    pushQuads(OpCode::Label, label_false, NULL_OPERAND, NULL_OPERAND);
    // 同样是因为没有 else if 所以下面的断言才成立
    assert(!p_istmt->else_clauses[0]->expr.has_value());
    generateBlockStmt(p_istmt->else_clauses[0]->block);
  }
  pushQuads(OpCode::Label, label_end, NULL_OPERAND, NULL_OPERAND);
}

void
IrGenerator::generateWhileStmt(const WhileStmtPtr &p_wstmt)
{
  std::string label_start =
      std::format("{}_start", replaceScopeQualifiers(p_stable->getCurScope()));
  std::string label_end =
      std::format("{}_end", replaceScopeQualifiers(p_stable->getCurScope()));

  pushQuads(OpCode::Label, label_start, NULL_OPERAND, NULL_OPERAND);

  std::string lhs;
  std::string rhs;
  OpCode op; // 跳转到 true

  std::string scope = p_stable->exitScope();
  if (p_wstmt->expr->type() != NodeType::ComparExpr) {
    lhs = generateExpr(p_wstmt->expr);
    rhs = "0";
    op = OpCode::Jeq;
  } else {
    auto p_coexpr = std::dynamic_pointer_cast<ComparExpr>(p_wstmt->expr);
    lhs = generateExpr(p_coexpr->lhs);
    rhs = generateExpr(p_coexpr->rhs);

    switch (p_coexpr->op) {
    case ComparOperator::Equal:
      op = OpCode::Jne;
      break;
    case ComparOperator::Nequal:
      op = OpCode::Jeq;
      break;
    case ComparOperator::Gequal:
      op = OpCode::Jlt;
      break;
    case ComparOperator::Great:
      op = OpCode::Jle;
      break;
    case ComparOperator::Lequal:
      op = OpCode::Jgt;
      break;
    case ComparOperator::Less:
      op = OpCode::Jge;
      break;
    }
  }
  p_stable->enterScope(scope, false);

  pushQuads(op, lhs, rhs, label_end);

  generateBlockStmt(p_wstmt->block);

  pushQuads(OpCode::Goto, label_start, NULL_OPERAND, NULL_OPERAND);
  pushQuads(OpCode::Label, label_end, NULL_OPERAND, NULL_OPERAND);
}

static std::string
opCode2Str(OpCode op)
{
  switch (op) {
  case OpCode::Add:
    return "+";
  case OpCode::Sub:
    return "-";
  case OpCode::Mul:
    return "*";
  case OpCode::Div:
    return "/";

  case OpCode::Jeq:
    return "j=";
  case OpCode::Jne:
    return "j!=";
  case OpCode::Jge:
    return "j>=";
  case OpCode::Jgt:
    return "j>";
  case OpCode::Jle:
    return "j<=";
  case OpCode::Jlt:
    return "j<";

  case OpCode::Eq:
    return "==";
  case OpCode::Neq:
    return "!=";
  case OpCode::Geq:
    return ">=";
  case OpCode::Gne:
    return ">";
  case OpCode::Leq:
    return "<=";
  case OpCode::Lne:
    return "<";

  case OpCode::Decl:
    return "decl";
  case OpCode::Assign:
    return "=";

  case OpCode::Label:
    return "label";
  case OpCode::Goto:
    return "goto";

  case OpCode::Push:
    return "push";
  case OpCode::Pop:
    return "pop";
  case OpCode::Call:
    return "call";
  case OpCode::Return:
    return "return";
  }
}

void
IrGenerator::printQuads(std::ofstream &out) const
{
  for (const auto &quad : quads) {
    out << std::format(
      "({}, {}, {}, {})",
      opCode2Str(quad.op),
      quad.arg1.name,
      quad.arg2.name,
      quad.res.name
    ) << std::endl;
  }
}

} // namespace ir
