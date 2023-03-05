#include "Token.hpp"

auto ToString(Token::ValueType const& v) -> std::string
{
  return std::visit(
      []<typename T>(T const& v) {
        if constexpr (std::is_arithmetic_v<T>) {
          return std::to_string(v);
        } else if constexpr (std::is_same_v<std::string, T>) {
          return v;
        } else if constexpr (std::is_same_v<bool, T>) {
          if (v == true) {
            return "true";
          } else if (v == false) {
            return "false";
          }
        }
      },
      v);
}
auto Token::toString() -> std::string
{
  return "Not implemented yet.";
}
