#include "TokenKind.hpp"

static const KwMapT gKwMap{
#define KEYWORD(x, y) {#x, TokenKind::Kw##x},
#include "TokenKind.def"
};

static char const* gTokenNames[]{
#define TOK(x) #x,
#define KEYWORD(x, y) y,
#define PUNCT(x, y) y,
#include "TokenKind.def"
};

auto GetKwMap() -> KwMapT const& { return gKwMap; }
auto TokenKindToString(TokenKind type) -> char const*
{
  if (type < TokenKind::TokenSize) {
    return gTokenNames[static_cast<std::underlying_type_t<TokenKind>>(type)];
  }
  return nullptr;
};