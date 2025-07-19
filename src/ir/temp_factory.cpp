#include "type.hpp"
#include "symbol.hpp"
#include "position.hpp"
#include "temp_factory.hpp"

namespace ir {

void
TempFactory::reset()
{
  cnt = 0;
}

sym::TempPtr
TempFactory::produce(util::Position pos, type::TypePtr type)
{
  auto temp = std::make_shared<sym::Temp>();
  temp->name = std::format("%{}", cnt++);
  temp->pos = pos;
  temp->mut = false;
  temp->init = true;
  temp->type = std::move(type);

  return temp;
}

} // namespace ir
