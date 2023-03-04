#pragma once
#include "utils/utils.hpp"

#define DEFINE_TYPES(...)                                                                                              \
  enum class Type { __VA_ARGS__ };                                                                                     \
  Type const mType;

#define DEFINE_KINDS(...)                                                                                              \
  enum class Kind { __VA_ARGS__ __VA_OPT__(, ) SIZE };                                                                 \
  Kind const mKind;

struct TypeBase {
public:
  DEFINE_KINDS(Boolean,
               // Numeric
               I8, I16, I32, I64, U8, U16, U32, U64, F32, F64,
               //
               Textual, Never, Tuple, Array, Slice, Struct, Enum, Union, Functions, Closures, Reference, RawPointer,
               FunctionPointer, TraitObjects, ImplTrait)
public:
  TypeBase(TypeBase::Kind kind) : mKind(kind) {}
  virtual ~TypeBase() = default;
};

struct Boolean final : TypeBase {
public:
  Boolean() : TypeBase(TypeBase::Kind::Boolean){};
  ~Boolean() override = default;
};

struct Numeric final : TypeBase {
public:
  Numeric(TypeBase::Kind kind) : TypeBase(kind){};
  ~Numeric() override = default;
};

struct Never final : TypeBase {
public:
  Never() : TypeBase(TypeBase::Kind::Never) {}
  ~Never() override = default;
};

bool operator==(TypeBase const& lhs, TypeBase const& rhs);

static Boolean TypeBoolean;
static Never TypeNever;

static Numeric TypeI8{TypeBase::Kind::I8};
static Numeric TypeI16{TypeBase::Kind::I16};
static Numeric TypeI32{TypeBase::Kind::I32};
static Numeric TypeI64{TypeBase::Kind::I64};
static Numeric TypeU8{TypeBase::Kind::U8};
static Numeric TypeU16{TypeBase::Kind::U16};
static Numeric TypeU32{TypeBase::Kind::U32};
static Numeric TypeU64{TypeBase::Kind::U64};

static Numeric TypeF32{TypeBase::Kind::F32};
static Numeric TypeF64{TypeBase::Kind::F64};

#undef DEFINE_TYPES
#undef DEFINE_KINDS