#pragma once

#include "targ_hash_literal.hpp"
#include "targ_type_traits.hpp"
#include <expected>
#include <format>
#include <string_view>

namespace ngg::targ
{

enum class arg_error
{
    missing,
    no_value_provided,
    parser_error,
    no_such_handler,
    value_out_of_range,
};

constexpr auto arg_error_to_string (arg_error err) noexcept -> std::string_view
{
    switch (err)
    {
        using enum arg_error;
    case missing:
        return "no value nor default";
    case no_value_provided:
        return "no value nor default";
    case parser_error:
        return "cannot parse provided value";
    case no_such_handler:
        return "no known way to handle the value";
    case value_out_of_range:
        return "value out of range";
    }
}

template <is_arg_type Arg>
using parsed_arg = std::expected<typename Arg::value_type, arg_error>;

template <hash_literal... args> struct arg
{
    constexpr arg()                                = delete;
    constexpr arg(const arg<args...> &)            = delete;
    constexpr arg &operator=(const arg<args...> &) = delete;
    constexpr arg(arg<args...> &&)                 = delete;
    constexpr arg &operator=(arg<args...> &&)      = delete;
    template <typename T>
        requires std::is_convertible_v<T, typename std::string_view>
    static constexpr auto match (T &&other) noexcept -> bool
    {
        std::string_view sv(std::forward<T>(other));
        return ((args == sv) || ...);
    }
};

} // namespace ngg::targ

template <typename CharT> struct std::formatter<ngg::targ::arg_error, CharT>
{
    constexpr auto parse (std::basic_format_parse_context<CharT> &ctx)
        -> decltype(ctx.begin())
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != static_cast<CharT>('}'))
        {
            throw std::format_error("invalid format spec for Color");
        }
        return it;
    }

    template <typename FormatContext>
    auto format (const ngg::targ::arg_error &c, FormatContext &ctx)
        -> decltype(ctx.out())
    {
        if (auto name = ngg::targ::arg_error_to_string(c); !name.empty())
        {
            return std::format_to(
                ctx.out(), "{}",
                std::basic_string_view<CharT>(
                    reinterpret_cast<const CharT *>(name.data()), name.size()));
        }
        using U = std::underlying_type_t<ngg::targ::arg_error>;
        return std::format_to(ctx.out(), "{}", static_cast<U>(c));
    }
};
