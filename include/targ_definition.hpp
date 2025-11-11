#pragma once

#include "targ_handler.hpp"
#include <array>
#include <concepts>
#include <expected>
#include <format>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ngg::targ
{

enum class arg_error
{
    no_value_provided,
    no_valid_handler,
    end_of_args,
    out_of_range,
    cannot_parse,
};
template <typename Tval, size_t ConsumeN, arg_handler_invokable<Tval> Thandler,
          size_t NamesSize>
struct definition
{
    static constexpr size_t consume_amount = ConsumeN;
    using value_type                       = Tval;
    using handler_type                     = Thandler;
    using names_type = std::array<std::string_view, NamesSize>;
    using expected   = std::expected<Tval, arg_error>;

    consteval definition (Thandler handler,
                          std::convertible_to<std::string_view> auto &&...names)
        : arg_names({{std::forward<decltype(names)>(names)...}}),
          handler(std::forward<Thandler>(handler))
    {
    }

    const names_type arg_names;
    const handler_type handler;
};

template <typename Tval, size_t N = 1>
consteval auto
define_value (arg_handler_invokable<Tval> auto handler,
              std::convertible_to<std::string_view> auto &&...names)
    requires(!std::is_same_v<decltype(handler), nullhandler_t>)
{
    return definition<Tval, N, decltype(handler), sizeof...(names)>(
        std::forward<decltype(handler)>(handler),
        std::forward<decltype(names)>(names)...);
}

template <typename Tval, size_t N = 1>
consteval auto
define_value (std::convertible_to<std::string_view> auto &&...names)
{
    return definition<Tval, N, handler<Tval>, sizeof...(names)>(
        handler<Tval>{}, std::forward<decltype(names)>(names)...);
}

template <typename Tval>
consteval auto
define_flag (std::convertible_to<std::string_view> auto &&...names)
{
    return definition<Tval, 0, nullhandler_t, sizeof...(names)>(
        nullhandler, std::forward<decltype(names)>(names)...);
}

template <typename T> struct is_definition : std::false_type
{
};

template <typename Tval, size_t ConsumeN, typename Thandler, std::size_t N>
struct is_definition<definition<Tval, ConsumeN, Thandler, N>> : std::true_type
{
};

template <typename T>
inline constexpr bool is_definition_v =
    is_definition<std::remove_cv_t<std::remove_reference_t<T>>>::value;

template <typename T>
concept definition_instance = is_definition_v<T>;

} // namespace ngg::targ

template <> struct std::formatter<ngg::targ::arg_error, char>
{
    static constexpr auto
    arg_error_to_string (ngg::targ::arg_error err) noexcept -> std::string_view
    {
        switch (err)
        {
            using enum ngg::targ::arg_error;
        case no_value_provided:
            return "no value provided";
        case no_valid_handler:
            return "no valid handler";
        case end_of_args:
            return "end of args";
        case out_of_range:
            return "value out of range";
        case cannot_parse:
            return "cannot parse argument value";
        }
    }
    template <typename ParserContext> constexpr auto parse (ParserContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format (const ngg::targ::arg_error &err, FormatContext &ctx) const
    {
        if (auto name = arg_error_to_string(err); !name.empty())
        {
            return std::format_to(ctx.out(), "{}", name);
        }
        using U = std::underlying_type_t<ngg::targ::arg_error>;
        return std::format_to(ctx.out(), "{}", static_cast<U>(err));
    }
};
