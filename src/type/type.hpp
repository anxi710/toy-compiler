#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

namespace type {

enum class TypeKind : std::uint8_t {
  VOID, // 返回值类型可以为空，即代表不返回任何值
  I32,
  ARRAY,
  TUPLE,
  UNKNOWN
};

enum class RefKind : std::uint8_t {
  NORMAL,    // 普通变量
  MUTABLE,   // 可变引用
  IMMUTABLE  // 不可变引用
};

struct Type {
  TypeKind kind = TypeKind::UNKNOWN;
  RefKind  ref  = RefKind::NORMAL;

  Type(TypeKind kind) : kind(kind) {}
  virtual ~Type() = default;
  virtual std::string str() const = 0;
};
using TypePtr = std::shared_ptr<Type>;

struct VoidType : Type {
  VoidType() : Type(TypeKind::VOID) {}
  ~VoidType() override = default;
  std::string str() const override { return "void"; }
};
using VoidTypePtr = std::shared_ptr<Type>;

struct IntType : Type {
  IntType() : Type(TypeKind::I32) {}
  ~IntType() override = default;
  std::string str() const override { return "i32"; }
};
using IntTypePtr = std::shared_ptr<IntType>;

struct ArrayType : Type {
  int     size;
  TypePtr etype;

  ArrayType() : Type(TypeKind::ARRAY) {}
  ArrayType(int size, TypePtr etype)
    : Type(TypeKind::ARRAY), size(size), etype(std::move(etype)) {}
  ~ArrayType() override = default;

  std::string str() const override {
    std::ostringstream oss;
    oss << "[" << etype->str() << ", " << size << "]";
    return oss.str();
  }
};
using ArrayTypePtr = std::shared_ptr<ArrayType>;

struct TupleType : Type {
  int                  size;
  std::vector<TypePtr> etypes;

  TupleType() : Type(TypeKind::TUPLE) {}
  TupleType(std::vector<TypePtr> etypes)
    : Type(TypeKind::TUPLE), size(etypes.size()),
      etypes(std::move(etypes)) {}
  ~TupleType() override = default;

  std::string str() const override {
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
};
using TupleTypePtr = std::shared_ptr<TupleType>;

bool typeEqual(const TypePtr &a, const TypePtr &b);

} // namespace type
