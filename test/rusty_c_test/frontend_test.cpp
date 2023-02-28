#include "frontend/Lexer.hpp"
#include "gtest/gtest.h"

#include <string>
using namespace std::string_literals;
// class LexerTest: public ::testing::Test {
//   protected:
// }

TEST(FrontendTest, LexerTest)
{
  auto codes = R"(
    i8 u8 i16 u16 i32 u32 i64 u64 f32 f64 bool true false asdf leaving hello world's ;; {}()[]!= == <= >= & -> ; :, let
    12345 0xAB_CD_EF
)";
  auto tokens = Lexer{codes}.tokenize();
  for (auto tok : tokens) {
    std::cout << tok.toString() << '\n';
  }
}
