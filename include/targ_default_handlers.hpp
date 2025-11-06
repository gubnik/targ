#pragma once

#include "targ_handler.hpp"
#include <charconv>
#include <optional>
#include <string_view>

namespace ngg::targ
{
template <>
constexpr auto handle_arg<bool>(std::string_view arg_str) noexcept
    -> std::optional<bool>
{
    return true;
}

template <>
constexpr auto handle_arg<std::string_view>(std::string_view arg_str) noexcept
    -> std::optional<std::string_view>
{
    return arg_str;
}

template <typename T>
constexpr auto handle_arg (std::string_view arg_str) noexcept
    -> std::optional<T>
{
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

    T val{};
    auto res =
        std::from_chars(arg_str.data(), arg_str.data() + arg_str.size(), val);
    if (res.ec == std::errc())
    {
        return val;
    }
    return std::nullopt;
}

} // namespace ngg::targ
