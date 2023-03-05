#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Scope {
public:
  std::vector<std::unique_ptr<int>> mTypes;
  std::unordered_map<std::string, int*> identifiers;
  Scope() = default;
  Scope(Scope&&) = default;
  ~Scope() = default;
};

struct Scopes {
  std::vector<Scope> mScopes;
};
int main(int argc, char* argv[]) {
  Scopes s;
  s.mScopes.push_back({});
}