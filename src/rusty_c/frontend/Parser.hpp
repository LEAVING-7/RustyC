#pragma once
#include "common.hpp"

#include "Lexer.hpp"
#include "Sema/Sema.hpp"
#include "Syntax.hpp"

class Parser {
  Cursor<Token*> mCursor;
  Sema mSema;

public:
  Parser(std::vector<Token>& token, DiagnosticsEngine& diags)
      : mCursor(token.data(), token.data() + token.size()), mSema(diags)
  {
  }

public:
  auto parseExpr(TokenKind end) -> std::unique_ptr<Expr>;

  auto parseExprWithBlock(TokenKind end) -> std::unique_ptr<ExprWithBlock>;
  auto parseBlockExpr() -> std::unique_ptr<BlockExpr>;
  auto parseIfExpr() -> std::unique_ptr<IfExpr>;
  auto parseLoopExpr() -> std::unique_ptr<LoopExpr>;
  auto parseInfiniteLoopExpr() -> std::unique_ptr<InfiniteLoopExpr>;
  auto parsePredicateLoopExpr() -> std::unique_ptr<PredicateLoopExpr>;

  auto parseExprWithoutBlock(TokenKind end) -> std::unique_ptr<Expr>;
  auto parseLiteralExpr() -> std::unique_ptr<LiteralExpr>;
  auto parseGroupedExpr(TokenKind end) -> std::unique_ptr<GroupedExpr>;
  auto parseBinaryExpr(TokenKind end, i32 bp) -> std::unique_ptr<Expr>;

  // auto parseBinaryOp(TokenKind end) -> std::unique_ptr<ExprWithoutBlock> { return parseBinaryOp(end, -1); }
  // auto parseBinaryOp(TokenKind end, i32 ptp) -> std::unique_ptr<ExprWithoutBlock>;

  auto parseStmts() -> std::vector<std::unique_ptr<Stmt>>;
  auto parseStmt() -> std::unique_ptr<Stmt>;
  auto parseLetStmt() -> std::unique_ptr<LetStmt>;
  auto parseExprStmt() -> std::unique_ptr<ExprStmt>;

  auto expect1(TokenKind type, std::source_location loc) -> bool;
  auto consume1(TokenKind type, std::source_location loc) -> bool;
  void error(std::source_location& loc);
};