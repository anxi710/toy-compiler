#pragma once

#include <vector>
#include <unordered_map>

#include "type.hpp"

namespace type {

// 唯一性区分不同类型
struct TypeKey {
  TypeKind kind; // 只使用 ARRAY 和 TUPLE
  int size;
  std::vector<TypePtr> etypes;

  TypeKey(TypeKind kind, int size, std::vector<TypePtr> etypes)
    : kind(kind), size(size), etypes(std::move(etypes)) {}

  bool operator== (const TypeKey &other) const {
    if (kind != other.kind || size != other.size ||
        etypes.size() != other.etypes.size()) {
      return false;
    }
    // 依次检查各元素的类型
    for (std::size_t i = 0; i < etypes.size(); ++i) {
      if (etypes[i] != other.etypes[i]) {
        return false;
      }
    }
    return true;
  }
};

} // namespace type

namespace std {

// 计算 TypeKey 的 hash 值
template<>
struct hash<type::TypeKey> {
  std::size_t operator() (const type::TypeKey &k) const {
    std::size_t h = std::hash<int>()(static_cast<int>(k.kind));
    for (const auto &p : k.etypes) {
      h ^= (std::hash<type::Type*>()(p.get())
        + 0x9e3779b9 + (h << 6) + (h >> 2));
    }
    return h;
  }
};

} // namespace std

namespace type {

class TypeFactory {
public:
  TypeFactory() = default;

public:
  // 唯一存在的类型的实例
  // 所有使用到的位置都指向这些实例，保证类型系统唯一性
  static const TypePtr ANY_TYPE;
  static const TypePtr INT_TYPE;
  static const TypePtr BOOL_TYPE;
  static const TypePtr UNIT_TYPE;
  static const TypePtr UNKNOWN_TYPE;

public:
  static bool isArray(const TypePtr &type) {
    return type->kind == TypeKind::ARRAY;
  }

  TypePtr getArray(int size, TypePtr etype) {
    // Array 的所有元素类型一致，因此只存储一份！
    TypeKey key{TypeKind::ARRAY, size, {etype}};

    if (auto iter = cache.find(key); iter != cache.end()) {
      return iter->second;
    }

    auto array = std::make_shared<ArrayType>(size, etype);
    cache[key] = array;
    return array;
  }

  static bool isTuple(const TypePtr &type) {
    return type->kind == TypeKind::TUPLE;
  }

  TypePtr getTuple(std::vector<TypePtr> etypes) {
    TypeKey key{TypeKind::TUPLE, static_cast<int>(etypes.size()), etypes};

    if (auto iter = cache.find(key); iter != cache.end()) {
      return iter->second;
    }

    auto tuple = std::make_shared<TupleType>(etypes);
    cache[key] = tuple;
    return tuple;
  }

private:
  std::unordered_map<TypeKey, TypePtr> cache;
};

bool typeEquals(const TypePtr &lhs, const TypePtr &rhs);

} // namespace type
