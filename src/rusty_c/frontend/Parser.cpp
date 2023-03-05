#include "Parser.hpp"
#include "utils/utils.hpp"
#include <iostream>

bool IsUnaryTok(TokenKind tok) { return UnaryExpr::MapKind(tok) != UnaryExpr::Kind::SIZE; }

#define GOTO_ERROR_IF(cond)                                                                                            \
  if (cond)                                                                                                            \
    goto ERROR;

auto Parser::parseExprStmt(PredT pred) -> std::unique_ptr<ExprStmt>
{
  if (auto& tok = peek(); tok.isOneOf(PunLBrace, Kwif, Kwwhile, Kwloop)) {
    auto ret = std::make_unique<ExprStmt>(parseExprWithBlock(pred));
    skipIf(PunSemi);
    return ret;
  } else {
    auto ret = std::make_unique<ExprStmt>(parseExprWithoutBlock(pred));
    skipIf(PunSemi);
    return ret;
  }
}

auto Parser::parseExpr(PredT pred) -> std::unique_ptr<Expr>
{
  if (peek().isOneOf(Kwloop, Kwwhile, PunLBrace, Kwif)) {
    return parseExprWithBlock(pred);
  } else {
    return parseExprWithoutBlock(pred);
  }
}

auto Parser::parseExprWithoutBlock(PredT pred) -> std::unique_ptr<Expr>
{
  if (peek().is(Kwreturn)) {
    return parseReturnExpr();
  }
  return parseBinaryExpr(pred, -1);
}

auto Parser::parseLiteralExpr() -> std::unique_ptr<LiteralExpr>
{
  auto const& tok = peek();
  skip();
  if (tok.is(Identifier)) {
    return std::make_unique<LiteralExpr>(LiteralExpr::Kind::Identifier, tok.getValue());
  } else {
    auto ty = static_cast<LiteralExpr::Kind>(tok.getValueIndex());
    return std::make_unique<LiteralExpr>(ty, tok.getValue());
  }
}

auto Parser::parseBinaryExpr(PredT pred, i32 bp) -> std::unique_ptr<Expr>
{
  if (peek().is(END) && pred(peek())) {
    return nullptr;
  }
  std::unique_ptr<Expr> left;

  if (auto tok = peek().getKind(); tok == PunLParen) {
    left = parseGroupedExpr(pred); // parse grouped expression
  } else if (tok == NumberLiteral || tok == StringLiteral) {
    left = parseLiteralExpr();
  } else if (tok == Identifier) {
    if (peek(1).is(PunLParen)) { // parse function call expression
      auto callee = peek().get<std::string>();
      skip();
      skip();
      std::vector<std::unique_ptr<Expr>> args{};
      while (!peek().is(PunRParen)) {
        args.push_back(parseExpr([](auto tok) { return tok.is(PunComma); }));
        skipIf(PunComma);
      }
      skip(); // skip RParen
      left = std::make_unique<CallExpr>(callee, std::move(args));
    } else {
      left = parseLiteralExpr();
    }
  } else if (auto kind = UnaryExpr::MapKind(tok); kind != UnaryExpr::Kind::SIZE) { // parse unary expression
    auto [bp] = UnaryExpr::BindingPower(kind);                                     // prefix
    skip();
    auto right = parseBinaryExpr(pred, bp);
    left = std::make_unique<UnaryExpr>(kind, std::move(right));
  } else {
    mDiags.report(SMLoc(), DiagId::ErrExpectedExpr);
  }

  while (!peek().is(END) && !pred(peek())) {
    auto tokKind = peek().getKind();
    auto op = BinaryExpr::MapKind(tokKind);
    if (op == BinaryExpr::Kind::SIZE) {
      mDiags.report(SMLoc(), DiagId::ErrInvalidBinaryOp, TokenKindToString(tokKind));
      return nullptr;
    }
    auto [bpl, bpr] = BinaryExpr::BindingPower(op);
    if (bpl < bp) {
      break;
    }
    skip();
    auto right = parseBinaryExpr(pred, bpr);
    left = std::make_unique<BinaryExpr>(op, std::move(left), std::move(right));
  }
  return left;
}

auto Parser::parseGroupedExpr(PredT pred) -> std::unique_ptr<GroupedExpr>
{
  consume(PunLParen);
  auto expr = parseExpr([](auto v) { return v.is(PunRParen); });
  consume(PunRParen);
  return std::make_unique<GroupedExpr>(std::move(expr));
}

