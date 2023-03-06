#pragma once
#include "common.hpp"
#include "utils/utils.hpp"

#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"

enum class DiagId {
#define DIAG(id, level, msg) id,
#include "Diagnostic.def"
};

class DiagnosticsEngine {
  static auto GetDiagnosticText(DiagId id) -> char const*;
  static auto GetDiagnosticKind(DiagId id) -> llvm::SourceMgr::DiagKind;

  llvm::SourceMgr& mSrcMgr;
  u32 mNumErrors;

public:
  DiagnosticsEngine(llvm::SourceMgr& srcMgr) : mSrcMgr(srcMgr), mNumErrors(0) {}

  auto numErrors() -> u32 { return mNumErrors; }
  template <typename... Args>
  void report(llvm::SMLoc loc, DiagId id, Args&&... args)
  {
    std::string msg = utils::vformat(GetDiagnosticText(id), utils::make_format_args(std::forward<Args>(args)...));
    auto kind = GetDiagnosticKind(id);
    mSrcMgr.PrintMessage(loc, kind, msg);
    mNumErrors += (kind == llvm::SourceMgr::DK_Error) ? 1 : 0;
  }
};
