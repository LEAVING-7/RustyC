#pragma once
#include "Diagnostic.hpp"
#include "Token.hpp"
#include "common.hpp"
#include <cassert>
#include <iterator>

template <std::contiguous_iterator Iter>
class Cursor {
public:
  using difference_type = decltype(std::declval<Iter>() - std::declval<Iter>());
  using value_type = decltype(*std::declval<Iter>());

private:
  Iter const mBegin;
  Iter const mEnd;
  Iter mCurrent;

public:
  Cursor(Iter begin, Iter end) : mBegin(begin), mEnd(end), mCurrent(begin) {}

  auto isEnd() const -> bool { return mCurrent == mEnd; }
  auto curr() -> Iter { return mCurrent; }
  auto prev(difference_type n = 1) -> Iter { return std::prev(mCurrent, n); }
  auto peek(difference_type n = 0) const -> value_type& { return const_cast<value_type&>(*(mCurrent + n)); }
  auto skip(difference_type n = 1) { std::advance(mCurrent, n); }

  auto reset() { mCurrent = mBegin; }
  auto reset(Iter start) { mCurrent = start; }
};

class Lexer {
  enum class IntegerType {
    None,
    i8,
    i16,
    i32,
    i64,
    u8,
    u16,
    u32,
    u64,
  };

public:
  Cursor<char const*> mCursor;
  char const* mLineHead;
  u32 mLine;
  u32 mColumn;
  DiagnosticsEngine& mDiags;

public:
  Lexer(std::string_view buffer, DiagnosticsEngine& diag)
      : mCursor(buffer.data(), buffer.data() + buffer.size()), mLine(1), mColumn(0), mLineHead(buffer.data()),
        mDiags(diag)
  {
  }
  ~Lexer() = default;
  auto tokenize() -> std::vector<Token>;

private:
  auto getLoc() -> SMLoc { return SMLoc{}; }

  void skipUntil(std::function<bool(char)>&& fn);
  auto nextToken() -> Token;
  auto skipWhiteSpace();

  auto scanStringLiteral() -> Token;
  auto scanPunct() -> Token;
  auto scanIntegerSuffix() -> IntegerType;
  auto scanIdentifier() -> Token;
  auto scanNumber() -> Token;
  auto scanFloat() -> Token;
  auto scanInteger(i32 base) -> Token;

  auto skip() -> void { mCursor.skip(); }
  auto peek() -> char { return mCursor.peek(); }
};