auto Parser::isItemStart(Token const& tok) -> bool
{
  return tok.is(Kwfn); // TODO: add more
}

auto Parser::parseItem() -> std::unique_ptr<Item>
{
  if (peek().is(Kwfn)) {
    return parseFunctionItem();
  }
  utils::Unreachable(utils::SrcLoc::current());
}

auto Parser::parseCrate() -> Crate
{
  std::vector<std::unique_ptr<Item>> items{};
  while (!peek().is(END)) {
    items.push_back(parseItem());
  }
  return {std::move(items)};
}

auto Parser::parseStmt(PredT pred) -> std::unique_ptr<Stmt>
{
  if (peek().is(Kwlet)) {
    return parseLetStmt();
  } else if (peek().is(PunSemi)) { // skip empty statement
    skip();
    return nullptr;
  } else {
    return parseExprStmt(pred);
  }
}

auto Parser::parseLetStmt() -> std::unique_ptr<LetStmt>
{
  consume(Kwlet);
  expect(Identifier);
  auto name = peek().get<std::string>();
  skip();

  std::unique_ptr<TypeBase> expectType{nullptr};
  if (peek().is(PunColon)) {
    skip();
    expectType = parseType();
  }

  consume(PunEq);
  auto expr = parseExpr([](auto v) { return v.is(PunSemi); });
  skip(); // skip semicolon
  return std::make_unique<LetStmt>(name, std::move(expectType), std::move(expr));
}

auto Parser::parseExprWithBlock(PredT pred) -> std::unique_ptr<ExprWithBlock>
{

  if (peek().is(PunLBrace)) {
    return parseBlockExpr();
  } else if (peek().isOneOf(Kwloop, Kwwhile)) {
    return parseLoopExpr();
  } else if (peek().is(Kwif)) {
    return parseIfExpr();
  }
  utils::Unreachable(utils::SrcLoc::current());
}

auto Parser::parseBlockExpr() -> std::unique_ptr<BlockExpr>
{
  consume(PunLBrace);
  std::vector<std::unique_ptr<Stmt>> stmts{};
  std::vector<std::unique_ptr<Item>> items{};

  std::unique_ptr<Expr> ret{nullptr};

  bool isItemEnd = false;

  while (!peek().is(PunRBrace)) { // TODO expr without block
    if (isItemStart(peek())) {
      items.push_back(parseItem());
      isItemEnd = true;
      continue;
    }
    auto stmt = parseStmt([](auto v) { return v.isOneOf(PunRBrace, PunSemi); });
    stmts.push_back(std::move(stmt));
    isItemEnd = false;
  }
  if (!peek(-1).is(PunSemi) && !isItemEnd) {
    auto back = std::move(stmts.back());
    stmts.pop_back();
    ret = std::move(back->as<ExprStmt>()->mExpr); // move the expression out of the statement
  }
  consume(PunRBrace);
  return std::make_unique<BlockExpr>(std::move(stmts), std::move(items), std::move(ret));
}
auto Parser::parseIfExpr() -> std::unique_ptr<IfExpr>
{
  consume(Kwif);
  auto cond = parseExpr([](auto v) { return v.is(PunLBrace); });
  auto block = parseBlockExpr();
  auto else_ = std::unique_ptr<ExprWithBlock>{nullptr};
  if (peek().is(Kwelse)) {
    skip(); // skip 'else'
    if (peek().is(Kwif)) {
      else_ = parseIfExpr();
    } else if (peek().is(PunLBrace)) {
      else_ = parseBlockExpr();
    } else {
      assert(0);
    }
  }
  return std::make_unique<IfExpr>(std::move(cond), std::move(block), std::move(else_));
}
auto Parser::parseLoopExpr() -> std::unique_ptr<LoopExpr>
{
  if (peek().is(Kwloop)) {
    return parseInfiniteLoopExpr();
  } else if (peek().is(Kwwhile)) {
    return parsePredicateLoopExpr();
  }
  utils::Unreachable(utils::SrcLoc::current());
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
  auto cond = parseExpr([](auto v) { return v.is(PunLBrace); });
  auto body = parseBlockExpr();
  return std::make_unique<PredicateLoopExpr>(std::move(cond), std::move(body));
}

