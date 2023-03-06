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
  Iter mBegin;
  Iter mEnd;
  Iter mCurrent;

public:
  Cursor() = default;
  Cursor(Cursor const&) = default;
  Cursor& operator=(Cursor const&) = default;
  Cursor(Cursor&&) = default;
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

  u32 mCurrBuffer = 0;
  DiagnosticsEngine& mDiags;
  llvm::SourceMgr& mSourceMgr;

public:
  Lexer(llvm::SourceMgr& srcMgr, DiagnosticsEngine& diag)
      : mSourceMgr(srcMgr), mDiags(diag), mCurrBuffer(srcMgr.getMainFileID())
  {
    mCursor = Cursor(srcMgr.getMemoryBuffer(mCurrBuffer)->getBufferStart(),
                     srcMgr.getMemoryBuffer(mCurrBuffer)->getBufferEnd());
  }
  ~Lexer() = default;
  auto tokenize() -> std::vector<Token>;

private:
  auto getBuffer() -> llvm::StringRef { return mSourceMgr.getMemoryBuffer(mCurrBuffer)->getBuffer(); }
  auto getLoc() -> llvm::SMLoc { return llvm::SMLoc::getFromPointer(mCursor.curr()); }

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

  auto curr() -> char const* { return mCursor.curr(); }
  auto skip() -> void { mCursor.skip(); }
  auto peek() -> char { return mCursor.peek(); }
};
