#include "ast.hpp"
#include "visitor.hpp"

namespace ast {

void
Prog::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
Type::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
Arg::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
FuncHeaderDecl::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
FuncDecl::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
EmptyStmt::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
VarDeclStmt::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
BreakExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
ContinueExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
ExprStmt::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
RetExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
BracketExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
Variable::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
AssignElem::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
ArrayAccess::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
TupleAccess::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
Number::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
ArrayElems::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
TupleElems::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
AssignExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
CmpExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
AriExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
CallExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
ElseClause::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
IfExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
LoopExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
WhileLoopExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
Interval::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
IterableVal::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
ForLoopExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

void
StmtBlockExpr::accept(NodeVisitor &visitor)
{
  visitor.visit(*this);
}

} // namespace ast
