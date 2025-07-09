#include "type_factory.hpp"

namespace type {

const TypePtr TypeFactory::INT_TYPE  = std::make_shared<IntType>();
const TypePtr TypeFactory::BOOL_TYPE = std::make_shared<BoolType>();
const TypePtr TypeFactory::UNIT_TYPE = std::make_shared<UnitType>();
const TypePtr TypeFactory::UNKNOWN_TYPE = std::make_shared<UnknownType>();

} // namespace type
