#pragma once

#include <compare>
#include <concepts>
#include <type_traits>

namespace ngg::targ
{

template <typename T>
concept is_arg_type = requires {
    typename T::arg_type;
    typename T::value_type;
};

template <typename... Ts>
concept is_pack_arg_types = (is_arg_type<Ts> && ...);

template <typename Arg>
concept has_default_arg_value = is_arg_type<Arg> && requires() {
    { Arg::default_value } -> std::convertible_to<typename Arg::value_type>;
};

template <typename Arg>
concept has_min_arg_value =
    is_arg_type<Arg> && std::three_way_comparable<typename Arg::value_type> &&
    requires() {
        { Arg::min_value } -> std::convertible_to<typename Arg::value_type>;
    };

template <typename Arg>
concept has_max_arg_value =
    is_arg_type<Arg> && std::three_way_comparable<typename Arg::value_type> &&
    requires() {
        { Arg::max_value } -> std::convertible_to<typename Arg::value_type>;
    };

template <typename Arg>
concept is_flag_arg = is_arg_type<Arg> && requires {
    typename Arg::flag_type;
} && std::is_same_v<typename Arg::value_type, typename Arg::flag_type>;

} // namespace ngg::targ
