#include "IRGen.hpp"

static auto GenLLVMType(TypeBase const* ty, llvm::LLVMContext& ctx) -> llvm::Type*;

auto IRGen::genCrate(Crate* crate) -> void
{
  auto guard = enterScope();
  for (auto& item : crate->mItems) {
    genItem(item.get());
  }
}
auto IRGen::genStmt(Stmt* stmt) -> void
{
  switch (stmt->mType) {
  case Stmt::Type::Let:
    return genLetStmt(stmt->as<LetStmt>());
  case Stmt::Type::Expression:
    return genExprStmt(stmt->as<ExprStmt>());
  default:
    utils::Unimplemented(utils::SrcLoc::current());
  }
}
auto IRGen::genBlockExpr(BlockExpr* blockExpr) -> llvm::Value*
{

  auto guard = enterScope();
  auto BB = llvm::BasicBlock::Create(mModule->getContext(), "entry", currentFunction());
  mBuilder.SetInsertPoint(BB);

  for (auto& item : blockExpr->mItems) {
    genItem(item.get());
  }
  for (auto& stmt : blockExpr->mStmts) {
    genStmt(stmt.get());
  }

  if (blockExpr->mReturn) {
    return genExpr(blockExpr->mReturn.get());
  } else {
    return nullptr;
  }
}
auto IRGen::genExprStmt(ExprStmt* exprStmt) -> void { genExpr(exprStmt->mExpr.get()); }
auto IRGen::genLetStmt(LetStmt* letStmt) -> void
{
  auto value = genExpr(letStmt->mExpr.get());
  // assert(mValueScopes.insertValue(letStmt->mName, value));
}
auto IRGen::genItem(Item* item) -> void
{
  switch (item->mKind) {
  case Item::Kind::Function:
    return genFunctionItem(item->as<FunctionItem>());
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
  case Item::Kind::SIZE:
    utils::Unimplemented(utils::SrcLoc::current());
    break;
  }
}
auto IRGen::genFunctionItem(FunctionItem* functionItem) -> void
{
  auto guard = enterScope();
  {
    auto fn = mModule->getFunction(functionItem->mName);
    if (!fn) { // if not exists, create a external linkage
      auto fnTy = llvm::dyn_cast<llvm::FunctionType>(GenLLVMType(functionItem->mFnType.get(), mCtx));
      fn = llvm::Function::Create(fnTy, llvm::Function::ExternalLinkage, functionItem->mName, mModule.get());
    }
    assert(fn != nullptr && !fn->empty());
    pushFunction(fn);
    auto body = genBlockExpr(functionItem->mBody.get());
    popFunction();
  }
}
auto IRGen::genExpr(Expr* expr) -> llvm::Value* {}
auto IRGen::genLiteralExpr(LiteralExpr* literalExpr) -> llvm::Value*
{
  switch (literalExpr->mKind) {
  case LiteralExpr::Kind::Bool:
    return llvm::ConstantInt::getBool(mCtx, std::get<bool>(literalExpr->mValue));
  case LiteralExpr::Kind::I8:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(8, std::get<int8_t>(literalExpr->mValue), true));
  case LiteralExpr::Kind::I16:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(16, std::get<int16_t>(literalExpr->mValue), true));
  case LiteralExpr::Kind::I32:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(32, std::get<int32_t>(literalExpr->mValue), true));
  case LiteralExpr::Kind::I64:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(64, std::get<int64_t>(literalExpr->mValue), true));
  case LiteralExpr::Kind::U8:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(8, std::get<uint8_t>(literalExpr->mValue), false));
  case LiteralExpr::Kind::U16:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(16, std::get<uint16_t>(literalExpr->mValue), false));
  case LiteralExpr::Kind::U32:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(32, std::get<uint32_t>(literalExpr->mValue), false));
  case LiteralExpr::Kind::U64:
    return llvm::ConstantInt::get(mCtx, llvm::APInt(64, std::get<uint64_t>(literalExpr->mValue), false));
  case LiteralExpr::Kind::F32:
    return llvm::ConstantFP::get(mCtx, llvm::APFloat(std::get<float>(literalExpr->mValue)));
  case LiteralExpr::Kind::F64:
    return llvm::ConstantFP::get(mCtx, llvm::APFloat(std::get<double>(literalExpr->mValue)));
  case LiteralExpr::Kind::String:
    utils::Unimplemented(utils::SrcLoc::current());
  case LiteralExpr::Kind::Identifier: {
    assert(std::holds_alternative<std::string>(literalExpr->mValue));
    auto ty = lookupIdentifier(std::get<std::string>(literalExpr->mValue));
    // return ;
  }
  }
}
auto IRGen::genGroupedExpr(GroupedExpr* groupedExpr) -> llvm::Value* { return genExpr(groupedExpr->mExpr.get()); }
auto IRGen::genOperatorExpr(OperatorExpr* operatorExpr) -> llvm::Value*
{
  switch (operatorExpr->mType) {
  case OperatorExpr::Type::Unary:
    return genUnaryExpr(operatorExpr->as<UnaryExpr>());
  case OperatorExpr::Type::Binary:
    return genBinaryExpr(operatorExpr->as<BinaryExpr>());
  }
}
auto IRGen::genBinaryExpr(BinaryExpr* binaryExpr) -> llvm::Value*
{
  auto lhs = genExpr(binaryExpr->mLeft.get());
  auto rhs = genExpr(binaryExpr->mRight.get());

  switch (binaryExpr->mKind) {
  case BinaryExpr::Kind::Add:

  case BinaryExpr::Kind::Sub:
  case BinaryExpr::Kind::Mul:
  case BinaryExpr::Kind::Div:
  case BinaryExpr::Kind::Rem:
  case BinaryExpr::Kind::BitAnd:
  case BinaryExpr::Kind::BitOr:
  case BinaryExpr::Kind::BitXor:
  case BinaryExpr::Kind::Shl:
  case BinaryExpr::Kind::Shr:
  case BinaryExpr::Kind::Eq:
  case BinaryExpr::Kind::Ne:
  case BinaryExpr::Kind::Gt:
  case BinaryExpr::Kind::Lt:
  case BinaryExpr::Kind::Ge:
  case BinaryExpr::Kind::Le:
  case BinaryExpr::Kind::Assignment:
  case BinaryExpr::Kind::SIZE:
    break;
  }
}
auto IRGen::genUnaryExpr(UnaryExpr* unaryExpr) -> llvm::Value* {}
auto IRGen::genCallExpr(CallExpr* callExpr) -> llvm::Value*
{
  auto callee = mModule->getFunction(callExpr->mCallee);
  assert(callee);

  assert(callee->arg_size() == callExpr->mArgs.size());
  std::vector<llvm::Value*> args{};
  for (i32 i = 0; i < callExpr->mArgs.size(); ++i) {
    args.push_back(genExpr(callExpr->mArgs[i].get()));
    if (args.back() == nullptr) {
      return nullptr;
    }
  }
  return mBuilder.CreateCall(callee, args, "calltmp");
}
auto IRGen::genReturnExpr(ReturnExpr* returnExpr) -> llvm::Value* {}
auto IRGen::genExprWithoutBlock(ExprWithoutBlock* exprWithoutBlock) -> llvm::Value* {}
auto IRGen::genIfExpr(IfExpr* ifExpr) -> llvm::Value* {}
auto IRGen::genLoopExpr(LoopExpr* loopExpr) -> llvm::Value* {}
auto IRGen::genInfiniteLoopExpr(InfiniteLoopExpr* infiniteLoopExpr) -> llvm::Value* {}
auto IRGen::genPredicateLoopExpr(PredicateLoopExpr* predicateExpr) -> llvm::Value* {}

