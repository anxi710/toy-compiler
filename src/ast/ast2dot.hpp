#pragma once

#include <fstream>

#include "ast.hpp"

namespace par::ast {

// dot 结点声明
struct DotNodeDecl {
  std::string name;  // 结点名 - 唯一，用于区分不同结点
  std::string label; // 标签 - 不唯一，用于图片显示

  DotNodeDecl() = default;
  explicit DotNodeDecl(const std::string &n, const std::string &l)
    : name(n), label(l) {}
  explicit DotNodeDecl(std::string &&n, std::string &&l)
    : name(std::move(n)), label(std::move(l)) {}

  ~DotNodeDecl() = default;

  [[nodiscard]]
  auto initialized() const -> bool;
  [[nodiscard]]
  auto toString() const -> std::string;
};

void ast2Dot(std::ofstream &out, const ProgPtr &prog);

} // namesapce par::ast
