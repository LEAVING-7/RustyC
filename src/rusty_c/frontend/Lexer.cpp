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
  char ch = mCursor.peek();
  while (::isspace(ch) || ch == '\n') {
    switch (ch) {
    case '\n': {
      ++mLine;
      mCursor.skip();
      mLineHead = mCursor.curr();
    } break;
    default: {
      mCursor.skip();
    } break;
    }
    ch = mCursor.peek();
  }
}

auto Lexer::scanIdentifier() -> Token
{
  auto start = mCursor.curr();
  mCursor.skip();
  while (IsIdentifier(mCursor.peek()) || IsDecDigit(mCursor.peek())) {
    mCursor.skip();
  }

  std::string value(start, mCursor.curr());

  auto iter = GetKwMap().find(value);
  if (iter == GetKwMap().end()) {
    return {mLine, mColumn, TokenKind::Identifier, value};
  } else {
    if (iter->second == TokenKind::Kwtrue) {
      return {mLine, mColumn, TokenKind::NumberConstant, true};
    } else if (iter->second == TokenKind::Kwfalse) {
      return {mLine, mColumn, TokenKind::NumberConstant, false};
    } else [[likely]] { // keyword
      return {mLine, mColumn, iter->second, value};
    }
  }
}

auto Lexer::scanIntegerSuffix() -> Lexer::IntegerType
{
  if (char ch = mCursor.peek(); ch == 'i') {
    mCursor.skip();
    if (char st = mCursor.peek(); st == '8') {
      mCursor.skip();
      return IntegerType::i8;
    } else if (st == '1') {
      mCursor.skip();
      if (char sc = mCursor.peek(); sc == '6') {
        mCursor.skip();
        return IntegerType::i16;
      }
      assert(0);
    } else if (st == '3') {
      mCursor.skip();
      if (char sc = mCursor.peek(); sc == '2') {
        mCursor.skip();
        return IntegerType::i32;
      }
      assert(0);
    } else if (st == '6') {
      mCursor.skip();
      if (char sc = mCursor.peek(); sc == '4') {
        mCursor.skip();
        return IntegerType::i64;
      }
      assert(0);
    }
  } else if (ch == 'u') {
    mCursor.skip();
    if (char st = mCursor.peek(); st == '8') {
      mCursor.skip();
      return IntegerType::u8;
    } else if (st == '1') {
      mCursor.skip();
      if (char sc = mCursor.peek(); sc == '6') {
        mCursor.skip();
        return IntegerType::u16;
      }
      assert(0);
    } else if (st == '3') {
      mCursor.skip();
      if (char sc = mCursor.peek(); sc == '2') {
        mCursor.skip();
        return IntegerType::u32;
      }
      assert(0);
    } else if (st == '6') {
      mCursor.skip();
      if (char sc = mCursor.peek(); sc == '4') {
        mCursor.skip();
        return IntegerType::u64;
      }
      assert(0);
    }
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
    while (mCursor.peek() == '_') {
      mCursor.skip();
    }
  }

  u64 value = CharToInt(mCursor.peek());
  mCursor.skip();
  while (IsNDigit(mCursor.peek(), base) || mCursor.peek() == '_') {
    if (mCursor.peek() == '_') {
      mCursor.skip();
      continue;
    }
    value = value * base + CharToInt(mCursor.peek());
    mCursor.skip();
  }
  auto type = scanIntegerSuffix();
  switch (type) {
  case IntegerType::i8:
    return {mLine, mColumn, TokenKind::NumberConstant, (i8)value};
  case IntegerType::i16:
    return {mLine, mColumn, TokenKind::NumberConstant, (i16)value};
  case IntegerType::i32:
    return {mLine, mColumn, TokenKind::NumberConstant, (i32)value};
  case IntegerType::i64:
    return {mLine, mColumn, TokenKind::NumberConstant, (i64)value};
  case IntegerType::u8:
    return {mLine, mColumn, TokenKind::NumberConstant, (u8)value};
  case IntegerType::u16:
    return {mLine, mColumn, TokenKind::NumberConstant, (u16)value};
  case IntegerType::u32:
    return {mLine, mColumn, TokenKind::NumberConstant, (u32)value};
  case IntegerType::u64:
    return {mLine, mColumn, TokenKind::NumberConstant, (u64)value};
  case IntegerType::None: // default case i32
    return {mLine, mColumn, TokenKind::NumberConstant, (i32)value};
    break;
  }
}

