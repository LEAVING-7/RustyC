#include "Types.hpp"

bool operator==(TypeBase const& lhs, TypeBase const& rhs)
{
  if (&lhs == &rhs) {
    return true;
  }
  if (lhs.mKind != rhs.mKind) {
    return false;
  }
  switch (lhs.mKind) {
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
  case TypeBase::Kind::Textual:
  case TypeBase::Kind::Tuple:
  case TypeBase::Kind::Array:
  case TypeBase::Kind::Slice:
  case TypeBase::Kind::Struct:
  case TypeBase::Kind::Enum:
  case TypeBase::Kind::Union:
  case TypeBase::Kind::Functions:
  case TypeBase::Kind::Closures:
  case TypeBase::Kind::Reference:
  case TypeBase::Kind::RawPointer:
  case TypeBase::Kind::FunctionPointer:
  case TypeBase::Kind::TraitObjects:
  case TypeBase::Kind::ImplTrait:
  case TypeBase::Kind::SIZE:
    utils::Unreachable(utils::SrcLoc::current());
    break;
  }
}