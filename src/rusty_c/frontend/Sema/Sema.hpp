#pragma once

#include "frontend/Lexer.hpp"

#include "Scope.hpp"
// namespace {
class Sema {
  DiagnosticsEngine& mDiags;
  Scopes mScopes;

public:
  Sema(DiagnosticsEngine& diags) : mDiags(diags) {}
  

  void enterScope() { return mScopes.enterScope(); }
  void leaveScope() { return mScopes.leaveScope(); }
};
// } // namespace