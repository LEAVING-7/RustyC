#include "common.hpp"

#include "Lexer.hpp"
#include "Syntax.hpp"

class Parser {
  Cursor<Token*> mCursor;

public:
  Parser(std::vector<Token>& token) : mCursor(token.data(), token.data() + token.size()) {}

public:
  auto parseExpr(TokenKind end) -> std::unique_ptr<Expr>;
  auto parseExprWithBlock(TokenKind end) -> std::unique_ptr<ExprWithBlock>;

  auto parseExprWithoutBlock(TokenKind end) -> std::unique_ptr<ExprWithoutBlock>;
  auto parseLiteralExpr() -> std::unique_ptr<LiteralExpr>;
  auto parseNegationExpr(TokenKind end) -> std::unique_ptr<NegationExpr>;
  auto parseOperatorExpr(TokenKind end) -> std::unique_ptr<ExprWithoutBlock>;

  auto parseBinaryOp(TokenKind end) -> std::unique_ptr<ExprWithoutBlock> { return parseBinaryOp(end, -1); }
  auto parseBinaryOp(TokenKind end, i32 ptp) -> std::unique_ptr<ExprWithoutBlock>;

  auto parseGroupedExpr(TokenKind end) -> std::unique_ptr<GroupedExpr>;

  auto parseStmt() -> std::unique_ptr<Stmt>;
  auto parseLetStmt() -> std::unique_ptr<LetStmt>;

  auto expect(TokenKind type) -> bool;
  auto consume(TokenKind type) -> bool;
  void error();
};