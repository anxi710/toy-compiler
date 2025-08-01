#!/usr/bin/perl
use strict;
use warnings;

my @nodes = (
    "Prog",
    "Arg",
    "FuncHeaderDecl",
    "FuncDecl",
    "EmptyStmt",
    "VarDeclStmt",
    "EmptyExpr",
    "BreakExpr",
    "ContinueExpr",
    "ExprStmt",
    "RetExpr",
    "BracketExpr",
    "Variable",
    "AssignElem",
    "ArrAcc",
    "TupAcc",
    "Number",
    "ArrElems",
    "TupElems",
    "AssignExpr",
    "CmpExpr",
    "AriExpr",
    "CallExpr",
    "ElseClause",
    "IfExpr",
    "LoopExpr",
    "WhileLoopExpr",
    "RangeExpr",
    "IterableVal",
    "ForLoopExpr",
    "StmtBlockExpr",
);

print <<"HEADER";
// clang-format off
// Generated by generate_accept.pl
// **This file is auto-generated. Do not edit manually.**

#include "ast.hpp"
#include "oop_visitor.hpp"

namespace ast {
HEADER

foreach my $node (@nodes) {
    print <<"FUNC";

void
${node}::accept(OOPVisitor &visitor)
{
  visitor.visit(*this);
}
FUNC
}

print "\n} // namespace ast\n";
