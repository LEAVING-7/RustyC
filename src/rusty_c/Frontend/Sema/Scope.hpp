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

class ScopeRecord {
public:
  enum class RecordType : u8 {
    Pop,
    Push,
  };

private:
  std::vector<Scope> mScopes;
  std::vector<bool> mValidBit{false};
  std::vector<RecordType> mRecords;
  i32 mCurrentScope = -1;
  i32 mCurrentRecord = 0;

public:
  ScopeRecord() = default;
  ScopeRecord(ScopeRecord&&) = default;
  ~ScopeRecord() = default;

  auto lookupIdentifier(std::string const& name) const -> TypeBase*
  {
    for (i32 i = mCurrentScope; i >= 0; --i) {
      if (mValidBit[i] == false) {
        continue;
      }
      auto it = mScopes[i].identifiers.find(name);
      if (it != mScopes[i].identifiers.end()) {
        return it->second.get();
      }
    }
    return nullptr;
  }
  auto lookupItem(std::string const& name) const -> Item*
  {
    for (i32 i = mCurrentScope; i >= 0; --i) {
      if (mValidBit[i] == false) {
        continue;
      }
      auto it = mScopes[i].items.find(name);
      if (it != mScopes[i].items.end()) {
        return it->second;
      }
    }
    return nullptr;
  }

  auto enterScope() -> void
  {
    assert(mRecords[mCurrentRecord] == RecordType::Push);

    do {
      ++mCurrentScope;
    } while (mValidBit[mCurrentScope] == false);

    ++mCurrentRecord;
  }

  auto leaveScope() -> void
  {
    assert(mRecords[mCurrentRecord] == RecordType::Pop);

    mValidBit[mCurrentScope] = false;

    while (mValidBit[mCurrentScope] == false) {
      --mCurrentScope;
      if (mCurrentScope == -1) {
        break;
      }
    }

    ++mCurrentRecord;
  }

  auto currentScope() -> Scope&
  {
    assert(mCurrentScope >= 0);
    return mScopes[mCurrentScope];
  }

  auto pushScope() -> void
  {
    mRecords.push_back(RecordType::Push);
    mValidBit.push_back(true);
  }
  auto popScope(Scope&& scope) -> void
  {
    mScopes.push_back(std::move(scope));
    mRecords.push_back(RecordType::Pop);
  }
};

class Scopes {
public:
  std::vector<Scope> mScopes;
  ScopeRecord mRecord;

  Scopes() = default;
  ~Scopes() = default;
  auto insertIdentifier(std::string const& name, std::unique_ptr<TypeBase> type) -> bool;
  auto lookupIdentifier(std::string const& name) -> TypeBase*;
  auto lookupIdInCurr(std::string const& name) -> TypeBase*;
  auto lookupIdUntil(std::string const& name, i32 until) -> TypeBase*;

  auto insertItem(std::string const& name, Item* type) -> bool;
  auto lookupItem(std::string const& name) -> Item*;
  auto lookupItemInCurr(std::string const& name) -> Item*;
  auto lookupItemUntil(std::string const& name, i32 until) -> Item*;

  auto size() { return mScopes.size(); }

  auto current() -> Scope& { return mScopes.back(); }
  void enterScope()
  {
    mScopes.push_back({});
    mRecord.pushScope();
  }
  void leaveScope()
  {
    assert(!mScopes.empty());
    mRecord.popScope(std::move(mScopes.back()));
    mScopes.pop_back();
  }
  auto moveRecord() -> ScopeRecord&& { return std::move(mRecord); }
};

template <typename T>
  requires requires(T t) {
             {
               t.enterScope()
               } -> std::same_as<void>;
             {
               t.leaveScope()
               } -> std::same_as<void>;
           }
class ScopeGuard {
  T& mScopes;

public:
  ScopeGuard(T& scopes) : mScopes(scopes) { mScopes.enterScope(); }
  ~ScopeGuard() { mScopes.leaveScope(); }
};