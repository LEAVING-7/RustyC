#pragma once
#include "Syntax.hpp"

// leaf node
// BinaryExpr, BlockExpr, CallExpr, GroupedExpr, IfExpr, InfiniteLoopExpr, LiteralExpr, PredicateLoopExpr, ReturnExpr,
// UnaryExpr, LetStmt, FunctionItem
template <typename T, typename... Args>
struct Visitor {
  virtual auto walk(BinaryExpr* expr, Args... args) -> T = 0;
  virtual auto walk(BlockExpr* expr, Args... args) -> T = 0;
  virtual auto walk(CallExpr* expr, Args... args) -> T = 0;
  auto walkExpr(Expr* expr, Args... args) -> T
  {
    switch (expr->mType) {
    case Expr::Type::WithBlock:
      return walk(expr->as<ExprWithBlock>(), std::forward<Args>(args)...);
    case Expr::Type::WithoutBlock:
      return walk(expr->as<ExprWithoutBlock>(), std::forward<Args>(args)...);
    default:
      utils::Unreachable(utils::SrcLoc::current());
    }
  }
  virtual auto walk(ExprStmt* stmt, Args... args) -> T = 0;
  auto walk(ExprWithBlock* expr, Args... args) -> T
  {
    switch (expr->mType) {
    case ExprWithBlock::Type::Block:
      return walk(expr->as<BlockExpr>(), std::forward<Args>(args)...);
    case ExprWithBlock::Type::Loop:
      return walk(expr->as<LoopExpr>(), std::forward<Args>(args)...);
    case ExprWithBlock::Type::If:
      return walk(expr->as<IfExpr>(), std::forward<Args>(args)...);
    default:
      utils::Unimplemented(utils::SrcLoc::current());
    }
  }
  auto walk(ExprWithoutBlock* expr, Args... args) -> T
  {
    switch (expr->mType) {
    case ExprWithoutBlock::Type::Literal:
      return walk(expr->as<LiteralExpr>(), std::forward<Args>(args)...);
    case ExprWithoutBlock::Type::Grouped:
      return walk(expr->as<GroupedExpr>(), std::forward<Args>(args)...);
    case ExprWithoutBlock::Type::Operator:
      return walk(expr->as<OperatorExpr>(), std::forward<Args>(args)...);
    case ExprWithoutBlock::Type::Call:
      return walk(expr->as<CallExpr>(), std::forward<Args>(args)...);
    case ExprWithoutBlock::Type::Return:
      return walk(expr->as<ReturnExpr>(), std::forward<Args>(args)...);
    default:
      utils::Unimplemented(utils::SrcLoc::current());
    }
  }
  virtual auto walk(GroupedExpr* expr, Args... args) -> T = 0;
  virtual auto walk(IfExpr* expr, Args... args) -> T = 0;
  virtual auto walk(InfiniteLoopExpr* expr, Args... args) -> T = 0;
  virtual auto walk(LiteralExpr* expr, Args... args) -> T = 0;
  auto walk(LoopExpr* expr, Args... args) -> T
  {
    switch (expr->mType) {
    case LoopExpr::Type::InfiniteLoop:
      return walk(expr->as<InfiniteLoopExpr>(), std::forward<Args>(args)...);
    case LoopExpr::Type::PredicateLoop:
      return walk(expr->as<PredicateLoopExpr>(), std::forward<Args>(args)...);
    default:
      utils::Unreachable(utils::SrcLoc::current());
    }
  }
  auto walk(OperatorExpr* expr, Args... args) -> T
  {
    switch (expr->mType) {
    case OperatorExpr::Type::Binary:
      return walk(expr->as<BinaryExpr>(), std::forward<Args>(args)...);
    case OperatorExpr::Type::Unary:
      return walk(expr->as<UnaryExpr>(), std::forward<Args>(args)...);
    default:
      utils::Unreachable(utils::SrcLoc::current());
    }
  }
  virtual auto walk(ReturnExpr* expr, Args... args) -> T = 0;
  virtual auto walk(PredicateLoopExpr* expr, Args... args) -> T = 0;
  virtual auto walk(UnaryExpr* expr, Args... args) -> T = 0;

  auto walkStmt(Stmt* stmt, Args... args) -> T
  {
    switch (stmt->mType) {
    case Stmt::Type::Let:
      return walk(stmt->as<LetStmt>(), std::forward<Args>(args)...);
    case Stmt::Type::Expression:
      return walk(stmt->as<ExprStmt>(), std::forward<Args>(args)...);
    default:
      utils::Unimplemented(utils::SrcLoc::current());
    }
  }
  virtual auto walk(LetStmt* stmt, Args... args) -> T = 0;

  auto walkItem(Item* item, Args... args) -> T
  {
    switch (item->mKind) {
    case Item::Kind::Function:
      return walk(item->as<FunctionItem>(), std::forward<Args>(args)...);
    default:
      utils::Unimplemented(utils::SrcLoc::current());
    }
  }
  virtual auto walk(FunctionItem* item, Args... args) -> T = 0;
};

auto CrateToString(Crate* crate) -> std::string;