#include "Scope.hpp"

auto Scopes::insertIdentifier(std::string const& name, std::unique_ptr<TypeBase> type) -> bool
{
  return mScopes.back().identifiers.insert({name, std::move(type)}).second;
}
auto Scopes::lookupIdentifier(std::string const& name) -> TypeBase* { return lookupIdUntil(name, 0); }

auto Scopes::lookupIdInCurr(std::string const& name) -> TypeBase*
{
  assert(!mScopes.empty());
  return mScopes.back().identifiers[name].get();
}

auto Scopes::lookupIdUntil(std::string const& name, i32 until) -> TypeBase*
{
  assert(!mScopes.empty() && until >= 0);
  i32 currentScope = mScopes.size() - 1;
  for (auto curr = currentScope; curr >= until; curr--) {
    auto iter = mScopes[curr].identifiers.find(name);
    if (iter != mScopes[curr].identifiers.end()) {
      return iter->second.get();
    }
  }
  return nullptr;
}

auto Scopes::insertItem(std::string const& name, Item* item) -> bool
{
  return mScopes.back().items.insert({name, std::move(item)}).second;
}

auto Scopes::lookupItem(std::string const& name) -> Item* { return lookupItemUntil(name, 0); };

auto Scopes::lookupItemInCurr(std::string const& name) -> Item*
{
  assert(!mScopes.empty());
  return mScopes.back().items[name];
}

auto Scopes::lookupItemUntil(std::string const& name, i32 until) -> Item*
{
  assert(!mScopes.empty() && until >= 0);
  i32 currentScope = mScopes.size() - 1;
  for (auto curr = currentScope; curr >= until; curr--) {
    auto iter = mScopes[curr].items.find(name);
    if (iter != mScopes[curr].items.end()) {
      return iter->second;
    }
  }
  return nullptr;
}