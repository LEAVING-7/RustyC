#include "Scope.hpp"

auto Scopes::insertIdentifier(std::string const& name, std::unique_ptr<TypeBase> type) -> bool
{
  return mScopes.back().identifiers.insert({name, std::move(type)}).second;
}
auto Scopes::lookupIdentifier(std::string const& name) -> TypeBase*
{
  assert(!mScopes.empty());
  i32 currentScope = mScopes.size() - 1;
  for (auto curr = currentScope; curr >= 0; curr--) {
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

auto Scopes::lookupItem(std::string const& name) -> Item*
{
  assert(!mScopes.empty());
  i32 currentScope = mScopes.size() - 1;
  for (auto curr = currentScope; curr >= 0; curr--) {
    auto iter = mScopes[curr].items.find(name);
    if (iter != mScopes[curr].items.end()) {
      return iter->second;
    }
  }
  return nullptr;
};