auto Lexer::scanFloat() -> Token
{
  auto start = mCursor.curr();
  char* end = nullptr;
  double value = std::strtod(start, &end);
  if (end == nullptr) {
    assert(0);
  }

  mCursor.skip(std::abs(start - end));

  if (char ch = mCursor.peek(); ch == '_') {
    mCursor.skip();
    if (mCursor.peek() == 'f' && mCursor.peek(1) == '3' && mCursor.peek(2) == '2') {
      mCursor.skip(3);
      return {mLine, mColumn, TokenKind::NumberConstant, (float)value};
    } else if (mCursor.peek() == 'f' && mCursor.peek(1) == '6' && mCursor.peek(2) == '4') {
      mCursor.skip(3);
      return {mLine, mColumn, TokenKind::NumberConstant, (double)value};
    } else {
      assert(0);
    }
  }
  return {mLine, mColumn, TokenKind::NumberConstant, (double)value};
}

auto Lexer::scanNumber() -> Token
{
  auto start = mCursor.curr();

  i32 base = 10;

  if (mCursor.peek() == '0') {
    mCursor.skip();
    if (mCursor.peek() == 'b') {
      base = 2;
      mCursor.skip();
    } else if (mCursor.peek() == 'o') {
      base = 8;
      mCursor.skip();
    } else if (mCursor.peek() == 'x') {
      base = 16;
      mCursor.skip();
    }
  }

  while (IsNDigit(mCursor.peek(), base) || mCursor.peek() == '_') {
    mCursor.skip();
  }

  if (char ch = mCursor.peek(); ch == '.' 
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
  mColumn = mCursor.curr() - mLineHead + 1;
  skipWhiteSpace();
  if (auto ch = mCursor.peek(); IsIdentifier(ch)) {
    return scanIdentifier();
  } else if (IsDecDigit(ch)) {
    return scanNumber();
  } else if (IsPunct(ch)) {
    return scanPunct();
  } else {
    while (!mCursor.isEnd()) {
      mCursor.skip();
    }
    return {mLine, mColumn, TokenKind::END};
  }
}

auto Lexer::scanPunct() -> Token
{
  TokenKind type = TokenKind::END;
  switch (auto ch = mCursor.peek(); ch) {
  case '+': {
    type = TokenKind::PunPlus;
    mCursor.skip();
  } break;
  case '-': {
    if (ch = mCursor.peek(1); ch == '>') {
      type = TokenKind::PunRArrow;
      mCursor.skip(2);
    } else {
      type = TokenKind::PunMinus;
      mCursor.skip();
    }
  } break;
  case '*': {
    type = TokenKind::PunStar;
    mCursor.skip();
  } break;
  case '/': {
    type = TokenKind::PunSlash;
    mCursor.skip();
  } break;
  case '%': {
    type = TokenKind::PunPercent;
    mCursor.skip();
  } break;
  case '=': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunEqEq;
      mCursor.skip(2);
    } else {
      type = TokenKind::PunEq;
      mCursor.skip();
    }
  } break;
  case '<': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunLe;
      mCursor.skip(), mCursor.skip();
    } else {
      type = TokenKind::PunLt;
      mCursor.skip();
    }

  } break;
  case '>': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunGe;
      mCursor.skip(), mCursor.skip();
    } else {
      type = TokenKind::PunGt;
      mCursor.skip();
    }

  } break;
  case ';': {
    type = TokenKind::PunSemi;
    mCursor.skip();

  } break;
  case '!': {
    if (ch = mCursor.peek(1); ch == '=') {
      type = TokenKind::PunNe;
      mCursor.skip(), mCursor.skip();
    } else {
      type = TokenKind::PunNot;
      mCursor.skip();
    }
  } break;
  case '{': {
    type = TokenKind::PunLBrace;
    mCursor.skip();
  } break;
  case '}': {
    type = TokenKind::PunRBrace;
    mCursor.skip();
  } break;
  case '(': {
    type = TokenKind::PunLParen;
    mCursor.skip();
  } break;
  case ')': {
    type = TokenKind::PunRParen;
    mCursor.skip();
  } break;
  case '[': {
    type = TokenKind::PunLBrack;
    mCursor.skip();
  } break;
  case ']': {
    type = TokenKind::PunRBrack;
    mCursor.skip();
  } break;
  case ',': {
    type = TokenKind::PunComma;
    mCursor.skip();
  } break;
  case ':': {
    type = TokenKind::PunColon;
    mCursor.skip();
  } break;
  case '&': {
    type = TokenKind::PunAnd;
    mCursor.skip();
  } break;
  case '\'': {
    type = TokenKind::PunSQuote;
    mCursor.skip();
  } break;
  case '"': {
    type = TokenKind::PunDQuote;
    mCursor.skip();
  } break;
  }

  return {mLine, mColumn, type};
}