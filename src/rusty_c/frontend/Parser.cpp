#include "Parser.hpp"
#include <iostream>

#define consume(tok) this->consume1(tok, std::source_location::current())
#define expect(tok) this->expect1(tok, std::source_location::current())

bool IsUnaryTok(TokenKind tok) { return UnaryExpr::MapKind(tok) != UnaryExpr::Kind::SIZE; }

auto Parser::parseExprStmt() -> std::unique_ptr<ExprStmt>
{
  if (auto& tok = mCursor.peek(); tok.isOneOf(PunLBrace, Kwif, Kwwhile, Kwloop)) {
    auto ret = std::make_unique<ExprStmt>(parseExprWithBlock(PunSemi));
    consume(PunSemi);
    return ret;
  } else {
    auto ret = std::make_unique<ExprStmt>(parseExprWithoutBlock(PunSemi));
    consume(PunSemi);
    return ret;
  }
}

auto Parser::parseExpr(TokenKind end) -> std::unique_ptr<Expr>
{
  if (mCursor.peek().isOneOf(Kwloop, Kwwhile, PunLBrace, Kwif)) {
    return parseExprWithBlock(end);
  } else {
    return parseExprWithoutBlock(end);
  }
}

auto Parser::parseExprWithoutBlock(TokenKind end) -> std::unique_ptr<Expr> { return parseBinaryExpr(end, -1); }

auto Parser::parseLiteralExpr() -> std::unique_ptr<LiteralExpr>
{
  auto const& tok = mCursor.peek();
  mCursor.skip();
  auto ty = static_cast<LiteralExpr::Kind>(tok.getValueIndex());
  return std::make_unique<LiteralExpr>(ty, tok.getValue());
}

auto Parser::parseBinaryExpr(TokenKind end, i32 bp) -> std::unique_ptr<Expr>
{
  if (mCursor.peek().is(END) && mCursor.peek().getKind() == end) {
    return nullptr;
  }
  std::unique_ptr<Expr> left;

  if (auto tok = mCursor.peek().getKind(); tok == PunLParen) {
    left = parseGroupedExpr(end);
  } else if (tok == NumberConstant || tok == Identifier) {
    left = parseLiteralExpr();
  } else if (auto kind = UnaryExpr::MapKind(tok); kind != UnaryExpr::Kind::SIZE) {
    auto [bp] = UnaryExpr::BindingPower(kind); // prefix
    auto right = parseBinaryExpr(end, bp);
    left = std::make_unique<UnaryExpr>(kind, std::move(right));
  } else {
    assert(0);
  }

  while (!mCursor.peek().is(END) && mCursor.peek().getKind() != end) {
    auto tokKind = mCursor.peek().getKind();
    auto op = BinaryExpr::MapKind(tokKind);
    if (op == BinaryExpr::Kind::SIZE) {
      assert(0);
      break;
    }
    auto [bpl, bpr] = BinaryExpr::BindingPower(op);
    if (bpl < bp) {
      break;
    }
    mCursor.skip();
    auto right = parseBinaryExpr(end, bpr);
    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }
  return left;
}

auto Parser::parseGroupedExpr(TokenKind end) -> std::unique_ptr<GroupedExpr>
{
  consume(PunLParen);
  auto expr = parseExpr(PunRParen);
  consume(PunRParen);
  return std::make_unique<GroupedExpr>(std::move(expr));
}

auto Parser::parseStmt() -> std::unique_ptr<Stmt>
{
  if (mCursor.peek().is(Kwlet)) {
    return parseLetStmt();
  } else {
    return parseExprStmt();
  }
}

auto Parser::parseLetStmt() -> std::unique_ptr<LetStmt>
{
  mCursor.skip();
  assert(expect(Identifier));
  auto name = mCursor.peek().get<std::string>();
  mCursor.skip();
  assert(consume(PunEq));
  auto expr = parseExpr(PunSemi);
  mCursor.skip(); // skip semicolon
  return std::make_unique<LetStmt>(name, std::move(expr));
}

auto Parser::parseExprWithBlock(TokenKind end) -> std::unique_ptr<ExprWithBlock>
{
  if (mCursor.peek().is(PunLBrace)) {
    return parseBlockExpr();
  } else if (mCursor.peek().isOneOf(Kwloop, Kwwhile)) {
    return parseLoopExpr();
  } else if (mCursor.peek().is(Kwif)) {
    return parseIfExpr();
  }
  assert(0);
  return {};
}

auto Parser::parseBlockExpr() -> std::unique_ptr<BlockExpr>
{
  consume(PunLBrace);
  std::vector<std::unique_ptr<Stmt>> stmts{};
  while (!mCursor.peek().is(PunRBrace)) { // TODO expr without block
    stmts.push_back(parseStmt());
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
  while (!mCursor.peek().is(END)) {
    vec.push_back(parseStmt());
  }
  return vec;
}

auto Parser::expect1(TokenKind type, std::source_location loc) -> bool
{
  if (mCursor.peek().getKind() != type) {
    error(loc);
    return false;
  }
  return true;
}
auto Parser::consume1(TokenKind type, std::source_location loc) -> bool
{
  if (!expect(type)) {
    error(loc);
    return false;
  }
  mCursor.skip();
  return true;
}

void Parser::error(std::source_location& loc)
{
  std::cerr << std::format("file: {}:{}:{}\nUnexpected: {}\n", loc.file_name(), loc.line(), loc.column(),
                           mCursor.peek().toString());
}