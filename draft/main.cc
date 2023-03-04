#include "frontend/Lexer.hpp"
#include "frontend/Parser.hpp"
#include <iostream>


namespace dd {

} // namespace dd

int main(int argc, char* argv[])
{
  auto code = R"(
let i = 123;
x = -x + -1 * !s / 2 -9%3;
if a == 234 & 2 * 8 {
  k = 123;
} else if x == 3 {
  b = 123.123;
};
  )";
  llvm::SourceMgr mgr{};
  DiagnosticsEngine engine{mgr};

  auto tokens = Lexer{code, engine}.tokenize();
  auto parser = Parser(tokens, engine);
  std::string str;
  auto stmts = parser.parseStmts();

  StringifyStmt stringify{str};
  for (auto& stmt : stmts) {
    stringify.visitStmt(stmt.get());
  }

  std::cout << stringify.str;
}