#pragma once
#include "utils/utils.hpp"

#define DEFINE_TYPES(...)                                                                                              \
  enum class Type { __VA_ARGS__ };                                                                                     \
  Type const mType;

#define DEFINE_KINDS(...)                                                                                              \
  enum class Kind { __VA_ARGS__, SIZE };                                                                 \
  Kind const mKind;

#define IMPL_AS(Target)                                                                                                \
  template <typename T>                                                                                                \
    requires std::derived_from<T, Target> decltype(auto)                                                               \
  as()                                                                                                                 \
  {                                                                                                                    \
    return static_cast<T*>(this);                                                                                      \
  }                                                                                                                    \
  template <typename T>                                                                                                \
    requires std::derived_from<T, Target> decltype(auto)                                                               \
  as() const                                                                                                           \
  {                                                                                                                    \
    return static_cast<T const*>(this);                                                                                \
  }                                                                                                                    \
  Target() = delete;

struct TypeBase {
public:
  DEFINE_KINDS(Boolean,
               // Numeric
               I8, I16, I32, I64, U8, U16, U32, U64, F32, F64, Char, Str,
               //
               Never, Tuple, Array, Slice, Struct, Enum, Union, Functions, Closures, Reference, RawPointer,
               FunctionPointer, TraitObjects, ImplTrait,
               //
               Unknown)
  IMPL_AS(TypeBase);

public:
  TypeBase(TypeBase::Kind kind) : mKind(kind) {}
  virtual ~TypeBase() = default;
};

struct Unknown final : TypeBase {
public:
  Unknown() : TypeBase(TypeBase::Kind::Unknown) {}
  ~Unknown() override = default;
};

struct Boolean final : TypeBase {
public:
  Boolean() : TypeBase(TypeBase::Kind::Boolean){};
  ~Boolean() override = default;
};

#define DEFINE_NUMERIC_TYPE(Type)                                                                                      \
  struct Type final : TypeBase {                                                                                       \
  public:                                                                                                              \
    Type() : TypeBase(TypeBase::Kind::Type)                                                                            \
    {                                                                                                                  \
    }                                                                                                                  \
    ~Type() override = default;                                                                                        \
  };

DEFINE_NUMERIC_TYPE(I8)
DEFINE_NUMERIC_TYPE(I16)
DEFINE_NUMERIC_TYPE(I32)
DEFINE_NUMERIC_TYPE(I64)
DEFINE_NUMERIC_TYPE(U8)
DEFINE_NUMERIC_TYPE(U16)
DEFINE_NUMERIC_TYPE(U32)
DEFINE_NUMERIC_TYPE(U64)
DEFINE_NUMERIC_TYPE(F32)
DEFINE_NUMERIC_TYPE(F64)
#undef DEFINE_NUMERIC_TYPE


struct Never final : TypeBase {
public:
  Never() : TypeBase(TypeBase::Kind::Never) {}
  ~Never() override = default;
};

struct Char final : TypeBase {
public:
  Char() : TypeBase(TypeBase::Kind::Char) {}
  ~Char() override = default;
};

struct Str final : TypeBase {
public:
  Str() : TypeBase(TypeBase::Kind::Str) {}
  ~Str() override = default;
};

struct TupleType final : TypeBase {
public:
  std::vector<std::unique_ptr<TypeBase>> mTypes;

public:
  TupleType() : TypeBase(TypeBase::Kind::Tuple), mTypes{} {} // unit
  TupleType(std::vector<std::unique_ptr<TypeBase>> types) : TypeBase(TypeBase::Kind::Tuple), mTypes(std::move(types)) {}
  ~TupleType() override = default;

  bool isUnit() const { return mTypes.empty(); }
};

struct FunctionType final : TypeBase {
public:
  std::vector<std::unique_ptr<TypeBase>> mParams;
  std::unique_ptr<TypeBase> mRet;

public:
  FunctionType(std::vector<std::unique_ptr<TypeBase>> params)
      : TypeBase(TypeBase::Kind::Functions), mParams(std::move(params)), mRet(std::make_unique<TupleType>())
  {
  }
  FunctionType(std::vector<std::unique_ptr<TypeBase>> params, std::unique_ptr<TypeBase> ret)
      : TypeBase(TypeBase::Kind::Functions), mParams(std::move(params)), mRet(std::move(ret))
  {
  }
  ~FunctionType() override = default;
};

auto TypeEquals(TypeBase const* lhs, TypeBase const* rhs) -> bool;

auto TypeClone(TypeBase const* type) -> std::unique_ptr<TypeBase>;
inline auto TypeClone(std::unique_ptr<TypeBase> const& type) -> std::unique_ptr<TypeBase>
{
  return TypeClone(type.get());
}

inline Never TypeNever;
inline TupleType TypeUnit{};

auto TypeToString(TypeBase const* type) -> std::string;

auto GetNumBoolMap(std::string_view target) -> std::unique_ptr<TypeBase>;

#undef DEFINE_TYPES
#undef DEFINE_KINDS
#undef IMPL_AS