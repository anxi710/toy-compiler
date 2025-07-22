#pragma once

#include <array>
#include <string>
#include <cstdint>

#include "panic.hpp"

namespace cg {

enum class Register : std::uint8_t {
  // caller saved register
  A0 =  0, A1 =  1, A2  =  2, A3  =  3,
  A4 =  4, A5 =  5, A6  =  6, A7  =  7,
  T0 =  8, T1 =  9, T2  = 10, T3  = 11,
  T4 = 12, T5 = 13, T6  = 14,

  // callee saved register
  S0 = 15, S1 = 16, S2  = 17, S3  = 18,
  S4 = 19, S5 = 20, S6  = 21, S7  = 22,
  S8 = 23, S9 = 24, S10 = 25, S11 = 26,
};

constexpr unsigned char REG_SIZE = 8; // 8 byte = 64 bit

constexpr unsigned char CALLER_SAVED_REG_CNT = 15;
constexpr unsigned char CALLEE_SAVED_REG_CNT = 12;

constexpr unsigned char AVAILABLE_REG_CNT =
  CALLER_SAVED_REG_CNT + CALLEE_SAVED_REG_CNT;

inline int toIndex(Register r) {
  return static_cast<int>(r);
}

inline Register toReg(int idx) {
  return static_cast<Register>(idx);
}

inline bool isCaller(Register reg) {
  return toIndex(reg) < CALLER_SAVED_REG_CNT;
}

inline bool isCallee(Register reg) {
  return toIndex(reg) >= CALLER_SAVED_REG_CNT;
}

constexpr std::array<Register, CALLER_SAVED_REG_CNT> CALLER_SAVED_REGS = {
  Register::A0, Register::A1, Register::A2, Register::A3,
  Register::A4, Register::A5, Register::A6, Register::A7,
  Register::T0, Register::T1, Register::T2, Register::T3,
  Register::T4, Register::T5, Register::T6
};

constexpr std::array<Register, CALLEE_SAVED_REG_CNT> CALLEE_SAVED_REGS = {
  Register::S0, Register::S1, Register::S2,  Register::S3,
  Register::S4, Register::S5, Register::S6,  Register::S7,
  Register::S8, Register::S9, Register::S10, Register::S11
};

inline std::string toString(Register r) {
  switch (r) {
    case Register::A0:  return "a0";
    case Register::A1:  return "a1";
    case Register::A2:  return "a2";
    case Register::A3:  return "a3";
    case Register::A4:  return "a4";
    case Register::A5:  return "a5";
    case Register::A6:  return "a6";
    case Register::A7:  return "a7";
    case Register::T0:  return "t0";
    case Register::T1:  return "t1";
    case Register::T2:  return "t2";
    case Register::T3:  return "t3";
    case Register::T4:  return "t4";
    case Register::T5:  return "t5";
    case Register::T6:  return "t6";
    case Register::S0:  return "s0";
    case Register::S1:  return "s1";
    case Register::S2:  return "s2";
    case Register::S3:  return "s3";
    case Register::S4:  return "s4";
    case Register::S5:  return "s5";
    case Register::S6:  return "s6";
    case Register::S7:  return "s7";
    case Register::S8:  return "s8";
    case Register::S9:  return "s9";
    case Register::S10: return "s10";
    case Register::S11: return "s11";
    default: UNREACHABLE("invalid register");
  }
}

} // namespace cg

namespace std {

template<>
struct formatter<cg::Register> : formatter<std::string> {
  auto format(const cg::Register &reg, format_context &ctx) const {
    return formatter<std::string>::format(toString(reg), ctx);
  }
};

} // namespace std
