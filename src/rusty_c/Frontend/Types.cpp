#include "Types.hpp"
#include <unordered_map>

auto TypeEquals(TypeBase const* lhs, TypeBase const* rhs) -> bool
{
  if (lhs == nullptr || rhs == nullptr) {
    return false;
  }
  if (lhs == rhs) {
    return true;
  }
  if (lhs->mKind != rhs->mKind) {
    return false;
  }
  switch (lhs->mKind) {
  case TypeBase::Kind::Boolean:
  case TypeBase::Kind::I8:
  case TypeBase::Kind::I16:
  case TypeBase::Kind::I32:
  case TypeBase::Kind::I64:
  case TypeBase::Kind::U8:
  case TypeBase::Kind::U16:
  case TypeBase::Kind::U32:
  case TypeBase::Kind::U64:
  case TypeBase::Kind::F32:
  case TypeBase::Kind::F64:
  case TypeBase::Kind::Never:
    return true;
  case TypeBase::Kind::Functions: {
    auto lhsFn = lhs->as<FunctionType>();
    auto rhsFn = rhs->as<FunctionType>();

    if (lhsFn->mParams.size() != rhsFn->mParams.size()) {
      return false;
    }

    for (auto i = 0; i < lhsFn->mParams.size(); ++i) {
      if (!TypeEquals(lhsFn->mParams[i].get(), rhsFn->mParams[i].get())) {
        return false;
      }
    }
    return true;
  } break;
  case TypeBase::Kind::Tuple: {
    auto lhsTuple = lhs->as<TupleType>();
    auto rhsTuple = rhs->as<TupleType>();

    if (lhsTuple->mTypes.size() != rhsTuple->mTypes.size()) {
      return false;
    }
    for (auto i = 0; i < lhsTuple->mTypes.size(); ++i) {
      if (!TypeEquals(lhsTuple->mTypes[i].get(), rhsTuple->mTypes[i].get())) {
        return false;
      }
    }
    return true;
  } break;
  case TypeBase::Kind::Array:
  case TypeBase::Kind::Slice:
  case TypeBase::Kind::Struct:
  case TypeBase::Kind::Enum:
  case TypeBase::Kind::Union:
  case TypeBase::Kind::Closures:
  case TypeBase::Kind::Reference:
  case TypeBase::Kind::RawPointer:
  case TypeBase::Kind::FunctionPointer:
  case TypeBase::Kind::TraitObjects:
  case TypeBase::Kind::ImplTrait:
  case TypeBase::Kind::SIZE:
  case TypeBase::Kind::Char:
  case TypeBase::Kind::Str:
  case TypeBase::Kind::Unknown:
    utils::Unreachable(utils::SrcLoc::current());
    break;
  }
}

