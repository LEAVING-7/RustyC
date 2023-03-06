#pragma once
#include <cstdint>
#include <cstdlib>
#include <vector>
using u8 = ::std::uint8_t;
using u16 = ::std::uint16_t;
using u32 = ::std::uint32_t;
using u64 = ::std::uint64_t;
using i8 = ::std::int8_t;
using i16 = ::std::int16_t;
using i32 = ::std::int32_t;
using i64 = ::std::int64_t;
#include "marco.hpp"
#include <fmt/format.h>
#include <iostream>
#include <source_location>
#include <string_view>

namespace utils {
using SrcLoc = std::source_location;

using fmt::format;
using fmt::make_format_args;
using fmt::vformat;

template <typename... Args>
[[noreturn]] constexpr void Unreachable(SrcLoc loc, std::string_view fmt = "", Args... args)
{
  std::cout << format("Unreachable at: {}:{}:{}\n", loc.file_name(), loc.line(), loc.column())
            << vformat(fmt, make_format_args(std::forward<Args>(args)...));
  abort();
};

template <typename... Args>
[[noreturn]] constexpr void Unimplemented(SrcLoc loc, std::string_view fmt = "", Args... args)
{
  std::cout << format("Unimplemented at: {}:{}:{}\n", loc.file_name(), loc.line(), loc.column())
            << vformat(fmt, make_format_args(std::forward<Args>(args)...));
  abort();
};
#define TODO(why) static_assert(false, "TODO: " why)

} // namespace utils