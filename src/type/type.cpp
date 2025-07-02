#include "type.hpp"

namespace type {

bool
typeEqual(const TypePtr &a, const TypePtr &b) {
  if (!a || !b) {
    return false;
  }
  if (a->kind != b->kind || a->ref != b->ref) {
    return false;
  }

  switch (a->kind) {
    case TypeKind::VOID:
    case TypeKind::I32:
      return true;

    case TypeKind::ARRAY: {
      auto aa = std::static_pointer_cast<ArrayType>(a);
      auto bb = std::static_pointer_cast<ArrayType>(b);
      return aa->size == bb->size && typeEqual(aa->etype, bb->etype);
    }

    case TypeKind::TUPLE: {
      auto ta = std::static_pointer_cast<TupleType>(a);
      auto tb = std::static_pointer_cast<TupleType>(b);
      if (ta->etypes.size() != tb->etypes.size()) {
        return false;
      }
      for (size_t i = 0; i < ta->etypes.size(); ++i) {
        if (!typeEqual(ta->etypes[i], tb->etypes[i])) {
          return false;
        }
      }
      return true;
    }

    case TypeKind::UNKNOWN:
    default:
      return false;
  }
}

} // namespace type
