#pragma once

#include "targ_arg.hpp"
#include <format>

template <> struct std::formatter<ngg::targ::arg_error, char>
{
    static constexpr auto
    arg_error_to_string (ngg::targ::arg_error err) noexcept -> std::string_view
    {
        switch (err)
        {
            using enum ngg::targ::arg_error;
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
