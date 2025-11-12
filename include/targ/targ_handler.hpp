#pragma once

#include <concepts>
#include <optional>
#include <string_view>

namespace ngg::targ
{

template <typename Tval>
constexpr auto handle_arg(std::string_view arg_str) noexcept
    -> std::optional<Tval>;

template <typename Tval>
concept default_arg_handler = requires(std::string_view sv) {
    { ngg::targ::handle_arg<Tval>(sv) } -> std::same_as<std::optional<Tval>>;
};

template <typename Tval> struct handler
{
    constexpr auto operator()(std::string_view to_match) const noexcept
        -> std::optional<Tval>
    {
        return targ::handle_arg<Tval>(to_match);
    }
};

template <typename Thandler, typename Tval>
concept arg_handler_invokable =
    requires(const Thandler handler, std::string_view arg_str) {
        {
            handler.operator()(arg_str)
        } -> std::convertible_to<std::optional<Tval>>;
    };

struct nullhandler_t
{
    constexpr auto operator()(std::string_view) const noexcept -> auto
    {
        return std::nullopt;
    }
};

static constexpr auto nullhandler = nullhandler_t{};

} // namespace ngg::targ
