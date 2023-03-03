#include "frontend/Lexer.hpp"
#include "frontend/Parser.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
  auto code = R"(
let i = 0;

while i < 10 {
    i = i + 1;
}
  )";
  llvm::SourceMgr mgr{};
  DiagnosticsEngine engine{mgr};
  auto tokens = Lexer{code, engine}.tokenize();
  auto parser = Parser(tokens, engine);
  std::string str;
  StringifyStmt stmt{str};
  for (auto stmts = parser.parseStmts(); auto& st : stmts) {
    stmt.visitStmt(st.get());
    str += '\n';
  }
  std::cout << str << '\n';
}