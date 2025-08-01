/**
 * @file type.hpp
 * @brief Defines the type system for the toy compiler, including basic and composite types.
 *
 * This header provides the core type representations used in the compiler's type system.
 * It includes definitions for primitive types (i32, bool, unit), composite types (array, tuple),
 * as well as special types (unknown, any). Each type is represented as a class derived from the
 * abstract base class `Type`, which provides common interfaces for type information and operations.
 *
 * @namespace type
 *   Contains all type-related definitions and utilities.
 *
 * @enum TypeKind
 *   Enumerates the kinds of types supported, including basic, composite, and special types.
 *
 * @enum RefKind
 *   Enumerates the kinds of references (normal, mutable, immutable) for type qualifiers.
 *
 * @struct Type
 *   Abstract base class for all types. Provides interfaces for type kind, reference kind,
 *   memory size, iterability, string representation, and element access for composite types.
 *
 * @struct AnyType
 *   Represents a type that matches any other type (used for generic or placeholder purposes).
 *
 * @struct UnknownType
 *   Represents an unknown type, typically used during type inference or error recovery.
 *
 * @struct UnitType
 *   Represents the unit type, analogous to void or () in some languages.
 *
 * @struct IntType
 *   Represents the 32-bit integer type.
 *
 * @struct BoolType
 *   Represents the boolean type.
 *
 * @struct ArrayType
 *   Represents an array type with a fixed size and element type.
 *
 * @struct TupleType
 *   Represents a tuple type, which is a fixed-size collection of heterogeneous types.
 *
 * @typedef TypePtr
 *   Alias for std::shared_ptr<Type>, used for managing type instances.
 *
 * @note
 *   - All type instances are managed via shared pointers for polymorphic behavior.
 *   - Composite types (ArrayType, TupleType) provide element access and size information.
 *   - The type system supports reference qualifiers for future extensibility.
 */
#pragma once

#include <memory>
#include <ranges>
#include <vector>
#include <algorithm>

#include "panic.hpp"

namespace type {

// 类型种类
enum class TypeKind : std::uint8_t {
  I32, BOOL, UNIT, // 基础类型
  ARRAY, TUPLE,    // 复合类型
  UNKNOWN,
  ANY // 通用类型，用于匹配任意类型
};

// 引用种类
enum class RefKind : std::uint8_t {
  NORMAL,    // 普通变量
  MUTABLE,   // 可变引用
  IMMUTABLE  // 不可变引用
};

struct Type;
using  TypePtr = std::shared_ptr<Type>;

struct Type {
  TypeKind kind;
  RefKind  ref = RefKind::NORMAL;
  int      memory;
  bool     iterable; // 是否可迭代

  Type(TypeKind kind) : kind(kind) {}
  virtual ~Type() = default;

  [[nodiscard]] virtual std::string str() const = 0;
  virtual TypePtr getElemType(int idx = 0)  {
    UNREACHABLE("Shouldn't call this function!");
  }
  virtual int size() {
    UNREACHABLE("Shouldn't call this function!");
  }
};

struct AnyType : Type {
  AnyType() : Type(TypeKind::ANY) {
    memory = 0;
    iterable = false;
  }
  ~AnyType() override = default;

  [[nodiscard]] std::string str() const override { return "any"; }
};
using AnyTypePtr = std::shared_ptr<AnyType>;

struct UnknownType : Type {
  UnknownType() : Type(TypeKind::UNKNOWN) {
    memory = 0;
    iterable = false;
  }
  ~UnknownType() override = default;

  [[nodiscard]] std::string str() const override { return "unknown"; }
};
using UnknownTypePtr = std::shared_ptr<UnknownType>;

struct UnitType : Type {
  UnitType() : Type(TypeKind::UNIT) {
    memory = 0;
    iterable = false;
  }
  ~UnitType() override = default;

  [[nodiscard]] std::string str() const override { return "()"; }
};
using UnitTypePtr = std::shared_ptr<UnitType>;

// i32
struct IntType : Type {
  IntType() : Type(TypeKind::I32) {
    memory = 4;
    iterable = false;
  }
  ~IntType() override = default;

  [[nodiscard]] std::string str() const override { return "i32"; }
};
using IntTypePtr = std::shared_ptr<IntType>;

struct BoolType : Type {
  BoolType() : Type(TypeKind::BOOL) {
    memory = 1;
    iterable = false;
  }
  ~BoolType() override = default;

  [[nodiscard]] std::string str() const override { return "bool"; }
};
using BoolTypePtr = std::shared_ptr<BoolType>;

struct ArrayType : Type {
  int     m_size;
  TypePtr etype;

  ArrayType(int size, TypePtr type) : Type(TypeKind::ARRAY),
    m_size(size), etype(std::move(type))
  {
    memory = m_size * (etype->memory);
    iterable = true;
  }
  ~ArrayType() override = default;

  [[nodiscard]] std::string str() const override {
    return std::format("[{}; {}]", etype->str(), m_size);
  }
  TypePtr getElemType(int idx = 0) override { return etype; }
  int size() override { return m_size; }
};
using ArrayTypePtr = std::shared_ptr<ArrayType>;

struct TupleType : Type {
  int                  m_size;
  std::vector<TypePtr> etypes;

  TupleType(std::vector<TypePtr> types) : Type(TypeKind::TUPLE),
    m_size(types.size()), etypes(std::move(types))
  {
    memory = std::ranges::fold_left_first(
      etypes | std::views::transform([](const auto &type) {
        return type->memory;
      }),          // lazy evaluated 生成 Range
      std::plus{}  // 折叠操作函数，std::plus{} 是一个标准函数对象！
    ).value_or(0); // fold_left_first 返回的是一个 optional, 因为 etypes 可能为空
                   // 如果使用的是 fold_left 并指定了 accumulator 初始累积值，则不需要 optional
    iterable = false;
  }
  ~TupleType() override = default;

  [[nodiscard]] std::string str() const override {
    ASSERT_MSG(etypes.size() > 0, "Tuple elements <= 0!");

    auto inner = etypes
      | std::views::transform([](const auto &type) {
          return type->str();
        })
      | std::views::join_with(std::string{", "}) // join_with 是在两个元素之间插入分隔符！
      | std::ranges::to<std::string>();

    return "(" + inner + (etypes.size() == 1 ? "," : "") + ")";
  }
  TypePtr getElemType(int idx = 0) override {
    ASSERT_MSG(idx >= 0 && idx < m_size, "out of bounds access!");
    return etypes[idx];
  }
  int size() override { return m_size; }
};
using TupleTypePtr = std::shared_ptr<TupleType>;

} // namespace type
