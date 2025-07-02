#include "ast.hpp"
#include "visitor.hpp"

namespace ast {

void
Prog::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
Arg::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
BlockStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
FuncHeaderDecl::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
FuncDecl::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
ExprStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
BracketExpr::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
AssignElem::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
Variable::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
ArrayAccess::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
TupleAccess::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
Number::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
Factor::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
ArrayElems::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
TupleElems::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
RetStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
VarDeclStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
AssignStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
CmpExpr::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
AriExpr::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
CallExpr::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
ElseClause::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
IfStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
WhileStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
ForStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
LoopStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
BreakStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
ContinueStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
NullStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
FuncExprBlockStmt::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

void
IfExpr::accept(visit::NodeVisitor& visitor)
{
  visitor.visit(*this);
}

} // namespace ast
