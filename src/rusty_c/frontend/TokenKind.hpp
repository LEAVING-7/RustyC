#pragma once
#include "common.hpp"

#include <unordered_map>

enum TokenKind : u16 {
#define PUNCT(x, y) Pun##x,
#define TOK(x) x,
#define KEYWORD(x, y) Kw##x,
#include "TokenKind.def"
  TokenSize
};

using KwMapT = std::unordered_map<std::string_view, TokenKind>;
auto GetKwMap() -> KwMapT const&;
auto TokenKindToString(TokenKind type) -> char const*;
