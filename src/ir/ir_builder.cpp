#include <ranges>
#include <vector>

#include "ir_builder.hpp"

namespace ir {

static std::vector<IRQuadPtr>
concat(const std::vector<std::vector<IRQuadPtr>> &codes)
{
  std::size_t size = 0;
  for (const auto &code : codes) {
    size += code.size();
  }

  std::vector<IRQuadPtr> res;
  res.reserve(size);

  for (const auto &code : codes) {
    res.insert(res.end(), code.begin(), code.end());
  }

  return res;
}

static std::vector<IRQuadPtr>
concat(std::vector<std::vector<IRQuadPtr>> &&codes)
{
  std::size_t size = 0;
  for (const auto &code : codes) {
    size += code.size();
  }

  std::vector<IRQuadPtr> res;
  res.reserve(size);

  for (const auto &code : codes) {
    res.insert(
      res.end(),
      std::make_move_iterator(code.begin()),
      std::make_move_iterator(code.end())
    );
  }

  return res;
}

void
IRBuilder::visit(ast::Prog &prog)
{
  auto codes = prog.decls
    | std::views::transform([](const auto &decl) {
        return decl->ircode;
      })
    | std::ranges::to<std::vector<std::vector<IRQuadPtr>>>();

  prog.ircode = concat(std::move(codes));
}

void
IRBuilder::visit(ast::FuncDecl &fdecl)
{
  fdecl.ircode = concat({fdecl.header->ircode, fdecl.body->ircode});
}

void
IRBuilder::visit(ast::FuncHeaderDecl &fhdecl)
{
  auto quad = std::make_shared<IRQuad>(IROp::FUNC, nullptr, nullptr, fhdecl.name);
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
  auto codes = sbexpr.stmts
    | std::views::transform([](const auto &stmt) {
        return stmt->ircode;
      })
    | std::ranges::to<std::vector<std::vector<IRQuadPtr>>>();

  sbexpr.ircode = concat(std::move(codes));
}

void
IRBuilder::visit(ast::VarDeclStmt &vdstmt)
{
}

void
IRBuilder::visit(ast::ExprStmt &estmt)
{
}

void
IRBuilder::visit(ast::RetExpr &rexpr)
{
}

void
IRBuilder::visit(ast::BreakExpr &bexpr)
{
}

void
IRBuilder::visit(ast::ContinueExpr &cexpr)
{
}

void
IRBuilder::visit(ast::AssignExpr &aexpr)
{
}

void
IRBuilder::visit(ast::AssignElem &aelem)
{
}

void
IRBuilder::visit(ast::Variable &var)
{
}

void
IRBuilder::visit(ast::ArrAcc &aacc)
{
}

void
IRBuilder::visit(ast::TupAcc &tacc)
{
}

void
IRBuilder::visit(ast::CmpExpr &cexpr)
{
}

void
IRBuilder::visit(ast::AriExpr &aexpr)
{
}

void
IRBuilder::visit(ast::ArrElems &aelems)
{
}

void
IRBuilder::visit(ast::TupElems &telems)
{
}

void
IRBuilder::visit(ast::Number &num)
{
}

void
IRBuilder::visit(ast::BracketExpr &bexpr)
{
}

void
IRBuilder::visit(ast::CallExpr &cexpr)
{
}

void
IRBuilder::visit(ast::IfExpr &iexpr)
{
}

void
IRBuilder::visit(ast::ElseClause &eclause)
{
}

void
IRBuilder::visit(ast::WhileLoopExpr&wlexpr)
{
}

void
IRBuilder::visit(ast::ForLoopExpr &flexpr)
{
}

void
IRBuilder::visit(ast::Interval &interval)
{
}

void
IRBuilder::visit(ast::IterableVal &iter)
{
}

void
IRBuilder::visit(ast::LoopExpr&lexpr)
{
}


} // namespace ir
