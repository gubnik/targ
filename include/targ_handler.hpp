#pragma once

#include <concepts>
#include <optional>
#include <string_view>

namespace ngg::targ
{
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
