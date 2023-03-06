#pragma once

#include "../Types.hpp"
#include "Frontend/Syntax.hpp"
#include <llvm/ADT/StringMap.h>
#include <unordered_map>

struct Scope {
public:
  std::unordered_map<std::string, std::unique_ptr<TypeBase>> identifiers;
  std::unordered_map<std::string, Item*> items;
  Scope() = default;
  Scope(Scope&&) = default;
  ~Scope() = default;
};
class Scopes {

public:
  std::vector<Scope> mScopes;
  Scopes() = default;
  ~Scopes() = default;
  auto insertIdentifier(std::string const& name, std::unique_ptr<TypeBase> type) -> bool;
  auto lookupIdentifier(std::string const& name) -> TypeBase*;
  auto insertItem(std::string const& name, Item* type) -> bool;
  auto lookupItem(std::string const& name) -> Item*;
  auto current() -> Scope& { return mScopes.back(); }
  void enterScope();
  void leaveScope();
};
