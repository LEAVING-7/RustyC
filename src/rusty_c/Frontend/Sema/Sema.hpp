#pragma once

#include "Frontend/Lexer.hpp"
#include "Scope.hpp"
#include <stack>

class ScopeGuard {
  Scopes& mScopes;

public:
  ScopeGuard(Scopes& scopes) : mScopes(scopes) { mScopes.enterScope(); }
  ~ScopeGuard() { mScopes.leaveScope(); }
};

class Sema {
  DiagnosticsEngine& mDiags;
  Scopes mScopes;

  std::stack<FunctionItem*> mFunctionStack;

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

  auto actOnStmt(Stmt* expr) -> void;
  auto actOnItem(Item* expr) -> void;
  auto actOnFunctionItem(FunctionItem* expr) -> void;
  auto actOnLetStmt(LetStmt* expr) -> void;
  auto actOnExprStmt(ExprStmt* expr) -> void;

  // auto deleteScopes() -> void { mScopesStack.pop_back(); }
  // auto newScopes() -> void { mScopesStack.emplace_back(); }
  auto enterScope() -> ScopeGuard { return ScopeGuard(mScopes); }
  // auto enterScope() -> void { return mScopesStack.enterScope(); }
  // auto leaveScope() -> void { return mScopesStack.leaveScope(); }
  auto insertIdentifier(std::string const& name, std::unique_ptr<TypeBase> type) -> bool
  {
    return mScopes.insertIdentifier(name, std::move(type));
  }
  auto insertItem(std::string const& name, Item* item) -> bool { return mScopes.insertItem(name, (item)); }

  auto lookupIdentifier(std::string const& name) -> TypeBase* { return mScopes.lookupIdentifier(name); }
  auto lookupItem(std::string const& name) -> Item* { return mScopes.lookupItem(name); }
};
