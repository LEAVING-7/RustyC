#include "Token.hpp"
auto Token::toString() -> std::string
{
  if (mType == TokenKind::Identifier || mType == TokenKind::NumberConstant) {
    return std::format("({}: {}): {}: {}", mLine, mColumn, TokenKindToString(mType),
                       std::visit(
                           []<typename T>(T const& v) {
                             if constexpr (std::is_arithmetic_v<T>) {
                               return std::to_string(v) + " with size " + std::to_string(sizeof(v));
                             } else {
                               return v;
                             }
                           },
                           mValue));
  } else {
    return std::format("({}: {}): {}", mLine, mColumn, TokenKindToString(mType));
  }
}
