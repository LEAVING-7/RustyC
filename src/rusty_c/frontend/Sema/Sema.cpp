#include "Sema.hpp"
#include "utils/utils.hpp"

auto Sema::actOnCrate(Crate const* crate) -> void
{
  auto guard = enterScope();
  for (auto& item : crate->mItems) {
    actOnItem(item.get());
  }
}

auto Sema::actOnExprWithBlock(ExprWithBlock* expr) -> std::unique_ptr<TypeBase>
{
  switch (expr->mType) {
  case ExprWithBlock::Type::Block:
    return actOnBlockExpr(expr->as<BlockExpr>());
  case ExprWithBlock::Type::Loop:
    return actOnLoopExpr(expr->as<LoopExpr>());
  case ExprWithBlock::Type::If:
    return actOnIfExpr(expr->as<IfExpr>());
  case ExprWithBlock::Type::IfLet:
  case ExprWithBlock::Type::Match:
    utils::Unimplemented(utils::SrcLoc::current(), "IfLet and Match are not implemented yet");
    break;
  }
  utils::Unreachable(utils::SrcLoc::current());
}
auto Sema::actOnBlockExpr(BlockExpr* expr) -> std::unique_ptr<TypeBase>
{
  auto guard = enterScope();
  for (auto& item : expr->mItems) {
    actOnItem(item.get());
  }
  for (auto& stmt : expr->mStmts) {
    actOnStmt(stmt.get());
  }

  if (!expr->mStmts.empty() && expr->mStmts.back()->mType == Stmt::Type::Expression) {
    auto e = expr->mStmts.back()->as<ExprStmt>();
    if (e->mExpr->mType == Expr::Type::WithoutBlock) {
      auto k = e->mExpr->as<ExprWithoutBlock>();
      if (k->mType == ExprWithoutBlock::Type::Return) {
        return actOnExpr(k->as<ReturnExpr>()->mExpr.get());
      }
    }
  }

  if (expr->mReturn) {
    return actOnExpr(expr->mReturn.get());
  } else {
    return std::make_unique<TupleType>(); // return unit
  }
}
auto Sema::actOnIfExpr(IfExpr* expr) -> std::unique_ptr<TypeBase>
{
  actOnExpr(expr->mCond.get());
  auto thenType = actOnBlockExpr(expr->mThen.get());
  if (expr->mElse) {
    assert(expr->mElse->mType == ExprWithBlock::Type::Block || expr->mElse->mType == ExprWithBlock::Type::If ||
           expr->mElse->mType == ExprWithBlock::Type::IfLet);
    auto elseType = actOnExprWithBlock(expr->mElse.get());
    if (!TypeEquals(thenType.get(), elseType.get())) {
      mDiags.report(llvm::SMLoc::getFromPointer(expr->mLoc), DiagId::ErrIncompatibleTypes, "if else expression",
                    TypeToString(thenType.get()), TypeToString(elseType.get()));
    }
  }
  return thenType;
}
auto Sema::actOnLoopExpr(LoopExpr* expr) -> std::unique_ptr<TypeBase>
{
  switch (expr->mType) {
  case LoopExpr::Type::InfiniteLoop:
    return actOnInfiniteLoopExpr(expr->as<InfiniteLoopExpr>());
  case LoopExpr::Type::PredicateLoop:
    return actOnPredicateLoopExpr(expr->as<PredicateLoopExpr>());
  }
  utils::Unimplemented(utils::SrcLoc::current());
}
auto Sema::actOnInfiniteLoopExpr(InfiniteLoopExpr* expr) -> std::unique_ptr<TypeBase>
{
  actOnBlockExpr(expr->mExpr.get());
  return std::make_unique<TupleType>(); // return unit
}
auto Sema::actOnPredicateLoopExpr(PredicateLoopExpr* expr) -> std::unique_ptr<TypeBase>
{
  actOnExpr(expr->mCond.get());
  actOnBlockExpr(expr->mExpr.get());
  return std::make_unique<TupleType>(); // return unit
}
auto Sema::actOnExprWithoutBlock(ExprWithoutBlock* expr) -> std::unique_ptr<TypeBase>
{
  switch (expr->mType) {
  case ExprWithoutBlock::Type::Literal:
    return actOnLiteralExpr(expr->as<LiteralExpr>());
  case ExprWithoutBlock::Type::Grouped:
    return actOnGroupedExpr(expr->as<GroupedExpr>());
  case ExprWithoutBlock::Type::Operator:
    return actOnOperatorExpr(expr->as<OperatorExpr>());
  case ExprWithoutBlock::Type::Call:
    return actOnCallExpr(expr->as<CallExpr>());
  case ExprWithoutBlock::Type::Return:
    return actOnReturnExpr(expr->as<ReturnExpr>());
  }
  utils::Unreachable(utils::SrcLoc::current());
}
auto Sema::actOnCallExpr(CallExpr* expr) -> std::unique_ptr<TypeBase>
{
  FunctionItem* fn = lookupItem(expr->mCallee)->as<FunctionItem>();
  if (fn == nullptr) {
    mDiags.report(llvm::SMLoc::getFromPointer(expr->getLoc()), DiagId::ErrInvalidFunctionCall, std::format("undeclared function '{}'", expr->mCallee));
    return std::make_unique<Unknown>();
  }

  if (fn->mParamNames.size() != expr->mArgs.size()) {
    mDiags.report(llvm::SMLoc::getFromPointer(expr->getLoc()), DiagId::ErrInvalidFunctionCall,
                  std::format("incompatible number of arguments, expected '{}' got '{}'", fn->mParamNames.size(),
                              expr->mArgs.size()));
    return TypeClone(fn->mFnType->mRet);
  }

  for (size_t i = 0; i < fn->mParamNames.size(); ++i) {
    auto argType = actOnExpr(expr->mArgs[i].get());
    if (!TypeEquals(fn->mFnType->mParams[i].get(), argType.get())) {
      mDiags.report(llvm::SMLoc::getFromPointer(expr->getLoc()), DiagId::ErrInvalidFunctionCall,
                    "incompatible parameter type at {}, expected '{}' got '{}'", i,
                    TypeToString(fn->mFnType->mParams[i].get()), TypeToString(argType.get()));
    }
  }
  return TypeClone(fn->mFnType->mRet);
}

