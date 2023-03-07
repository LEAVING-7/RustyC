#include "../Sema/Scope.hpp"
#include "../common.hpp"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <stack>

class IRGen {
  llvm::LLVMContext& mCtx;
  llvm::IRBuilder<> mBuilder;
  std::unique_ptr<llvm::Module> mModule;

  ScopeRecord mRecord;

  std::stack<llvm::Function*> mFunctionStack;

public:
  IRGen(llvm::LLVMContext& ctx, std::string_view modname, ScopeRecord&& record)
      : mCtx(ctx), mBuilder(ctx), mModule(std::make_unique<llvm::Module>(modname, ctx)), mRecord(std::move(record))
  {
  }
  ~IRGen() = default;

  auto genCrate(Crate* crate) -> void;

private:
  auto genStmt(Stmt* stmt) -> void;
  auto genExprStmt(ExprStmt* exprStmt) -> void;
  auto genLetStmt(LetStmt* letStmt) -> void;

  auto genItem(Item* item) -> void;
  auto genFunctionItem(FunctionItem* functionItem) -> void;
  auto genExternalBlockItem(ExternalBlockItem* externalBlockItem) -> void;

  auto genExpr(Expr* expr) -> llvm::Value*;
  auto genBlockExpr(BlockExpr* blockExpr) -> llvm::Value*;
  auto genLiteralExpr(LiteralExpr* literalExpr) -> llvm::Value*;
  auto genGroupedExpr(GroupedExpr* groupedExpr) -> llvm::Value*;
  auto genOperatorExpr(OperatorExpr* operatorExpr) -> llvm::Value*;
  auto genBinaryExpr(BinaryExpr* binaryExpr) -> llvm::Value*;
  auto genUnaryExpr(UnaryExpr* unaryExpr) -> llvm::Value*;
  auto genCallExpr(CallExpr* callExpr) -> llvm::Value*;
  auto genReturnExpr(ReturnExpr* returnExpr) -> llvm::Value*;
  auto genExprWithoutBlock(ExprWithoutBlock* exprWithoutBlock) -> llvm::Value*;
  auto genIfExpr(IfExpr* ifExpr) -> llvm::Value*;
  auto genLoopExpr(LoopExpr* loopExpr) -> llvm::Value*;
  auto genInfiniteLoopExpr(InfiniteLoopExpr* infiniteLoopExpr) -> llvm::Value*;
  auto genPredicateLoopExpr(PredicateLoopExpr* predicateExpr) -> llvm::Value*;

  auto lookupIdentifier(std::string const& name) -> TypeBase* { return mRecord.lookupIdentifier(name); }
  auto lookupItem(std::string const& name) -> Item* { return mRecord.lookupItem(name); };

  auto pushFunction(llvm::Function* func) -> void { mFunctionStack.push(func); }
  auto popFunction() -> void { mFunctionStack.pop(); }
  auto currentFunction() -> llvm::Function* { return mFunctionStack.top(); }
  auto enterScope() -> ScopeGuard<ScopeRecord> { return ScopeGuard(mRecord); }
};