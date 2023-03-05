#include "Lexer.hpp"

#include <cctype>

inline bool IsBinDigit(char c) { return c == '0' || c == '1'; }
inline bool IsOctDigit(char c) { return '0' <= c && c <= '7'; }
inline bool IsDecDigit(char c) { return ::isdigit(c); }
inline bool IsHexDigit(char c) { return ::isxdigit(c); }
bool IsNDigit(char c, i32 base)
{
  if (base == 2) {
    return IsBinDigit(c);
  } else if (base == 8) {
    return IsOctDigit(c);
  } else if (base == 16) {
    return IsHexDigit(c);
  } else {
    return IsDecDigit(c);
  }
}

bool IsNDigitOrUnderscore(char c, i32 base) { return IsNDigit(c, base) || '_' == c; }

bool IsLetter(char c) { return ::isalpha(c); }
bool IsIdentifier(char c) { return IsLetter(c) || c == '_'; }
bool IsAlnum(char c) { return ::isalnum(c); }
bool IsPunct(char c)
{
  return c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}' || c == '.' || c == '&' || c == '*' ||
         c == '+' || c == '-' || c == '~' || c == '!' || c == '/' || c == '%' || c == '<' || c == '>' || c == '^' ||
         c == '|' || c == '?' || c == ':' || c == ';' || c == '=' || c == ',' || c == '\'' || c == '"';
}

auto Lexer::tokenize() -> std::vector<Token>
{
  std::vector<Token> vec{};
  while (!mCursor.isEnd()) {
    vec.push_back(nextToken());
  }
  return vec;
}

auto Lexer::skipWhiteSpace()
{
  char ch = peek();
  while (::isspace(ch) || ch == '\n') {
    switch (ch) {
    case '\n': {
      skip();
    } break;
    default: {
      skip();
    } break;
    }
    ch = peek();
  }
}

auto Lexer::scanStringLiteral() -> Token
{
  auto start = mCursor.curr();
  skip();
  while (peek() != '"') {
    skip();
  }
  skip();
  std::string value(start, mCursor.curr());
  return {curr(), TokenKind::StringLiteral, value};
}

auto Lexer::scanIdentifier() -> Token
{
  auto start = mCursor.curr();
  skip();
  while (IsIdentifier(peek()) || IsDecDigit(peek())) {
    skip();
  }

  std::string value(start, mCursor.curr());

  auto iter = GetKwMap().find(value);
  if (iter == GetKwMap().end()) {
    return {curr(), TokenKind::Identifier, value};
  } else {
    if (iter->second == TokenKind::Kwtrue) {
      return {curr(), TokenKind::NumberLiteral, true};
    } else if (iter->second == TokenKind::Kwfalse) {
      return {curr(), TokenKind::NumberLiteral, false};
    } else [[likely]] { // keyword
      return {curr(), iter->second, value};
    }
  }
}

auto Lexer::scanIntegerSuffix() -> Lexer::IntegerType
{
  if (char ch = peek(); ch == 'i' || ch == 'u') {
    auto start = mCursor.curr();
    std::string_view view{start, 3}; // what if not long enough
    if (view.starts_with("i8")) {
      mCursor.skip(2);
      return IntegerType::i8;
    } else if (view.starts_with("i16")) {
      mCursor.skip(3);
      return IntegerType::i16;
    } else if (view.starts_with("i32")) {
      mCursor.skip(3);
      return IntegerType::i16;
    } else if (view.starts_with("i64")) {
      mCursor.skip(3);
      return IntegerType::i16;
    } else if (view.starts_with("u8")) {
      mCursor.skip(2);
      return IntegerType::u8;
    } else if (view.starts_with("u16")) {
      mCursor.skip(3);
      return IntegerType::u16;
    } else if (view.starts_with("u32")) {
      mCursor.skip(3);
      return IntegerType::u32;
    } else if (view.starts_with("u64")) {
      mCursor.skip(3);
      return IntegerType::u64;
    }
    skipUntil([](char c) { return !IsLetter(c) && !IsDecDigit(c); });
    mDiags.report(getLoc(), DiagId::ErrInvalidIntegerSuffix, std::string_view(start, mCursor.curr()));
  }
  return IntegerType::None;
}

static auto CharToInt(char c) -> i32
{
  if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('a' <= c && c <= 'z') {
    return c - 'a' + 10;
  } else if ('A' <= c && c <= 'Z') {
    return c - 'A' + 10;
  }
  assert(0);
  return 0;
}

auto Lexer::scanInteger(i32 base) -> Token
{
  bool hasPrefix = false;
  if (base == 16 || base == 8 || base == 2) {
    mCursor.skip(2);
    hasPrefix = true;
  }

  if (hasPrefix) {
    while (peek() == '_') {
      skip();
    }
  }

  u64 value = CharToInt(peek());
  skip();
  while (IsNDigit(peek(), base) || peek() == '_') {
    if (peek() == '_') {
      skip();
      continue;
    }
    value = value * base + CharToInt(peek());
    skip();
  }
  auto type = scanIntegerSuffix();
  switch (type) {
  case IntegerType::i8:
    return {curr(), TokenKind::NumberLiteral, (i8)value};
  case IntegerType::i16:
    return {curr(), TokenKind::NumberLiteral, (i16)value};
  case IntegerType::i32:
    return {curr(), TokenKind::NumberLiteral, (i32)value};
  case IntegerType::i64:
    return {curr(), TokenKind::NumberLiteral, (i64)value};
  case IntegerType::u8:
    return {curr(), TokenKind::NumberLiteral, (u8)value};
  case IntegerType::u16:
    return {curr(), TokenKind::NumberLiteral, (u16)value};
  case IntegerType::u32:
    return {curr(), TokenKind::NumberLiteral, (u32)value};
  case IntegerType::u64:
    return {curr(), TokenKind::NumberLiteral, (u64)value};
  case IntegerType::None: // default case i32
    return {curr(), TokenKind::NumberLiteral, (i32)value};
    break;
  }
}

