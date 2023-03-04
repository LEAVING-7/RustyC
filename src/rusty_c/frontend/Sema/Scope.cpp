#include "Scope.hpp"

auto Scopes::insert(std::string const& name, TypeBase* type) -> bool
{
  return mScopes.back().insert({name, type}).second;
}
auto Scopes::lookup(std::string const& name) -> TypeBase*
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