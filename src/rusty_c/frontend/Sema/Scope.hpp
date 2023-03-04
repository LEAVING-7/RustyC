#pragma once

#include "../Types.hpp"
#include "frontend/Syntax.hpp"
#include <unordered_map>


class Scopes {
  std::vector<std::unordered_map<std::string, TypeBase*>> mScopes;

public:
  Scopes() = default;
  auto insert(std::string const& name, TypeBase *type) -> bool;
  auto lookup(std::string const& name) -> TypeBase*;
  auto current() -> std::unordered_map<std::string, TypeBase*>& { return mScopes.back(); }
  void enterScope();
  void leaveScope();
};
