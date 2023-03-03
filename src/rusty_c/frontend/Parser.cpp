#include "Parser.hpp"
#include <iostream>

template <TokenMap T>
bool IsToken(TokenKind tok)
{
  return T::MapOp(tok) != T::Kind::SIZE;
}
bool IsBinaryExpr(TokenKind tok)
{
  return IsToken<ArithmeticOrLogicalExpr>(tok) || IsToken<ComparisonExpr>(tok) || tok == PunEq;
}
bool IsUnaryExpr(TokenKind tok) { return IsToken<NegationExpr>(tok); }

auto Parser::parseExprStmt(TokenKind end) -> std::unique_ptr<ExprStmt>
{
  if (auto& tok = mCursor.peek(); tok.isOneOf(PunLBrace, Kwif, Kwwhile, Kwloop)) {
    return std::make_unique<ExprStmt>(parseExprWithBlock(end));
  } else {
    return std::make_unique<ExprStmt>(parseExprWithoutBlock(end));
  }
}

auto Parser::parseExpr(TokenKind end) -> std::unique_ptr<Expr>
{
  if (mCursor.peek().isOneOf(Kwloop, Kwwhile, PunLBrace)) {
    return parseExprWithBlock(end);
  } else {
    return parseExprWithoutBlock(end);
  }
}

auto Parser::parseExprWithoutBlock(TokenKind end) -> std::unique_ptr<ExprWithoutBlock>
{
  if (mCursor.peek().is(end)) {
    return nullptr;
  }
  auto& tok = mCursor.peek();
  auto nextTokType = mCursor.peek(1).getType();
  if (tok.isOneOf(NumberConstant, Identifier)) {
    if (!mIsParsingOperatorExpr && IsBinaryExpr(nextTokType) || IsUnaryExpr(nextTokType)) {
      mIsParsingOperatorExpr = true;
      auto expr = parseOperatorExpr(end);
      mIsParsingOperatorExpr = false;
      return expr;
    } else {
      return parseLiteralExpr();
    }
  } else if (tok.is(PunLParen)) {
    return parseGroupedExpr(end);
  } else if (IsBinaryExpr(tok.getType()) || IsUnaryExpr(tok.getType())) {
    return parseOperatorExpr(end);
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
    } else if (auto op = PunEq) {
      auto prec = AssignmentExpr::GetPrec();
      while (prec > ptp) {
        mCursor.skip();
        auto right = parseExpr(end);
        left = std::make_unique<AssignmentExpr>(std::move(left), std::move(right));
        if (mCursor.peek().is(END) || mCursor.peek().is(end)) {
          return left;
        }
        tokTy = mCursor.peek().getType();
        prec = AssignmentExpr::GetPrec();
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

auto Parser::parseStmt(TokenKind end) -> std::unique_ptr<Stmt>
{
  if (mCursor.peek().is(Kwlet)) {
    return parseLetStmt();
  } else {
    return parseExprStmt(end);
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

auto Parser::parseExprWithBlock(TokenKind end) -> std::unique_ptr<ExprWithBlock>
{
  if (mCursor.peek().is(PunLBrace)) {
    return parseBlockExpr();
  } else if (mCursor.peek().isOneOf(Kwloop, Kwwhile)) {
    return parseLoopExpr();
  }
  assert(0);
  return {};
}

auto Parser::parseBlockExpr() -> std::unique_ptr<BlockExpr>
{
  consume(PunLBrace);
  std::vector<std::unique_ptr<Stmt>> stmts{};
  while (!mCursor.peek().is(PunRBrace)) { // TODO expr without block
    stmts.push_back(parseStmt(PunRBrace));
  }
  consume(PunRBrace);
  return std::make_unique<BlockExpr>(std::move(stmts), nullptr);
}
auto Parser::parseIfExpr() -> std::unique_ptr<IfExpr>
{
  consume(Kwif);
  auto cond = parseExpr(PunLBrace);
  consume(PunLBrace);
  auto block = parseBlockExpr();
  consume(PunRBrace);
  auto else_ = std::unique_ptr<ExprWithBlock>{nullptr};
  if (mCursor.peek().is(Kwelse)) {
    mCursor.skip(); // skip 'else'
    if (mCursor.peek().is(Kwif)) {
      else_ = parseIfExpr();
    } else if (mCursor.peek().is(PunLBrace)) {
      else_ = parseBlockExpr();
    } else {
      assert(0);
    }
  }
  return std::make_unique<IfExpr>(std::move(cond), std::move(block), std::move(else_));
}
auto Parser::parseLoopExpr() -> std::unique_ptr<LoopExpr>
{
  if (mCursor.peek().is(Kwloop)) {
    return parseInfiniteLoopExpr();
  } else if (mCursor.peek().is(Kwwhile)) {
    return parsePredicateLoopExpr();
  }
  assert(0);
  return {};
}
auto Parser::parseInfiniteLoopExpr() -> std::unique_ptr<InfiniteLoopExpr>
{
  consume(Kwloop);
  auto body = parseBlockExpr();
  return std::make_unique<InfiniteLoopExpr>(std::move(body));
}
auto Parser::parsePredicateLoopExpr() -> std::unique_ptr<PredicateLoopExpr>
{
  consume(Kwwhile);
  auto cond = parseExpr(PunLBrace);
  auto body = parseBlockExpr();
  return std::make_unique<PredicateLoopExpr>(std::move(cond), std::move(body));
}

auto Parser::parseStmts() -> std::vector<std::unique_ptr<Stmt>>
{
  std::vector<std::unique_ptr<Stmt>> vec{};
  while (!mCursor.isEnd()) {
    vec.push_back(parseStmt(PunSemi));
  }
  return vec;
}

auto Parser::expect(TokenKind type) -> bool
{
  if (mCursor.peek().getType() != type) {
    error();
    puts("ji");
    return false;
  }
  return true;
}
auto Parser::consume(TokenKind type) -> bool
{
  if (!expect(type)) {
    puts("ji");
    return false;
  }
  mCursor.skip();
  return true;
}

void Parser::error() { std::cerr << std::format("Unexpected: {}\n", mCursor.peek().toString()); }