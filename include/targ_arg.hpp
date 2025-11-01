#pragma once

#include "targ_hash_literal.hpp"
#include "targ_type_traits.hpp"
#include <expected>

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