auto TypeClone(TypeBase const* type) -> std::unique_ptr<TypeBase>
{
  if (type == nullptr) {
    return {nullptr};
  }
  std::unique_ptr<TypeBase> ret;
  switch (type->mKind) {
  case TypeBase::Kind::Boolean:
    return std::make_unique<Boolean>();

#define CASE_NUMERIC_TYPE(Type)                                                                                        \
  case TypeBase::Kind::Type:                                                                                           \
    return std::make_unique<Type>();
    CASE_NUMERIC_TYPE(I8)
    CASE_NUMERIC_TYPE(I16)
    CASE_NUMERIC_TYPE(I32)
    CASE_NUMERIC_TYPE(I64)
    CASE_NUMERIC_TYPE(U8)
    CASE_NUMERIC_TYPE(U16)
    CASE_NUMERIC_TYPE(U32)
    CASE_NUMERIC_TYPE(U64)
    CASE_NUMERIC_TYPE(F32)
    CASE_NUMERIC_TYPE(F64)
#undef CASE_NUMERIC_TYPE
  case TypeBase::Kind::Str:
    return std::make_unique<Str>();
  case TypeBase::Kind::Char:
    return std::make_unique<Char>();
  case TypeBase::Kind::Never:
    return std::make_unique<Never>();
  case TypeBase::Kind::Functions: {
    auto fn = type->as<FunctionType>();
    std::vector<std::unique_ptr<TypeBase>> paramTypes;
    std::unique_ptr<TypeBase> retType;
    for (i32 i = 0; i < fn->mParams.size(); ++i) {
      paramTypes.push_back(TypeClone(fn->mParams[i].get()));
    }
    retType = TypeClone(fn->mRet.get());
    return std::make_unique<FunctionType>(std::move(paramTypes), std::move(retType));
  }
  case TypeBase::Kind::Tuple:
  case TypeBase::Kind::Array:
  case TypeBase::Kind::Slice:
  case TypeBase::Kind::Struct:
  case TypeBase::Kind::Enum:
  case TypeBase::Kind::Union:
  case TypeBase::Kind::Closures:
  case TypeBase::Kind::Reference:
  case TypeBase::Kind::RawPointer:
  case TypeBase::Kind::FunctionPointer:
  case TypeBase::Kind::TraitObjects:
  case TypeBase::Kind::ImplTrait:
  case TypeBase::Kind::SIZE:
    break;
  case TypeBase::Kind::Unknown:
    utils::Unreachable(utils::SrcLoc::current());
    break;
  }

  utils::Unimplemented(utils::SrcLoc::current());
}

static const std::unordered_map<std::string_view, TypeBase> gTypeMap{
    {"bool", Boolean{}}, {"i8", I8{}},   {"i16", I16{}}, {"i32", I32{}}, {"i64", I64{}}, {"u8", U8{}},
    {"u16", U16{}},      {"u32", U32{}}, {"u64", U64{}}, {"f32", F32{}}, {"f64", F64{}},
};

auto GetNumBoolMap(std::string_view target) -> std::unique_ptr<TypeBase>
{
  auto const it = gTypeMap.find(target);
  if (it != gTypeMap.end()) {
    return std::make_unique<TypeBase>(it->second);
  }
  return {nullptr};
}

auto TypeToString(std::string& str, TypeBase const* type) -> void
{
  switch (type->mKind) {
  case TypeBase::Kind::Boolean:
    str += "bool";
    break;
  case TypeBase::Kind::I8:
    str += "i8";
    break;
  case TypeBase::Kind::I16:
    str += "i16";
    break;
  case TypeBase::Kind::I32:
    str += "i32";
    break;
  case TypeBase::Kind::I64:
    str += "i64";
    break;
  case TypeBase::Kind::U8:
    str += "u8";
    break;
  case TypeBase::Kind::U16:
    str += "u16";
    break;
  case TypeBase::Kind::U32:
    str += "u32";
    break;
  case TypeBase::Kind::U64:
    str += "u64";
    break;
  case TypeBase::Kind::F32:
    str += "f32";
    break;
  case TypeBase::Kind::F64:
    str += "f64";
    break;
  case TypeBase::Kind::Never:
    str += "!";
    break;
  case TypeBase::Kind::Tuple: {
    auto tuple = type->as<TupleType>();
    str += "(";
    for (auto& elem : tuple->mTypes) {
      TypeToString(str, elem.get());
      str += ",";
    }
    str += ")";
  } break;
  case TypeBase::Kind::Functions: {
    auto func = type->as<FunctionType>();
    str += "fn(";
    for (i32 i = 0; i < func->mParams.size(); ++i) {
      TypeToString(str, func->mParams[i].get());
      if (i != func->mParams.size() - 1) {
        str += ",";
      }
    }
    str += ")";
    if (func->mRet) {
      str += " -> ";
      TypeToString(str, func->mRet.get());
    }
  } break;
  default:
    utils::Unimplemented(utils::SrcLoc::current(), "TypeToString");
  }
}

auto TypeToString(TypeBase const* type) -> std::string
{
  std::string str{};
  TypeToString(str, type);
  return str;
}
