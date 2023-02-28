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


void ExprVisitor::expr(Expr* expr)
{
  switch (expr->mType) {
  case Expr::Type::WithBlock:
    this->exprWithBlock(expr->as<ExprWithBlock>());
    break;
  case Expr::Type::WithoutBlock:
    this->exprWithoutBlock(expr->as<ExprWithoutBlock>());
    break;
  }
}
void ExprVisitor::exprWithoutBlock(ExprWithoutBlock* expr)
{
  switch (expr->mType) {
  case ExprWithoutBlock::Type::Literal:
    this->literalExpr(expr->as<LiteralExpr>());
    break;
  case ExprWithoutBlock::Type::Grouped:
    this->groupedExpr(expr->as<GroupedExpr>());
    break;
  case ExprWithoutBlock::Type::Operator:
    this->operatorExpr(expr->as<OperatorExpr>());
    break;
  }
}

void ExprVisitor::operatorExpr(OperatorExpr* expr)
{
  switch (expr->mType) {
  case OperatorExpr::Type::ArithmeticOrLogical:
    this->arithmeticOrLogicalExpr(expr->as<ArithmeticOrLogicalExpr>());
    break;
  case OperatorExpr::Type::Negation:
    this->negationExpr(expr->as<NegationExpr>());
    break;
  case OperatorExpr::Type::Comparison:
    this->comparisonExpr(expr->as<ComparisonExpr>());
    break;
  }
}

/* void ExprVisitor::literalExpr(LiteralExpr* expr) {}
void ExprVisitor::groupedExpr(GroupedExpr* expr) {}
void ExprVisitor::comparisonExpr(ComparisonExpr* expr) {}
void ExprVisitor::negationExpr(NegationExpr* expr) {}
void ExprVisitor::arithmeticOrLogicalExpr(ArithmeticOrLogicalExpr* expr) {}
 */
void ExprVisitor::exprWithBlock(ExprWithBlock* expr) {}