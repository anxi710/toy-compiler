#include "type_factory.hpp"

namespace type {

const TypePtr TypeFactory::ANY_TYPE  = std::make_shared<AnyType>();
const TypePtr TypeFactory::INT_TYPE  = std::make_shared<IntType>();
const TypePtr TypeFactory::BOOL_TYPE = std::make_shared<BoolType>();
const TypePtr TypeFactory::UNIT_TYPE = std::make_shared<UnitType>();
const TypePtr TypeFactory::UNKNOWN_TYPE = std::make_shared<UnknownType>();

/**
 * @brief  判断两类型是否相同
 * @param  lhs 左值
 * @param  rhs 右值
 * @return 相同为 true，不同为 false
 */
bool
typeEquals(const TypePtr &lhs, const TypePtr &rhs)
{
  if (lhs == TypeFactory::ANY_TYPE || rhs == TypeFactory::ANY_TYPE) {
    return true;
  }
  return lhs == rhs;
}

} // namespace type
