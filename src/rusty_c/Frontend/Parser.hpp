#pragma once
#include "common.hpp"

#include "Lexer.hpp"
#include "Sema/Sema.hpp"
#include "Syntax.hpp"

class Parser {
  Cursor<Token*> mCursor;
  DiagnosticsEngine& mDiags;

public:
  Parser(std::vector<Token>& token, DiagnosticsEngine& diags)
      : mCursor(token.data(), token.data() + token.size()), mDiags(diags)
  {
  }

public:
  using PredT = bool(Token const&);
  auto parseCrate() -> Crate;

  auto parseExpr(PredT pred) -> std::unique_ptr<Expr>;

  auto parseExprWithBlock(PredT pred) -> std::unique_ptr<ExprWithBlock>;
  auto parseBlockExpr() -> std::unique_ptr<BlockExpr>;
  auto parseIfExpr() -> std::unique_ptr<IfExpr>;
  auto parseLoopExpr() -> std::unique_ptr<LoopExpr>;
  auto parseInfiniteLoopExpr() -> std::unique_ptr<InfiniteLoopExpr>;
  auto parsePredicateLoopExpr() -> std::unique_ptr<PredicateLoopExpr>;

  auto parseExprWithoutBlock(PredT pred) -> std::unique_ptr<Expr>;
  auto parseLiteralExpr() -> std::unique_ptr<LiteralExpr>;
  auto parseGroupedExpr(PredT pred) -> std::unique_ptr<GroupedExpr>;
  auto parseBinaryExpr(PredT pred, i32 bp) -> std::unique_ptr<Expr>;
  auto parseReturnExpr() -> std::unique_ptr<ReturnExpr>;

  auto parseStmts() -> std::vector<std::unique_ptr<Stmt>>;
  auto parseStmt(PredT pred = [](auto) { return true; }) -> std::unique_ptr<Stmt>;
  auto parseLetStmt() -> std::unique_ptr<LetStmt>;
  auto parseExprStmt(PredT pred = [](Token const& tok) { return tok.is(PunSemi); }) -> std::unique_ptr<ExprStmt>;

  // parse item
  auto isItemStart(Token const& tok) -> bool;
  auto parseItem() -> std::unique_ptr<Item>;

  auto parseFunctionItem() -> std::unique_ptr<FunctionItem>;
  auto parseExternalBlockItem() -> std::unique_ptr<ExternalBlockItem>;

  // parse type
  auto parseType() -> std::unique_ptr<TypeBase>;
  auto parseFunctionType() -> std::unique_ptr<FunctionType>;
  auto parseTupleType() -> std::unique_ptr<TupleType>;

  auto currSMLoc() -> llvm::SMLoc { return llvm::SMLoc::getFromPointer(currBufLoc()); }
  auto currBufLoc() -> char const* { return mCursor.peek().getLoc(); }
  auto skip() -> void { mCursor.skip(); };
  auto skipIf(TokenKind type) -> void;
  auto peek(i32 n = 0) -> Token const& { return mCursor.peek(n); };
  auto skipAfter(std::function<bool(Token const&)>&& pred) -> void;
  auto skipUntil(std::function<bool(Token const&)>&& pred) -> void;
  auto expect(TokenKind type) -> bool;
  auto consume(TokenKind type) -> bool;
  void error();
};