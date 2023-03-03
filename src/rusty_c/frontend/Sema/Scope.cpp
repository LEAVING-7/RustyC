#include "Scope.hpp"

auto Scopes::insert(LetStmt* stmt) -> bool { return mScopes.back().insert({stmt->mName, stmt}).second; }
auto Scopes::lookup(std::string const& name) -> LetStmt*
{
  assert(!mScopes.empty());
  auto currentScope = mScopes.size() - 1;
  for (auto curr = currentScope; curr >= 0; curr--) {
    auto iter = mScopes[curr].find(name);
    if (iter != mScopes[curr].end()) {
      return iter->second;
    }
  }
  return nullptr;
}
void Scopes::enterScope() { mScopes.push_back({}); }
void Scopes::leaveScope()
{
  if (!mScopes.empty()) {
    mScopes.pop_back();
  }
}