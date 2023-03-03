#include "Syntax.hpp"

std::map<ArithmeticOrLogicalExpr::Kind, i32> ArithmeticOrLogicalExpr::sPrecedence = {
    {Kind::Add, 10},   {Kind::Sub, 10},  {Kind::Mul, 11},   {Kind::Div, 11}, {Kind::Rem, 11},
    {Kind::BitAnd, 8}, {Kind::BitOr, 6}, {Kind::BitXor, 7}, {Kind::Shl, 9},  {Kind::Shr, 9}};

auto ArithmeticOrLogicalExpr::GetPrec(Kind type) -> i32
{
  if (auto iter = sPrecedence.find(type); iter != sPrecedence.end()) {
    return iter->second;
  } else {
    printf("%d", type);
    assert(0);
    return {};
  }
}

auto ArithmeticOrLogicalExpr::MapOp(TokenKind type) -> Kind
{
  switch (type) {
    using enum ArithmeticOrLogicalExpr::Kind;
  case PunPlus:
    return Add;
  case PunMinus:
    return Sub;
  case PunStar:
    return Mul;
  case PunSlash:
    return Div;
  case PunPercent:
    return Rem;
  case PunAnd:
    return BitAnd;
  case PunOr:
    return BitOr;
  case PunCaret:
    return BitXor;
  case PunShl:
    return Shl;
  case PunShr:
    return Shr;
  default:
    return SIZE;
  }
}

auto NegationExpr::MapOp(TokenKind token) -> Kind
{
  switch (token) {
  case PunNot:
    return Kind::Not;
  case PunMinus:
    return Kind::Neg;
  default:
    return Kind::SIZE;
  }
}
auto NegationExpr::GetPrec(Kind type) -> i32
{
  switch (type) {
  case Kind::Not:
  case Kind::Neg:
    return 13;
  default:
    assert(0);
    return {};
  }
}

auto ComparisonExpr::MapOp(TokenKind token) -> Kind
{
  switch (token) {
  case PunEqEq:
    return Kind::Eq;
  case PunNe:
    return Kind::Ne;
  case PunGt:
    return Kind::Gt;
  case PunLt:
    return Kind::Lt;
  case PunGe:
    return Kind::Ge;
  case PunLe:
    return Kind::Le;
  default:
    return Kind::SIZE;
  }
}
auto ComparisonExpr::GetPrec(Kind type) -> i32
{
  switch (type) {
  case Kind::Eq:
  case Kind::Ne:
  case Kind::Gt:
  case Kind::Lt:
  case Kind::Ge:
  case Kind::Le:
    return 13;
  default:
    assert(0);
    return {};
  }
}

void StmtVisitor::visitStmt(Stmt* stmt)
{
  switch (stmt->mType) {
  case Stmt::Type::Item:
    assert(0);
    break;
  case Stmt::Type::Let:
    this->visit(stmt->as<LetStmt>());
    break;
  case Stmt::Type::Expression:
    this->visit(stmt->as<ExprStmt>());
    break;
  }
}

void ExprVisitor::visitExpr(Expr* expr)
{
  switch (expr->mType) {
  case Expr::Type::WithBlock:
    this->visitExprWithBlock(expr->as<ExprWithBlock>());
    break;
  case Expr::Type::WithoutBlock:
    this->visitExprWithoutBlock(expr->as<ExprWithoutBlock>());
    break;
  }
}

void ExprVisitor::visitExprWithoutBlock(ExprWithoutBlock* expr)
{
  switch (expr->mType) {
  case ExprWithoutBlock::Type::Literal:
    this->visit(expr->as<LiteralExpr>());
    break;
  case ExprWithoutBlock::Type::Grouped:
    this->visit(expr->as<GroupedExpr>());
    break;
  case ExprWithoutBlock::Type::Operator:
    this->visitOperatorExpr(expr->as<OperatorExpr>());
    break;
  }
}

void ExprVisitor::visitOperatorExpr(OperatorExpr* expr)
{
  switch (expr->mType) {
  case OperatorExpr::Type::ArithmeticOrLogical:
    this->visit(expr->as<ArithmeticOrLogicalExpr>());
    break;
  case OperatorExpr::Type::Negation:
    this->visit(expr->as<NegationExpr>());
    break;
  case OperatorExpr::Type::Comparison:
    this->visit(expr->as<ComparisonExpr>());
    break;
  case OperatorExpr::Type::Assignment:
    this->visit(expr->as<AssignmentExpr>());
    break;
  }
}

