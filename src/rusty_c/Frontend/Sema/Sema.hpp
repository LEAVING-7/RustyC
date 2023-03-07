#pragma once

#include "Frontend/Lexer.hpp"
#include "Scope.hpp"
#include <stack>

class Sema {
  DiagnosticsEngine& mDiags;
  Scopes mScopes;

  struct FunctionProp {
    FunctionItem* fn;
    size_t loc; // location in scopes
  };
  std::stack<FunctionProp> mFunctionStack;

public:
  Sema(DiagnosticsEngine& diags) : mDiags(diags) {}

  auto actOnCrate(Crate const* crate) -> void;

  auto actOnExpr(Expr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnExprWithBlock(ExprWithBlock* expr) -> std::unique_ptr<TypeBase>;
  auto actOnBlockExpr(BlockExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnIfExpr(IfExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnLoopExpr(LoopExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnInfiniteLoopExpr(InfiniteLoopExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnPredicateLoopExpr(PredicateLoopExpr* expr) -> std::unique_ptr<TypeBase>;

  auto actOnExprWithoutBlock(ExprWithoutBlock* expr) -> std::unique_ptr<TypeBase>;
  auto actOnOperatorExpr(OperatorExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnBinaryExpr(BinaryExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnUnaryExpr(UnaryExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnLiteralExpr(LiteralExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnGroupedExpr(GroupedExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnReturnExpr(ReturnExpr* expr) -> std::unique_ptr<TypeBase>;
  auto actOnCallExpr(CallExpr* expr) -> std::unique_ptr<TypeBase>;

  auto actOnItem(Item* expr) -> void;
  auto actOnFunctionItem(FunctionItem* expr) -> void;
  auto actOnExternalBlockItem(ExternalBlockItem* expr) -> void;

  auto actOnStmt(Stmt* expr) -> void;
  auto actOnLetStmt(LetStmt* expr) -> void;
  auto actOnExprStmt(ExprStmt* expr) -> void;

  auto enterScope() -> ScopeGuard<Scopes> { return ScopeGuard(mScopes); }

  auto insertIdentifier(std::string const& name, std::unique_ptr<TypeBase> type) -> bool
  {
    return mScopes.insertIdentifier(name, std::move(type));
  }
  auto insertItem(std::string const& name, Item* item) -> bool { return mScopes.insertItem(name, (item)); }
  auto lookupIdentifier(std::string const& name) -> TypeBase*
  {
    return mScopes.lookupIdUntil(name, mFunctionStack.top().loc);
  }
  auto lookupItem(std::string const& name) -> Item* { return mScopes.lookupItemUntil(name, mFunctionStack.top().loc); }

  auto lookupIdUntil(std::string const& name, i32 until) -> TypeBase* { return mScopes.lookupIdUntil(name, until); }
  auto lookupItemUntil(std::string const& name, i32 until) -> Item* { return mScopes.lookupItemUntil(name, until); }
};
