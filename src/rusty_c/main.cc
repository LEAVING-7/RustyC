#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/ExtensibleRTTI.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/raw_ostream.h"
#include <concepts>
#include <iostream>


int main(int argc, char* argv[])
{
  llvm::InitLLVM X(argc, argv);
  llvm::outs() << "Hello, I'm Rusty C, 你好世界";
}