auto Sema::actOnOperatorExpr(OperatorExpr* expr) -> std::unique_ptr<TypeBase>
{
  switch (expr->mType) {
  case OperatorExpr::Type::Binary:
    return actOnBinaryExpr(expr->as<BinaryExpr>());
  case OperatorExpr::Type::Unary:
    return actOnUnaryExpr(expr->as<UnaryExpr>());
  }
  utils::Unimplemented(utils::SrcLoc::current());
}

auto Sema::actOnBinaryExpr(BinaryExpr* expr) -> std::unique_ptr<TypeBase>
{
  auto lhsType = actOnExpr(expr->mLeft.get());
  auto rhsType = actOnExpr(expr->mRight.get());
  if (!TypeEquals(lhsType.get(), rhsType.get())) {
    mDiags.report(llvm::SMLoc::getFromPointer(expr->getLoc()), DiagId::ErrIncompatibleTypes, "", TypeToString(lhsType.get()), "and",
                  TypeToString(rhsType.get()));
  }
  // TODO: check if the operator is valid for the type
  return lhsType;
}
auto Sema::actOnUnaryExpr(UnaryExpr* expr) -> std::unique_ptr<TypeBase>
{
  auto type = actOnExpr(expr->mRight.get());
  // TODO: check if the operator is valid for the type
  return type;
}

auto Sema::actOnLiteralExpr(LiteralExpr* expr) -> std::unique_ptr<TypeBase>
{
  switch (expr->mKind) {
  case LiteralExpr::Kind::Bool:
    return std::make_unique<Boolean>();

#define CASE_NUMERIC_TYPE(Type)                                                                                        \
  case LiteralExpr::Kind::Type:                                                                                        \
    return std::make_unique<Type>();
    CASE_NUMERIC_TYPE(I8)
    CASE_NUMERIC_TYPE(I16)
    CASE_NUMERIC_TYPE(I32)
    CASE_NUMERIC_TYPE(I64)
    CASE_NUMERIC_TYPE(U8)
    CASE_NUMERIC_TYPE(U16)
    CASE_NUMERIC_TYPE(U32)
    CASE_NUMERIC_TYPE(U64)
    CASE_NUMERIC_TYPE(F32)
    CASE_NUMERIC_TYPE(F64)
#undef CASE_NUMERIC_TYPE

  case LiteralExpr::Kind::String:
    return std::make_unique<Str>();
  case LiteralExpr::Kind::Identifier: {
    auto name = std::get<std::string>(expr->mValue);
    auto identifierType = lookupIdentifier(name);
    if (identifierType == nullptr) {
      auto itemType = lookupItem(name);
      if (itemType == nullptr) {
        mDiags.report(llvm::SMLoc::getFromPointer(expr->getLoc()), DiagId::ErrUndefinedSym, name);
        return std::make_unique<Unknown>();
      } else {
        if (itemType->mKind == Item::Kind::Function) {
          return TypeClone(itemType->as<FunctionItem>()->mFnType.get());
        } else {
          utils::Unimplemented(utils::SrcLoc::current());
        }
      }
    } else {
      return TypeClone(identifierType);
    }
  }
  default:
    utils::Unimplemented(utils::SrcLoc::current(), "String type is not implemented");
    break;
  }
}

