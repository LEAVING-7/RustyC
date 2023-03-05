#include "Syntax.hpp"

auto BinaryExpr::BindingPower(BinaryExpr::Kind kind) -> std::tuple<i32, i32>
{
  switch (kind) {
  case Kind::Add:
  case Kind::Sub:
    return {20, 21};
  case Kind::Mul:
  case Kind::Div:
  case Kind::Rem:
    return {22, 23};
  case Kind::BitAnd:
    return {16, 17};
  case Kind::BitOr:
    return {12, 13};
  case Kind::BitXor:
    return {14, 15};
  case Kind::Shl:
  case Kind::Shr:
    return {18, 19};
  case Kind::Eq:
  case Kind::Ne:
  case Kind::Gt:
  case Kind::Lt:
  case Kind::Ge:
  case Kind::Le:
    return {10, 11};
  case Kind::Assignment:
    return {3, 2};
  case Kind::SIZE:
    assert(0);
    return {};
  }
}
auto BinaryExpr::MapKind(TokenKind tok) -> BinaryExpr::Kind
{
  switch (tok) {
  case PunPlus:
    return Kind::Add;
  case PunMinus:
    return Kind::Sub;
  case PunStar:
    return Kind::Mul;
  case PunSlash:
    return Kind::Div;
  case PunPercent:
    return Kind::Rem;
  case PunCaret:
    return Kind::BitXor;
  case PunAnd:
    return Kind::BitAnd;
  case PunOr:
    return Kind::BitOr;
  case PunAndAnd:
  case PunOrOr:
    assert(0);
    return {};
  case PunShl:
    return Kind::Shl;
  case PunShr:
    return Kind::Shr;
  case PunEq:
    return Kind::Assignment;
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
    return BinaryExpr::Kind::SIZE;
  }
}

auto UnaryExpr::BindingPower(UnaryExpr::Kind kind) -> std::tuple<i32>
{
  switch (kind) {
  case Kind::Neg:
  case Kind::Not:
    return {26};
  case Kind::SIZE:
    assert(0);
    return {};
  }
}
auto UnaryExpr::MapKind(TokenKind tok) -> UnaryExpr::Kind
{
  switch (tok) {
  case PunNot:
    return Kind::Not;
  case PunMinus:
    return Kind::Neg;
  default:
    return Kind::SIZE;
  }
}

void ExprVisitor::visitExpr(Expr* expr)
{
  switch (expr->mType) {
  case Expr::Type::WithBlock:
    return this->visitExprWithBlock(expr->as<ExprWithBlock>());
  case Expr::Type::WithoutBlock:
    return this->visitExprWithoutBlock(expr->as<ExprWithoutBlock>());
  }
}

void ExprVisitor::visitExprWithoutBlock(ExprWithoutBlock* expr)
{
  switch (expr->mType) {
  case ExprWithoutBlock::Type::Literal:
    return this->visit(expr->as<LiteralExpr>());
  case ExprWithoutBlock::Type::Grouped:
    return this->visit(expr->as<GroupedExpr>());
  case ExprWithoutBlock::Type::Operator:
    return visitOperatorExpr(expr->as<OperatorExpr>());
  case ExprWithoutBlock::Type::Call:
    return visit(expr->as<CallExpr>());
  case ExprWithoutBlock::Type::Return:
    return visit(expr->as<ReturnExpr>());
  }
  utils::Unimplemented(utils::SrcLoc::current());
}

void ExprVisitor::visitOperatorExpr(OperatorExpr* expr)
{
  switch (expr->mType) {
  case OperatorExpr::Type::Unary:
    return this->visit(expr->as<UnaryExpr>());
  case OperatorExpr::Type::Binary:
    return this->visit(expr->as<BinaryExpr>());
  }
}

void ExprVisitor::visitExprWithBlock(ExprWithBlock* expr)
{
  switch (expr->mType) {
  case ExprWithBlock::Type::Block:
    return this->visit(expr->as<BlockExpr>());
  case ExprWithBlock::Type::Loop:
    return this->visitLoopExpr(expr->as<LoopExpr>());
  case ExprWithBlock::Type::If:
    return this->visit(expr->as<IfExpr>());
  case ExprWithBlock::Type::IfLet:
    return assert(0);
  case ExprWithBlock::Type::Match:
    return assert(0);
  }
}

void ExprVisitor::visitLoopExpr(LoopExpr* expr)
{
  switch (expr->mType) {
  case LoopExpr::Type::InfiniteLoop:
    return this->visit(expr->as<InfiniteLoopExpr>());
  case LoopExpr::Type::PredicateLoop:
    return this->visit(expr->as<PredicateLoopExpr>());
  }
};

