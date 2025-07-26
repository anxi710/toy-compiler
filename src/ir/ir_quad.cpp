#include <ranges>
#include <string>

#include "ir_quad.hpp"

namespace ir {

/**
 * @brief  IR Operator 转 string
 * @param  op 待转换的 IR Operator
 * @return 转换到的 string
 */
[[nodiscard]] std::string
irop2str(const IROp &op)
{
  switch (op) {
#define TRANSFORM(name, str) case IROp::name: return str;
  IROP_LIST(TRANSFORM)
#undef TRANSFORM
  } // end of switch
}

static std::string
dumpElems(const std::vector<Operand> &elems)
{
  if (elems.size() == 0) {
    return "";
  }

  return elems
  | std::views::transform([](const auto &elem){
      return elem.str();
    })
  | std::views::join_with(std::string{", "})
  | std::ranges::to<std::string>();
}

/**
 * @brief 四元式 pretty print
 */
[[nodiscard]] std::string
IRQuad::str() const
{
  switch (op) {
    case IROp::ADD: case IROp::SUB:
    case IROp::MUL: case IROp::DIV:
    case IROp::EQ:  case IROp::NEQ:
    case IROp::GT:  case IROp::GEQ:
    case IROp::LT:  case IROp::LEQ:
      return std::format("{} = {} {} {}",
        dst.str(), arg1.str(), irop2str(op), arg2.str()
      );
    case IROp::INDEX:
      return std::format("{} = {}[{}]",
        dst.str(), arg1.str(), arg2.str()
      );
    case IROp::DOT:
      return std::format("{} = {}.{}",
        dst.str(), arg1.str(), arg2.str()
      );
    case IROp::ASSIGN:
      return std::format("{} = {}", dst.str(), arg1.str());
    case IROp::GOTO:
      return std::format("{} {}", irop2str(op), label);
    case IROp::CALL:
      return std::format("{} = call {}({})",
        dst.str(), label, dumpElems(elems)
      );
    case IROp::LABEL: case IROp::FUNC:
      return std::format("{}:", label);
    case IROp::BEQZ:
      return std::format("if {} == 0 goto {}", arg1.str(), label);
    case IROp::BNEZ:
      return std::format("if {} != 0 goto {}", arg1.str(), label);
    case IROp::BGE:
      return std::format("if {} >= {} goto {}",
        arg1.str(), arg2.str(), label
      );
    case IROp::RETURN:
      return std::format("return {} -> {}", arg1.str(), label);
    case IROp::MAKE_ARR: case IROp::MAKE_TUP: {
      return std::format("{} = {}({})",
        dst.str(), irop2str(op), dumpElems(elems)
      );
    }
  } // end of switch
}

} // namespace ir
