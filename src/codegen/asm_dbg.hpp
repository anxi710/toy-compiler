#pragma once

#ifdef VERBOSE
#include <print>
#endif

namespace cg {

#ifdef VERBOSE
#define DBG(out, fmt, ...) std::println(out, fmt, ##__VA_ARGS__)
#else
#define DBG(out, fmt, ...)
#endif

} // namespace cg
