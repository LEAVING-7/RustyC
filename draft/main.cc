#include "Frontend/Lexer.hpp"
#include "Frontend/Parser.hpp"

#include <iostream>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/raw_ostream.h>

int main(int argc, char* argv[])
{
  /*   auto code = R"(
  fn kk(bar: i32) -> f64 {
    1234.0
  }

  fn ff() -> i32 {
    let x = 123;
    let kkk = if x == 0 {
        0;
    } else {
        1;
    };
    return 0;
  }

  fn main() {
    ff();
  }

    )";
    std::unordered_map<std::string, std::unique_ptr<TypeBase>> map;
    llvm::SourceMgr mgr{};
    DiagnosticsEngine engine{mgr};

    auto sema = Sema(engine);
    auto tokens = Lexer{code, engine}.tokenize();
    auto parser = Parser(tokens, engine);
    auto crate = parser.parseCrate();

    sema.actOnCrate(&crate); */
  // std::string str;

  // StringifyStmt stringify{str};

  // std::cout << stringify.str;

  llvm::InitLLVM vm(argc, argv);
  llvm::SmallVector<char const*, 256> args(argv + 1, argv + argc);

  llvm::outs() << "RustyC v0.0.1\n";

  for (char const* filename : args) {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> fileOrError = llvm::MemoryBuffer::getFile(filename);
    if (std::error_code bufErr = fileOrError.getError()) {
      llvm::errs() << "Error reading: " << filename << ':' << bufErr.message() << "\n";
      continue;
    }
    llvm::SourceMgr srcMgr;
    DiagnosticsEngine diags{srcMgr};

    srcMgr.AddNewSourceBuffer(std::move(fileOrError.get()), llvm::SMLoc());
    auto tokens = Lexer{srcMgr, diags}.tokenize();
    auto parser = Parser{tokens, diags};
    auto crate = parser.parseCrate();
    auto sema = Sema{diags};
    sema.actOnCrate(&crate);
  }
}