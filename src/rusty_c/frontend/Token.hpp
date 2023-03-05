#pragma once
#include "TokenKind.hpp"
#include <variant>
class Token {
public:
  using ValueType = std::variant<bool, i8, i16, i32, i64, u8, u16, u32, u64, float, double, std::string>;

private:
  TokenKind mType;
  ValueType mValue;
  u32 mLine, mColumn;

public:
  Token(u32 line, u32 column, TokenKind type) : mLine(line), mColumn(column), mType(type) {}
  template <typename T>
  Token(u32 line, u32 column, TokenKind type, T&& value)
      : mType(type), mLine(line), mColumn(column), mValue(std::forward<T>(value))
  {
  }
  auto getKind() const -> TokenKind { return this->mType; }
  auto is(TokenKind type) const -> bool { return this->mType == type; }
  auto isOneOf(TokenKind t1, TokenKind t2) const -> bool { return is(t1) || is(t2); }
  template <typename... Ts>
  auto isOneOf(TokenKind t1, TokenKind t2, Ts... ts) const -> bool
  {
    return is(t1) || isOneOf(t2, ts...);
  }
  auto getValue() const -> ValueType const& { return mValue; }
  auto getValueIndex() const -> size_t { return mValue.index(); }

  template <typename T>
  decltype(auto) get()
  {
    return std::get<T>(mValue);
  }
  template <typename T>
  decltype(auto) get() const
  {
    return std::get<T>(mValue);
  }

  auto toString() -> std::string;
};

auto ToString(Token::ValueType const& v) -> std::string;