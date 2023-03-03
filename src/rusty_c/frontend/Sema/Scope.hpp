#pragma once

#include "frontend/Syntax.hpp"
#include <unordered_map>


class Scopes {
  std::vector<std::unordered_map<std::string, LetStmt*>> mScopes;
public:
  Scopes() = default;
  auto insert(LetStmt* stmt) -> bool;
  auto lookup(std::string const& name) -> LetStmt*;
  void enterScope();
  void leaveScope();
};
