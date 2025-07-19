#include <ranges>
#include <string>

#include "panic.hpp"
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
    case IROp::ADD:      return "+";
    case IROp::SUB:      return "-";
    case IROp::MUL:      return "*";
    case IROp::DIV:      return "/";
    case IROp::EQ:       return "==";
    case IROp::NEQ:      return "!=";
    case IROp::GEQ:      return ">=";
    case IROp::GT:       return ">";
    case IROp::LEQ:      return "<=";
    case IROp::LT:       return "<";
    case IROp::ASSIGN:   return "=";
    case IROp::GOTO:     return "goto";
    case IROp::BNEZ:     return "bnez";
    case IROp::BLT:      return "blz";
    case IROp::LABEL:    return "label";
    case IROp::FUNC:     return "func";
    case IROp::RETURN:   return "return";
    case IROp::PARAM:    return "param";
    case IROp::CALL:     return "call";
    case IROp::INDEX:    return "[]";
    case IROp::DOT:      return ".";
    case IROp::MAKE_ARR: return "make_array";
    case IROp::MAKE_TUP: return "make_tuple";
  } // end of switch
}

/**
 * @brief 四元式 pretty print
 */
[[nodiscard]] std::string
IRQuad::str() const
{
  switch (op) {
    case IROp::ADD:
    case IROp::SUB:
    case IROp::MUL:
    case IROp::DIV:
    case IROp::EQ:
    case IROp::NEQ:
    case IROp::GEQ:
    case IROp::GT:
    case IROp::LEQ:
    case IROp::LT:
      return std::format(
        "  {} = {} {} {}",
        dst.str(),
        arg1.str(),
        irop2str(op),
        arg2.str()
      );
    case IROp::INDEX:
      return std::format(
        "  {} = {}[{}]",
        dst.str(),
        arg1.str(),
        arg2.str()
      );
    case IROp::DOT:
      return std::format(
        "  {} = {}.{}",
        dst.str(),
        arg1.str(),
        arg2.str()
      );
    case IROp::ASSIGN:
      return std::format(
        "  {} = {}",
        dst.str(),
        arg1.str()
      );
    case IROp::GOTO:
      return std::format(
        "  {} {}",
        irop2str(op),
        label
      );
    case IROp::CALL:
      return std::format(
        "  {} = call {}",
        dst.str(),
        label
      );
    case IROp::LABEL:
    case IROp::FUNC:
      return std::format(
        "{}:",
        label
      );
    case IROp::BNEZ:
      return std::format(
        "  if {} != 0 goto {}",
        arg1.str(),
        label
      );
    case IROp::BLT:
      return std::format(
        "  if {} < {} goto {}",
        arg1.str(),
        arg2.str(),
        label
      );
    case IROp::PARAM:
      return std::format(
        "  param {}",
        arg1.str()
      );
    case IROp::RETURN:
      return std::format(
        "  return {} ({})",
        arg1.str(),
        label
      );
    case IROp::MAKE_ARR:
    case IROp::MAKE_TUP: {
      ASSERT_MSG(
        !elems.empty(),
        "MAKE_ARR/MAKE_TUP with zero elements"
      );
      return std::format("  {} = {}({})",
        dst.str(),
        irop2str(op),
        elems
      | std::views::transform([](const auto &elem){
          return elem.str();
        })
      | std::views::join_with(std::string{", "})
      | std::ranges::to<std::string>()
      );
    }
  } // end of switch
}

} // namespace ir