auto Sema::actOnGroupedExpr(GroupedExpr* expr) -> std::unique_ptr<TypeBase> { return actOnExpr(expr->mExpr.get()); }
auto Sema::actOnReturnExpr(ReturnExpr* expr) -> std::unique_ptr<TypeBase>
{
  auto exprType = actOnExpr(expr->mExpr.get());
  auto currFn = mFunctionStack.top();
  if (!TypeEquals(exprType.get(), currFn->mFnType->mRet.get())) {
    mDiags.report(llvm::SMLoc::getFromPointer(expr->getLoc()), DiagId::ErrIncompatibleTypes, std::format("function '{}' return expression", currFn->mName),
                  TypeToString(exprType.get()), TypeToString(currFn->mFnType->mRet.get()));
  }
  return std::make_unique<Never>();
}
auto Sema::actOnStmt(Stmt* stmt) -> void
{
  if (stmt == nullptr) {
    return;
  }
  switch (stmt->mType) {
  case Stmt::Type::Expression:
    return actOnExprStmt(stmt->as<ExprStmt>());
  case Stmt::Type::Let:
    return actOnLetStmt(stmt->as<LetStmt>());
  }
}

auto Sema::actOnExpr(Expr* expr) -> std::unique_ptr<TypeBase>
{
  switch (expr->mType) {
  case Expr::Type::WithBlock:
    return actOnExprWithBlock(expr->as<ExprWithBlock>());
  case Expr::Type::WithoutBlock:
    return actOnExprWithoutBlock(expr->as<ExprWithoutBlock>());
  }
  utils::Unreachable(utils::SrcLoc::current());
}

auto Sema::actOnItem(Item* expr) -> void
{
  switch (expr->mKind) {
  case Item::Kind::Function:
    return actOnFunctionItem(expr->as<FunctionItem>());
  case Item::Kind::Module:
  case Item::Kind::ExternCrate:
  case Item::Kind::UseDeclaration:
  case Item::Kind::TypeAlias:
  case Item::Kind::Struct:
  case Item::Kind::Enumeration:
  case Item::Kind::Union:
  case Item::Kind::ConstantItem:
  case Item::Kind::StaticItem:
  case Item::Kind::Trait:
  case Item::Kind::Implementation:
  case Item::Kind::ExternBlock:
    utils::Unimplemented(utils::SrcLoc::current(), "Item type is not implemented");
  case Item::Kind::SIZE:
    utils::Unreachable(utils::SrcLoc::current());
    break;
  }
}

auto Sema::actOnFunctionItem(FunctionItem* item) -> void
{
  insertItem(item->mName, item);
  mFunctionStack.push(item);
  auto guard = enterScope();
  {
    // insert parameters names
    for (i32 i = 0; i < item->mParamNames.size(); ++i) {
      insertIdentifier(item->mParamNames[i], TypeClone(item->mFnType->mParams[i].get()));
    }
    auto retType = actOnBlockExpr(item->mBody.get());
    if (!TypeEquals(retType.get(), item->mFnType->mRet.get())) {
      mDiags.report(llvm::SMLoc::getFromPointer(item->getLoc()), DiagId::ErrIncompatibleTypes, std::format("function '{}' return type", item->mName),
                    TypeToString(item->mFnType->mRet.get()), TypeToString(retType.get()));
    }
  }
  mFunctionStack.pop();
}

auto Sema::actOnLetStmt(LetStmt* stmt) -> void
{
  auto type = actOnExpr(stmt->mExpr.get());
  if (stmt->mExpectType) {
    auto expectedType = stmt->mExpectType.get();
    if (!TypeEquals(type.get(), expectedType)) {
      mDiags.report(llvm::SMLoc::getFromPointer(stmt->getLoc()), DiagId::ErrIncompatibleTypes, "let statement", TypeToString(expectedType),
                    TypeToString(type.get()));
    }
  }
  insertIdentifier(stmt->mName, std::move(type));
}

auto Sema::actOnExprStmt(ExprStmt* stmt) -> void
{
  auto ty = actOnExpr(stmt->mExpr.get());
  // if (!TypeEquals(ty.get(), &TypeUnit) && !TypeEquals(ty.get(), &TypeNever)) {
  //   mDiags.report(SMLoc(), DiagId::ErrIncompatibleTypes, "expression statement", TypeToString(&TypeUnit),
  //                 TypeToString(ty.get()));
  // }
}