auto Parser::parseStmts() -> std::vector<std::unique_ptr<Stmt>>
{
  std::vector<std::unique_ptr<Stmt>> vec{};
  while (!peek().is(END)) {
    vec.push_back(parseStmt());
  }
  return vec;
}

auto Parser::parseFunctionItem() -> std::unique_ptr<FunctionItem>
{
  std::vector<std::string> paramNames{};
  std::vector<std::unique_ptr<TypeBase>> paramTypes{};

  consume(Kwfn);
  expect(Identifier);
  auto identifier = peek().get<std::string>();
  skip();
  consume(PunLParen);
  while (!peek().is(PunRParen)) {
    expect(Identifier);
    paramNames.push_back(peek().get<std::string>());
    skip();
    consume(PunColon);
    paramTypes.push_back(parseType());
  }
  consume(PunRParen);
  if (peek().is(PunRArrow)) {
    skip();
    auto retType = parseType();
    auto body = parseBlockExpr();
    return std::make_unique<FunctionItem>(identifier, std::move(paramNames),
                                          std::make_unique<FunctionType>(std::move(paramTypes), std::move(retType)),
                                          std::move(body));
  } else {
    // return unit type
    auto body = parseBlockExpr();
    return std::make_unique<FunctionItem>(identifier, std::move(paramNames),
                                          std::make_unique<FunctionType>(std::move(paramTypes)), std::move(body));
  }
}

auto Parser::parseReturnExpr() -> std::unique_ptr<ReturnExpr>
{
  consume(Kwreturn);
  auto expr = parseExpr([](auto v) { return v.is(PunSemi); });
  return std::make_unique<ReturnExpr>(std::move(expr));
}

//===----------------------------------------------------------------------===//
// Type parsing
//===----------------------------------------------------------------------===//

auto Parser::parseType() -> std::unique_ptr<TypeBase>
{
  auto tokKind = peek();
  if (tokKind.is(PunNot)) {
    return std::make_unique<Never>(TypeNever);
  } else if (tokKind.is(Identifier)) {
    auto typeName = peek().get<std::string>();
    skip();
    // if is a numeric type or boolean
    if (auto type = GetNumBoolMap(typeName); type != nullptr) {
      return type;
    }
  } else if (tokKind.is(Kwfn)) {
    return parseFunctionType();
  } else if (tokKind.is(PunLParen)) {
    return parseTupleType();
  }
  utils::Unreachable(utils::SrcLoc::current());
}

auto Parser::parseTupleType() -> std::unique_ptr<TupleType>
{
  consume(PunLParen);
  std::vector<std::unique_ptr<TypeBase>> types{};
  while (!peek().is(PunRParen)) {
    types.push_back(parseType());
    skipIf(PunComma);
  }
  consume(PunRParen);
  return std::make_unique<TupleType>(std::move(types));
}

auto Parser::parseFunctionType() -> std::unique_ptr<FunctionType>
{
  consume(Kwfn);
  consume(PunLParen);
  std::vector<std::unique_ptr<TypeBase>> args{};
  while (!peek().is(PunRParen)) {
    args.push_back(parseType());
    skipIf(PunComma);
  }
  consume(PunRParen);
  // default unit type
  std::unique_ptr<TypeBase> ret = std::make_unique<TupleType>();
  if (peek().is(PunRArrow)) {
    skip();
    ret = parseType();
  }
  return std::make_unique<FunctionType>(std::move(args), std::move(ret));
}

auto Parser::expect(TokenKind type) -> bool
{
  if (peek().getKind() != type) {
    mDiags.report(SMLoc(), DiagId::ErrUnexpected, TokenKindToString(type), TokenKindToString(peek().getKind()));
    return false;
  }
  return true;
}
auto Parser::consume(TokenKind type) -> bool
{
  if (!expect(type)) {
    return false;
  }
  skip();
  return true;
}

auto Parser::skipAfter(std::function<bool(Token const&)>&& pred) -> void
{
  while (!pred(peek()) && !peek().is(END)) {
    skip();
  }
}

auto Parser::skipIf(TokenKind type) -> void
{
  if (peek().getKind() == type) {
    skip();
  }
}

void Parser::skipUntil(std::function<bool(Token const&)>&& pred)
{
  while (pred(peek()) && !peek().is(END)) {
    skip();
  }
}

void Parser::error() { puts("error"); }

#undef consume
#undef GOTO_ERROR_IF