auto Lexer::scanFloat() -> Token
{
  auto start = mCursor.curr();
  char* end = nullptr;
  double value = std::strtod(start, &end);
  if (end == nullptr) {
    skipUntil(std::not_fn(IsAlnum));
    mDiags.report(getLoc(), DiagId::ErrInvalidFloatConstant);
  }

  mCursor.skip(std::abs(start - end));

  if (char ch = peek(); ch == '_') {
    auto start = mCursor.curr();
    std::string_view view{start, 3};
    if (view.starts_with("f32")) {
      mCursor.skip(3);
      return {curr(), TokenKind::NumberLiteral, (float)value};
    } else if (view.starts_with("f64")) {
      mCursor.skip(3);
      return {curr(), TokenKind::NumberLiteral, (double)value};
    } else {
      skipUntil([](char c) { return !IsAlnum(c); });
      mDiags.report(getLoc(), DiagId::ErrInvalidFloatSuffix, std::string_view{start, mCursor.curr()});
    }
  }
  return {curr(), TokenKind::NumberLiteral, (double)value};
}

auto Lexer::scanNumber() -> Token
{
  auto start = mCursor.curr();

  i32 base = 10;

  if (peek() == '0') {
    skip();
    if (peek() == 'b') {
      base = 2;
      skip();
    } else if (peek() == 'o') {
      base = 8;
      skip();
    } else if (peek() == 'x') {
      base = 16;
      skip();
    }
  }

  while (IsNDigit(peek(), base) || peek() == '_') {
    skip();
  }

  if (char ch = peek(); ch == '.' 
  /* || ch == 'E' || ch == 'e'
   */) {
    if (base == 8) {
      assert(0);
    }

    mCursor.reset(start);
    return scanFloat();
  }

  mCursor.reset(start);
  return scanInteger(base);
}

auto Lexer::nextToken() -> Token
{
  skipWhiteSpace();
  if (auto ch = peek(); IsIdentifier(ch)) {
    return scanIdentifier();
  } else if (IsDecDigit(ch)) {
    return scanNumber();
  } else if (IsPunct(ch)) {
    return scanPunct();
  } else {
    while (!mCursor.isEnd()) {
      skip();
    }
    return {curr(), TokenKind::END};
  }
}

auto Lexer::scanPunct() -> Token
{
  TokenKind type = TokenKind::END;
  switch (auto ch = peek(); ch) {
  case '+': {
    type = TokenKind::PunPlus;
    skip();
  } break;
  case '-': {
    if (ch = mCursor.peek(1); ch == '>') {
      type = TokenKind::PunRArrow;
      mCursor.skip(2);
    } else {
      type = TokenKind::PunMinus;
      skip();
    }
  } break;
  case '*': {
    type = TokenKind::PunStar;
    skip();
  } break;
  case '/': {
    type = TokenKind::PunSlash;
    skip();
  } break;
  case '%': {
    type = TokenKind::PunPercent;
    skip();
  } break;
  case '=': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunEqEq;
      mCursor.skip(2);
    } else {
      type = TokenKind::PunEq;
      skip();
    }
  } break;
  case '<': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunLe;
      skip(), skip();
    } else {
      type = TokenKind::PunLt;
      skip();
    }

  } break;
  case '>': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunGe;
      skip(), skip();
    } else {
      type = TokenKind::PunGt;
      skip();
    }

  } break;
  case ';': {
    type = TokenKind::PunSemi;
    skip();

  } break;
  case '!': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunNe;
      skip(), skip();
    } else {
      type = TokenKind::PunNot;
      skip();
    }
  } break;
  case '{': {
    type = TokenKind::PunLBrace;
    skip();
  } break;
  case '}': {
    type = TokenKind::PunRBrace;
    skip();
  } break;
  case '(': {
    type = TokenKind::PunLParen;
    skip();
  } break;
  case ')': {
    type = TokenKind::PunRParen;
    skip();
  } break;
  case '[': {
    type = TokenKind::PunLBrack;
    skip();
  } break;
  case ']': {
    type = TokenKind::PunRBrack;
    skip();
  } break;
  case ',': {
    type = TokenKind::PunComma;
    skip();
  } break;
  case ':': {
    type = TokenKind::PunColon;
    skip();
  } break;
  case '&': {
    type = TokenKind::PunAnd;
    skip();
  } break;
  case '\'': {
    type = TokenKind::PunSQuote;
    skip();
  } break;
  case '"': {
    type = TokenKind::PunDQuote;
    skip();
  } break;
  }

  return {curr(), type};
}

void Lexer::skipUntil(std::function<bool(char)>&& pred)
{
  while (mCursor.isEnd() && pred(peek())) {
    skip();
  }
}