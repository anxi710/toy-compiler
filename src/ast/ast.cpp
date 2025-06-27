#include "ast.hpp"

namespace par::ast {

// Node
void
Node::setPos(std::size_t row, std::size_t col)
{
  pos = util::Position{row, col};
}

void
Node::setPos(util::Position pos)
{
  this->pos = pos;
}

util::Position
Node::getPos() const
{
  return pos;
}

// virtual function: type
NodeType
Decl::type() const
{
  return NodeType::Decl;
}

NodeType
Prog::type() const
{
  return NodeType::Prog;
}

NodeType
VarDeclBody::type() const
{
  return NodeType::VarDeclBody;
}

NodeType
VarType::type() const
{
  return NodeType::VarType;
}

NodeType
Integer::type() const
{
  return NodeType::Integer;
}

NodeType
Array::type() const
{
  return NodeType::Array;
}

NodeType
Tuple::type() const
{
  return NodeType::Tuple;
}

NodeType
Arg::type() const
{
  return NodeType::Arg;
}

NodeType
Stmt::type() const
{
  return NodeType::Stmt;
}

NodeType
BlockStmt::type() const
{
  return NodeType::BlockStmt;
}

NodeType
FuncHeaderDecl::type() const
{
  return NodeType::FuncHeaderDecl;
}

NodeType
FuncDecl::type() const
{
  return NodeType::FuncDecl;
}

NodeType
Expr::type() const
{
  return NodeType::Expr;
}

NodeType
ExprStmt::type() const
{
  return NodeType::ExprStmt;
}

NodeType
ParenthesisExpr::type() const
{
  return NodeType::ParenthesisExpr;
}

NodeType
AssignElement::type() const
{
  return NodeType::AssignElement;
}

NodeType
Variable::type() const
{
  return NodeType::Variable;
}

NodeType
Dereference::type() const
{
  return NodeType::Dereference;
}

NodeType
ArrayAccess::type() const
{
  return NodeType::ArrayAccess;
}

NodeType
TupleAccess::type() const
{
  return NodeType::TupleAccess;
}

NodeType
Number::type() const
{
  return NodeType::Number;
}

NodeType
Factor::type() const
{
  return NodeType::Factor;
}

NodeType
ArrayElements::type() const
{
  return NodeType::ArrayElements;
}

NodeType
TupleElements::type() const
{
  return NodeType::TupleElements;
}

NodeType
RetStmt::type() const
{
  return NodeType::RetStmt;
}

NodeType
VarDeclStmt::type() const
{
  return NodeType::VarDeclStmt;
}

NodeType
AssignStmt::type() const
{
  return NodeType::AssignStmt;
}

NodeType
VarDeclAssignStmt::type() const
{
  return NodeType::VarDeclAssignStmt;
}

NodeType
ComparExpr::type() const
{
  return NodeType::ComparExpr;
}

NodeType
ArithExpr::type() const
{
  return NodeType::ArithExpr;
}

NodeType
CallExpr::type() const
{
  return NodeType::CallExpr;
}

NodeType
ElseClause::type() const
{
  return NodeType::ElseClause;
}

NodeType
IfStmt::type() const
{
  return NodeType::IfStmt;
}

NodeType
WhileStmt::type() const
{
  return NodeType::WhileStmt;
}

NodeType
ForStmt::type() const
{
  return NodeType::ForStmt;
}

NodeType
LoopStmt::type() const
{
  return NodeType::LoopStmt;
}

NodeType
BreakStmt::type() const
{
  return NodeType::BreakStmt;
}

NodeType
ContinueStmt::type() const
{
  return NodeType::ContinueStmt;
}

NodeType
NullStmt::type() const
{
  return NodeType::NullStmt;
}

NodeType
FuncExprBlockStmt::type() const
{
  return NodeType::FuncExprBlockStmt;
}

NodeType
IfExpr::type() const
{
  return NodeType::IfExpr;
}

}