void StmtVisitor::visitStmt(Stmt* stmt)
{
  if (stmt == nullptr) {
    return;
  }
  switch (stmt->mType) {
  case Stmt::Type::Let:
    this->visit(stmt->as<LetStmt>());
    break;
  case Stmt::Type::Expression:
    this->visit(stmt->as<ExprStmt>());
    break;
  default:
    utils::Unreachable(utils::SrcLoc::current());
  }
}

void StmtVisitor::visitItem(Item* item)
{
  switch (item->mKind) {
  case Item::Kind::Function:
    this->visit(item->as<FunctionItem>());
    break;
  default:
    utils::Unimplemented(utils::SrcLoc::current());
  }
}

//===----------------------------------------------------------------------===//
// StringifyExpr
//===----------------------------------------------------------------------===//

void StringifyExpr::visit(LiteralExpr* expr) { str += ToString(expr->mValue); }
void StringifyExpr::visit(GroupedExpr* expr)
{
  str += '(';
  this->visitExpr(expr->mExpr.get());
  str += ')';
}
void StringifyExpr::visit(UnaryExpr* expr)
{
  switch (expr->mKind) {
  case UnaryExpr::Kind::Neg:
    str += '-';
    return visitExpr(expr->mRight.get());
  case UnaryExpr::Kind::Not:
    str += '!';
    return visitExpr(expr->mRight.get());
  case UnaryExpr::Kind::SIZE:
    assert(0);
    break;
  }
}
void StringifyExpr::visit(BinaryExpr* expr)
{
  std::string c{};
  switch (expr->mKind) {
#define CASE(kind, str)                                                                                                \
  case BinaryExpr::Kind::kind:                                                                                         \
    c = #str;                                                                                                          \
    break;

    CASE(Add, +)
    CASE(Sub, -)
    CASE(Mul, *)
    CASE(Div, /)
    CASE(Rem, %)
    CASE(BitAnd, &)
    CASE(BitOr, |)
    CASE(BitXor, ^)
    CASE(Shl, <<)
    CASE(Shr, >>)
    CASE(Eq, ==)
    CASE(Ne, !=)
    CASE(Gt, >)
    CASE(Lt, <)
    CASE(Ge, >=)
    CASE(Le, <=)
    CASE(Assignment, =)
#undef CASE
  case BinaryExpr::Kind::SIZE:
    assert(0);
    break;
  }
  visitExpr(expr->mLeft.get());
  str += c;
  visitExpr(expr->mRight.get());
}
void StringifyExpr::visit(BlockExpr* expr)
{
  str += '{';
  for (auto& stmt : expr->mStmts) {
    mStmtVisitor.visitStmt(stmt.get());
  }
  str += '}';
}
void StringifyExpr::visit(IfExpr* expr)
{
  str += "if ";
  this->visitExpr(expr->mCond.get());
  this->visit(expr->mThen.get());
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

void StringifyExpr::visit(CallExpr* expr)
{
  str += expr->mCallee;
  str += '(';
  for (i32 i = 0; i < expr->mArgs.size(); ++i) {
    this->visitExpr(expr->mArgs[i].get());
    if (i != expr->mArgs.size() - 1) {
      str += ',';
    }
  }
  str += ')';
}

void StringifyExpr::visit(ReturnExpr* expr)
{
  str += "return ";
  this->visitExpr(expr->mExpr.get());
}

void StringifyStmt::visit(ExprStmt* stmt)
{
  mExprVisitor.visitExpr(stmt->mExpr.get());
  str += ';';
}
void StringifyStmt::visit(LetStmt* stmt)
{
  str += "let ";
  str += stmt->mName;
  str += '=';
  mExprVisitor.visitExpr(stmt->mExpr.get());
  str += ';';
};
void StringifyStmt::visit(FunctionItem* item)
{
  str += "fn ";
  str += item->mName;
  str += '(';
  for (i32 i = 0; i < item->mParamNames.size(); ++i) {
    str += item->mParamNames[i];
    str += ':';
    str += TypeToString(item->mFnType->mParams[i].get());
    if (i != item->mParamNames.size() - 1) {
      str += ',';
    }
  }
  str += ')';
  if (item->mFnType->mRet) {
    str += "->";
    str += TypeToString(item->mFnType->mRet.get());
  }
  mExprVisitor.visitExpr(item->mBody.get());
}