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
    case IROp::ADD:    return "+";
    case IROp::SUB:    return "-";
    case IROp::MUL:    return "*";
    case IROp::DIV:    return "/";
    case IROp::EQ:     return "==";
    case IROp::NEQ:    return "!=";
    case IROp::GEQ:    return ">=";
    case IROp::GT:     return ">";
    case IROp::LEQ:    return "<=";
    case IROp::LT:     return "<";
    case IROp::ASSIGN: return "=";
    case IROp::GOTO:   return "goto";
    case IROp::BEQ:    return "j==";
    case IROp::BNE:    return "j!=";
    case IROp::BGE:    return "j>=";
    case IROp::BGT:    return "j>";
    case IROp::BLE:    return "j<=";
    case IROp::BLT:    return "j<";
    case IROp::LABEL:  return "label";
    case IROp::FUNC:   return "func";
    case IROp::RETURN: return "return";
    case IROp::PARAM:  return "param";
    case IROp::CALL:   return "call";
    case IROp::INDEX:  return "[]";
    case IROp::DOT:    return ".";
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
    case IROp::INDEX:
    case IROp::DOT:
      return std::format(
        "({}, {}, {}, {})",
        irop2str(op),
        arg1.value.value()->name,
        arg2.value.value()->name,
        dst.value.value()->name
      );
    case IROp::ASSIGN:
      return std::format(
        "({}, {}, -, {})",
        irop2str(op),
        arg1.value.value()->name,
        dst.value.value()->name
      );
    case IROp::GOTO:
    case IROp::LABEL:
    case IROp::FUNC:
    case IROp::CALL:
      return std::format(
        "({}, -, -, {})",
        irop2str(op),
        label
      );
    case IROp::BEQ:
    case IROp::BNE:
    case IROp::BGE:
    case IROp::BGT:
    case IROp::BLE:
    case IROp::BLT:
      return std::format(
        "({}, {}, {}, {})",
        irop2str(op),
        arg1.value.value()->name,
        arg2.value.value()->name,
        label
      );
    case IROp::RETURN:
    case IROp::PARAM:
      return std::format(
        "({}, {}, -, -)",
        irop2str(op),
        arg1.value.value()->name
      );
  } // end of switch
}

} // namespace ir
