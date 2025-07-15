#pragma once

#include <memory>
#include <vector>
#include <sstream>

#include "panic.hpp"

namespace type {

// 类型种类
enum class TypeKind : std::uint8_t {
  I32,
  BOOL,
  UNIT,
  ARRAY,
  TUPLE,
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
  virtual TypePtr getElemType(int idx = 0) = 0;
  virtual int size() = 0;
};

struct AnyType : Type {
  AnyType() : Type(TypeKind::ANY) {
    memory = 0;
    iterable = false;
  }
  ~AnyType() override = default;

  [[nodiscard]] std::string str() const override { return "any"; }
  TypePtr getElemType(int idx = 0) override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
  int size() override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
};
using AnyTypePtr = std::shared_ptr<AnyType>;

struct UnknownType : Type {
  UnknownType() : Type(TypeKind::UNKNOWN) {
    memory = 0;
    iterable = false;
  }
  ~UnknownType() override = default;

  [[nodiscard]] std::string str() const override { return "unknown"; }
  TypePtr getElemType(int idx = 0) override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
  int size() override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
};
using UnknownTypePtr = std::shared_ptr<UnknownType>;

struct UnitType : Type {
  UnitType() : Type(TypeKind::UNIT) {
    memory = 0;
    iterable = false;
  }
  ~UnitType() override = default;

  [[nodiscard]] std::string str() const override { return "()"; }
  TypePtr getElemType(int idx = 0) override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
  int size() override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
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
  TypePtr getElemType(int idx = 0) override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
  int size() override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
};
using IntTypePtr = std::shared_ptr<IntType>;

struct BoolType : Type {
  BoolType() : Type(TypeKind::BOOL) {
    memory = 1;
    iterable = false;
  }
  ~BoolType() override = default;

  [[nodiscard]] std::string str() const override { return "bool"; }
  TypePtr getElemType(int idx = 0) override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
  int size() override {
    UNREACHABLE("Shouldn't call this funtion!");
  }
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
  TypePtr getElemType(int idx = 0) override {
    return etype;
  }
  int size() override {
    return m_size;
  }
};
using ArrayTypePtr = std::shared_ptr<ArrayType>;

struct TupleType : Type {
  int                  m_size;
  std::vector<TypePtr> etypes;

  TupleType(std::vector<TypePtr> types)
    : Type(TypeKind::TUPLE), m_size(types.size()),
      etypes(std::move(types))
  {
    memory = 0;
    for (const auto &etype : etypes) {
      memory += etype->memory;
    }
    iterable = false;
  }
  ~TupleType() override = default;

  [[nodiscard]] std::string str() const override {
    std::ostringstream oss;
    oss << "(";
    for (auto iter = etypes.cbegin(); iter != etypes.end(); ++iter) {
      oss << (*iter)->str();
      if (std::next(iter) != etypes.end()) {
        oss << ", ";
      }
    }
    oss << ")";
    return oss.str();
  }
  TypePtr getElemType(int idx = 0) override {
    ASSERT_MSG(
      idx >= 0 && idx < m_size,
      "out of bounds access!"
    );
    return etypes[idx];
  }
  int size() override {
    return m_size;
  }
};
using TupleTypePtr = std::shared_ptr<TupleType>;

} // namespace type
