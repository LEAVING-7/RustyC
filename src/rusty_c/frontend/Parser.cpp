#include "Parser.hpp"
#include <iostream>

template <TokenMap T>
bool IsToken(TokenKind tok)
{
  return T::MapOp(tok) != T::Kind::SIZE;
}
bool IsBinaryExpr(TokenKind tok) { return IsToken<ArithmeticOrLogicalExpr>(tok) || IsToken<ComparisonExpr>(tok); }
bool IsUnaryExpr(TokenKind tok) { return IsToken<NegationExpr>(tok); }

auto Parser::parseExpr(TokenKind end) -> std::unique_ptr<Expr>
{
  if (auto& tok = mCursor.peek(); tok.is(PunLBrace)) {
    return parseExprWithBlock(end);
  } else {
    return parseExprWithoutBlock(end);
  }
}
auto Parser::parseExprWithBlock(TokenKind end) -> std::unique_ptr<ExprWithBlock>
{
  assert(0);
  return {};
}

auto Parser::parseExprWithoutBlock(TokenKind end) -> std::unique_ptr<ExprWithoutBlock>
{
  if (mCursor.peek().is(end)) {
    return nullptr;
  }
  if (auto& tok = mCursor.peek(); tok.isOneOf(NumberConstant, Identifier)) {
    return parseLiteralExpr();
  } else if (IsBinaryExpr(tok.getType()) || IsUnaryExpr(tok.getType())) {
    return parseOperatorExpr(end);
  } else if (mCursor.peek().is(PunLParen)) {
    return parseGroupedExpr(end);
  } else {
    assert(0);
    return {};
  }
}

auto Parser::parseNegationExpr(TokenKind end) -> std::unique_ptr<NegationExpr>
{
  auto const& tok = mCursor.peek();
  mCursor.skip();
  auto ty = NegationExpr::MapOp(tok.getType());
  auto expr = parseExprWithoutBlock(end);
  return std::make_unique<NegationExpr>(ty, std::move(expr));
}

auto Parser::parseLiteralExpr() -> std::unique_ptr<LiteralExpr>
{
  auto const& tok = mCursor.peek();
  mCursor.skip();
  auto ty = static_cast<LiteralExpr::Kind>(tok.getValueIndex());
  return std::make_unique<LiteralExpr>(ty, tok.getValue());
}

auto Parser::parseOperatorExpr(TokenKind end) -> std::unique_ptr<ExprWithoutBlock>
{
  if (NegationExpr::MapOp(mCursor.peek().getType()) != NegationExpr::Kind::SIZE) {
    return parseNegationExpr(end);
  } else {
    return parseBinaryOp(end);
  }
}

auto Parser::parseBinaryOp(TokenKind end, i32 ptp) -> std::unique_ptr<ExprWithoutBlock>
{
  auto left = parseExprWithoutBlock(end);
  if (mCursor.peek().is(END) || mCursor.peek().is(end)) {
    return left;
  } else {
    auto tokTy = mCursor.peek().getType();
    if (auto op = ArithmeticOrLogicalExpr::MapOp(tokTy); op != ArithmeticOrLogicalExpr::Kind::SIZE) {
      auto prec = ArithmeticOrLogicalExpr::GetPrec(op);
      while (prec > ptp) {
        mCursor.skip();
        auto right = parseBinaryOp(end, prec);
        left = std::make_unique<ArithmeticOrLogicalExpr>(op, std::move(left), std::move(right));
        if (mCursor.peek().is(END) || mCursor.peek().is(end)) {
          return left;
        }
        tokTy = mCursor.peek().getType();
        op = ArithmeticOrLogicalExpr::MapOp(tokTy);
        prec = ArithmeticOrLogicalExpr::GetPrec(op);
      }
    } else if (auto op = ComparisonExpr::MapOp(tokTy); op != ComparisonExpr::Kind::SIZE) {
      auto prec = ComparisonExpr::GetPrec(op);
      while (prec > ptp) {
        mCursor.skip();
        auto right = parseBinaryOp(end, prec);
        left = std::make_unique<ComparisonExpr>(op, std::move(left), std::move(right));
        if (mCursor.peek().is(END) || mCursor.peek().is(end)) {
          return left;
        }
        tokTy = mCursor.peek().getType();
        op = ComparisonExpr::MapOp(tokTy);
        prec = ComparisonExpr::GetPrec(op);
      }
    }
    return left;
  }
}

auto Parser::parseGroupedExpr(TokenKind end) -> std::unique_ptr<GroupedExpr>
{
  consume(PunLParen);
  auto expr = parseOperatorExpr(PunRParen);
  consume(PunRParen);
  return std::make_unique<GroupedExpr>(std::move(expr));
}

auto Parser::parseStmt() -> std::unique_ptr<Stmt>
{
  if (mCursor.peek().is(Kwlet)) {
    return parseLetStmt();
  } else {
    assert(0);
    return {};
  }
}

auto Parser::parseLetStmt() -> std::unique_ptr<LetStmt>
{
  mCursor.skip();
  assert(expect(Identifier));
  auto name = mCursor.peek().get<std::string>();
  mCursor.skip();
  assert(consume(PunEq));
  auto expr = parseOperatorExpr(PunSemi);
  mCursor.skip(); // skip semicolon
  return std::make_unique<LetStmt>(name, std::move(expr));
}

auto Parser::expect(TokenKind type) -> bool
{
  if (mCursor.peek().getType() != type) {
    error();
    return false;
  }
  return true;
}
auto Parser::consume(TokenKind type) -> bool
{
  if (!expect(type)) {
    return false;
  }
  mCursor.skip();
  return true;
}

void Parser::error() { std::cerr << std::format("Unexpected: {}\n", mCursor.peek().toString()); }