/**
 * @file type_factory.hpp
 * @brief Defines the TypeFactory class and supporting structures for unique type management.
 *
 * This header provides mechanisms to ensure unique instances of types (such as arrays and tuples)
 * within the type system. It includes:
 * - The TypeKey struct for uniquely identifying types based on kind, size, and element types.
 * - Specialization of std::hash for TypeKey to allow its use in unordered containers.
 * - The TypeFactory class, which manages the creation and caching of unique type instances.
 * - Static type instances for common types (ANY, INT, BOOL, UNIT, UNKNOWN).
 * - Utility functions for type comparison and identification.
 *
 * All type instances created through TypeFactory are guaranteed to be unique, ensuring
 * pointer equality can be used for type comparison throughout the system.
 */
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

/**
 * @brief 工厂类，用于创建和管理类型系统中的各种类型实例，确保类型实例的唯一性。
 *
 * TypeFactory 提供了对基本类型（如 ANY_TYPE、INT_TYPE、BOOL_TYPE、UNIT_TYPE、UNKNOWN_TYPE）的唯一实例访问，
 * 并通过缓存机制保证数组类型（ArrayType）和元组类型（TupleType）的唯一性。
 *
 * - 所有类型的实例都通过 TypeFactory 创建和管理，保证类型系统中同一类型只有一个实例。
 * - 提供静态方法判断类型（如 isArray, isTuple）。
 * - 提供 getArray 和 getTuple 方法用于获取数组和元组类型的唯一实例。
 *
 * @note
 *  - TypePtr 为 std::shared_ptr<Type>。
 *  - TypeKey 用于唯一标识类型实例，需实现合适的哈希和比较操作。
 *  - cache 用于存储已创建的类型实例，避免重复创建。
 */
class TypeFactory {
public:
  // 唯一存在的类型的实例
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
