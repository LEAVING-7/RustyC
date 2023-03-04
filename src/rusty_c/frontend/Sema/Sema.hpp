#pragma once

#include "frontend/Lexer.hpp"

#include "Scope.hpp"
// namespace {
class ScopeGuard {
  Scopes& scopes;

public:
  ScopeGuard(Scopes& scopes) : scopes(scopes) { scopes.enterScope(); }
  ~ScopeGuard() { scopes.leaveScope(); }
};
class Sema {
  DiagnosticsEngine& mDiags;
  Scopes mScopes;

public:
  Sema(DiagnosticsEngine& diags) : mDiags(diags) {}

  auto declare() {}

  auto enterScope() -> ScopeGuard { return {mScopes}; }
};
// } // namespace