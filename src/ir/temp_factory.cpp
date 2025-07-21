#include "type.hpp"
#include "symbol.hpp"
#include "position.hpp"
#include "temp_factory.hpp"

namespace ir {

/**
 * @brief 生成一个临时变量
 *
 * @param pos  临时变量声明时的位置
 * @param type 临时变量类型
 * @return sym::TempPtr 生成的临时变量指针
 */
sym::TempPtr
TempFactory::produce(util::Position pos, type::TypePtr type)
{
  auto temp = std::make_shared<sym::Temp>();
  temp->name = std::format("%{}", cnt++);
  temp->pos = pos;
  temp->mut = false; // TODO: 临时变量是否一定不变还有待商榷
  temp->init = true;
  temp->type = std::move(type);

  return temp;
}

} // namespace ir