void ExprVisitor::visitExprWithBlock(ExprWithBlock* expr)
{
  switch (expr->mType) {
  case ExprWithBlock::Type::Block:
    this->visit(expr->as<BlockExpr>());
    break;
  case ExprWithBlock::Type::Loop:
    this->visitLoopExpr(expr->as<LoopExpr>());
    break;
  case ExprWithBlock::Type::If:
    this->visit(expr->as<IfExpr>());
    break;
  case ExprWithBlock::Type::IfLet:
    assert(0);
    break;
  case ExprWithBlock::Type::Match:
    assert(0);
    break;
  }
}

void ExprVisitor::visitLoopExpr(LoopExpr* expr)
{
  switch (expr->mType) {
  case LoopExpr::Type::InfiniteLoop:
    this->visit(expr->as<InfiniteLoopExpr>());
    break;
  case LoopExpr::Type::PredicateLoop:
    this->visit(expr->as<PredicateLoopExpr>());
    break;
  }
}

void StringifyExpr::visit(LiteralExpr* expr) { str += ToString(expr->mValue); };
void StringifyExpr::visit(GroupedExpr* expr)
{
  str += '(';
  this->visitExpr(expr->mExpr.get());
  str += ')';
};
void StringifyExpr::visit(ComparisonExpr* expr)
{
  this->visitExpr(expr->mLeft.get());
  switch (expr->mKind) {
  case ComparisonExpr::Kind::Eq:
    str += "==";
    break;
  case ComparisonExpr::Kind::Ne:
    str += "!=";
    break;
  case ComparisonExpr::Kind::Gt:
    str += ">";
    break;
  case ComparisonExpr::Kind::Lt:
    str += "<";
    break;
  case ComparisonExpr::Kind::Ge:
    str += ">=";
    break;
  case ComparisonExpr::Kind::Le:
    str += "<=";
    break;
  case ComparisonExpr::Kind::SIZE:
    assert(0);

    break;
  }
  this->visitExpr(expr->mRight.get());
};

void StringifyExpr::visit(NegationExpr* expr)
{
  switch (expr->mKind) {
  case NegationExpr::Kind::Neg:
    str += '-';
    break;
  case NegationExpr::Kind::Not:
    str += '!';
    break;
  case NegationExpr::Kind::SIZE:
    assert(0);

    break;
  }
  this->visitExpr(expr->mRight.get());
};
void StringifyExpr::visit(ArithmeticOrLogicalExpr* expr)
{
  this->visitExpr(expr->mLeft.get());
  switch (expr->mKind) {
  case ArithmeticOrLogicalExpr::Kind::Add:
    str += "+";
    break;
  case ArithmeticOrLogicalExpr::Kind::Sub:
    str += "-";
    break;
  case ArithmeticOrLogicalExpr::Kind::Mul:
    str += "*";
    break;
  case ArithmeticOrLogicalExpr::Kind::Div:
    str += "/";
    break;
  case ArithmeticOrLogicalExpr::Kind::Rem:
    str += "%";
    break;
  case ArithmeticOrLogicalExpr::Kind::BitAnd:
    str += "&";
    break;
  case ArithmeticOrLogicalExpr::Kind::BitOr:
    str += "|";
    break;
  case ArithmeticOrLogicalExpr::Kind::BitXor:
    str += "^";
    break;
  case ArithmeticOrLogicalExpr::Kind::Shl:
    str += "<<";
    break;
  case ArithmeticOrLogicalExpr::Kind::Shr:
    str += ">>";
    break;
  case ArithmeticOrLogicalExpr::Kind::SIZE:
    assert(0);
    break;
  }
  this->visitExpr(expr->mRight.get());
};

void StringifyExpr::visit(AssignmentExpr* expr)
{
  this->visitExpr(expr->mLeft.get());
  str += '=';
  this->visitExpr(expr->mRight.get());
}

void StringifyExpr::visit(BlockExpr* expr)
{
  str += '{';
  for (auto& stmt : expr->mStmts) {
    mStmtVisitor.visitStmt(stmt.get());
  }
  this->visitExprWithoutBlock(expr->mReturn.get());
  str += '{';
};
void StringifyExpr::visit(IfExpr* expr)
{
  str += "if ";
  this->visitExpr(expr->mCond.get());
  str += '{';
  this->visit(expr->mIf.get());
  str += '}';
  if (expr->mElse != nullptr) {
    str += " else ";
    this->visitExprWithBlock(expr->mElse.get());
  }
}

void StringifyExpr::visit(InfiniteLoopExpr* expr)
{
  str += "loop";
  this->visit(expr->mExpr.get());
}
void StringifyExpr::visit(PredicateLoopExpr* expr)
{
  str += "while ";
  this->visitExpr(expr->mCond.get());
  this->visit(expr->mExpr.get());
}