#include "Diagnostic.hpp"
static char const* gDiagnosticText[] = {
#define DIAG(id, level, msg) msg,
#include "Diagnostic.def"
};

static llvm::SourceMgr::DiagKind gDiagnosticKind[] = {
#define DIAG(id, level, msg) llvm::SourceMgr::DK_##level,
#include "Diagnostic.def"
};

auto DiagnosticsEngine::GetDiagnosticText(DiagId id) -> char const*
{
  return gDiagnosticText[static_cast<std::underlying_type_t<DiagId>>(id)];
}

auto DiagnosticsEngine::GetDiagnosticKind(DiagId id) -> llvm::SourceMgr::DiagKind
{
  return gDiagnosticKind[static_cast<std::underlying_type_t<DiagId>>(id)];
}
