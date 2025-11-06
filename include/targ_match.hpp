#pragma once

#include "targ_arg.hpp"
#include "targ_handler.hpp"
#include <optional>
#include <tuple>

namespace ngg::targ
{
template <is_arg_type Arg>
constexpr auto
get_arg_handler (std::convertible_to<std::string_view> auto &&to_match) noexcept
    -> std::optional<handler<typename Arg::value_type>>
{
    using str_type = std::remove_reference_t<decltype(to_match)>;
    if (Arg::arg_type::match(to_match) &&
        default_arg_handler<typename Arg::value_type>)
    {
        return handler<typename Arg::value_type>{};
    }
    return std::nullopt;
}

template <is_arg_type Arg>
constexpr auto
validate_argument (std::optional<typename Arg::value_type> val_opt) noexcept
    -> parsed_arg<Arg>
{
    if (!val_opt)
    {
        if constexpr (has_default_arg_value<Arg>)
        {
            return Arg::default_value;
        }
        else
        {
            return std::unexpected{arg_error::parser_error};
        }
    }
    else if constexpr (has_min_arg_value<Arg>)
    {
        if (*val_opt < Arg::min_value)
        {
            return std::unexpected{arg_error::value_out_of_range};
        }
    }
    if constexpr (has_max_arg_value<Arg>)
    {
        if (*val_opt > Arg::max_value)
        {
            return std::unexpected{arg_error::value_out_of_range};
        }
    }
    return *val_opt;
}

template <is_pack_arg_types... Args>
constexpr auto match_all (int argc, char **argv) noexcept
    -> std::tuple<parsed_arg<Args>...>
{
    // init with unexpected because default init is an expected default
    std::tuple<parsed_arg<Args>...> results = std::make_tuple(parsed_arg<Args>{
        std::unexpected<arg_error>{arg_error::missing}}...); // missing == 0
    // helper to try and match
    auto try_match_at = [&] (auto index, std::string_view sv,
                             int &i) noexcept -> void
    {
        constexpr std::size_t I = decltype(index)::value;
        using ArgT              = std::tuple_element_t<I, std::tuple<Args...>>;
        auto &slot              = std::get<I>(results);
        if (slot)
            return;
        if (auto handler_opt = get_arg_handler<ArgT>(sv); !handler_opt)
        {
            slot = std::unexpected{arg_error::no_such_handler};
            return;
        }
        else if constexpr (is_flag_arg<ArgT>)
        {
            auto val_opt = (*handler_opt)(sv);
            slot         = validate_argument<ArgT>(val_opt);
        }
        else
        {
            int next_i = i + 1;
            if (next_i >= argc)
            {
                slot = std::unexpected{arg_error::no_value_provided};
                return;
            }
            std::string_view arg_value(argv[next_i]);
            auto val_opt = (*handler_opt)(arg_value);
            slot         = validate_argument<ArgT>(val_opt);
            i++;
        }
    };

    // for all elements in argv, match with every other argv
    for (int i = 0; i < argc; ++i)
    {
        std::string_view sv(argv[i]);
        (
            // hack because we cannot iterate over this kind of sequence
            // directly
            [&]<std::size_t... Is>(std::index_sequence<Is...>)
            {
                (try_match_at(std::integral_constant<std::size_t, Is>{}, sv, i),
                 ...);
            })(std::index_sequence_for<Args...>{});
    }
    return results;
}

} // namespace ngg::targ