auto IRGen::genExternalBlockItem(ExternalBlockItem* externalBlockItem) -> void
{
  for (auto& item : externalBlockItem->mItems) {
    assert(item->isDeclaration());
    auto llvmFnTy = llvm::dyn_cast<llvm::FunctionType>(GenLLVMType(item->mFnType.get(), mCtx));
    auto llvmFn = llvm::Function::Create(llvmFnTy, llvm::Function::ExternalLinkage, item->mName, mModule.get());
    for (auto& arg : llvmFn->args()) {
      arg.setName(item->mParamNames[arg.getArgNo()]);
    }
  }
}

static auto GenLLVMType(TypeBase const* ty, llvm::LLVMContext& ctx) -> llvm::Type*
{
  switch (ty->mKind) {
  case TypeBase::Kind::Boolean:
    return llvm::Type::getInt1Ty(ctx);
  case TypeBase::Kind::I8:
    return llvm::Type::getInt8Ty(ctx);
  case TypeBase::Kind::I16:
    return llvm::Type::getInt16Ty(ctx);
  case TypeBase::Kind::I32:
    return llvm::Type::getInt32Ty(ctx);
  case TypeBase::Kind::I64:
    return llvm::Type::getInt64Ty(ctx);
  case TypeBase::Kind::U8:
    return llvm::Type::getInt8Ty(ctx);
  case TypeBase::Kind::U16:
    return llvm::Type::getInt16Ty(ctx);
  case TypeBase::Kind::U32:
    return llvm::Type::getInt32Ty(ctx);
  case TypeBase::Kind::U64:
    return llvm::Type::getInt64Ty(ctx);
  case TypeBase::Kind::F32:
    return llvm::Type::getFloatTy(ctx);
  case TypeBase::Kind::F64:
    return llvm::Type::getDoubleTy(ctx);
  case TypeBase::Kind::Functions: {
    auto fnTy = ty->as<FunctionType>();
    auto retTy = GenLLVMType(fnTy->mRet.get(), ctx);
    auto paramTys = std::vector<llvm::Type*>{};
    for (auto& paramTy : fnTy->mParams) {
      paramTys.push_back(GenLLVMType(paramTy.get(), ctx));
    }
    return llvm::FunctionType::get(retTy, paramTys, false);
  }
  case TypeBase::Kind::Tuple: {
    auto tupleTy = ty->as<TupleType>();
    if (tupleTy->isUnit()) {
      return llvm::Type::getVoidTy(ctx);
    } else {
      utils::Unimplemented(utils::SrcLoc::current());
    }
  }
  case TypeBase::Kind::Char:
  case TypeBase::Kind::Str:
  case TypeBase::Kind::Never:
  case TypeBase::Kind::Array:
  case TypeBase::Kind::Slice:
  case TypeBase::Kind::Struct:
  case TypeBase::Kind::Enum:
  case TypeBase::Kind::Union:

  case TypeBase::Kind::Closures:
  case TypeBase::Kind::Reference:
  case TypeBase::Kind::RawPointer:
  case TypeBase::Kind::FunctionPointer:
  case TypeBase::Kind::TraitObjects:
  case TypeBase::Kind::ImplTrait:
  case TypeBase::Kind::Unknown:
  case TypeBase::Kind::SIZE:
    utils::Unimplemented(utils::SrcLoc::current());
    break;
  }
}