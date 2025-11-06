#include "include/targ_definition.hpp"
#include "include/targ_handler.hpp"
#include <array>
#include <expected>
#include <optional>
#include <print>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace targ = ngg::targ;

template <ngg::targ::definition_instance... Definitions> struct match_table
{
    consteval match_table (Definitions... defs) noexcept
        : names(defs.arg_names...), handlers(defs.handler...)
    {
    }

    constexpr auto match (std::string_view sv) const noexcept -> bool
    {
        auto match_lambda = [&] (auto index, std::string_view sv) -> bool
        {
            constexpr size_t I     = decltype(index)::value;
            const auto &curr_names = std::get<I>(names);
            for (const auto &name : curr_names)
                if (name == sv)
                    return true;
            return false;
        };

        return [&]<std::size_t... Is>(std::index_sequence<Is...>) -> bool
        {
            bool result = false;
            ((result || (result = match_lambda(
                             std::integral_constant<std::size_t, Is>{}, sv),
                         false)),
             ...);
            return result;
        }(std::index_sequence_for<Definitions...>{});
    }

    template <typename D>
    using element_t = std::conditional_t<
        (D::consume_amount == 0), bool,
        std::conditional_t<
            (D::consume_amount == 1), typename D::expected,
            std::array<typename D::expected, D::consume_amount>>>;

    [[nodiscard]]
    constexpr auto match_all (std::string_view sv, int argc,
                              char **argv) const noexcept
        -> std::tuple<element_t<Definitions>...>
    {
        constexpr auto res_f = []<typename D>() -> element_t<D>
        {
            if constexpr (D::consume_amount == 0)
            {
                return false;
            }
            else if constexpr (D::consume_amount == 1)
            {
                return element_t<D>{
                    std::unexpected(ngg::targ::arg_error::no_value_provided)};
            }
            else
            {
                return element_t<D>{
                    std::unexpected(ngg::targ::arg_error::no_value_provided)};
            }
        };
        auto results = std::tuple<element_t<Definitions>...>{
            res_f.template operator()<Definitions>()...};
        const auto try_match_at = [&] (auto index, size_t &i)
        {
            constexpr size_t I = decltype(index)::value;
            using result_type  = std::tuple_element_t<I, decltype(results)>;
            using definition_type =
                std::tuple_element_t<I, std::tuple<Definitions...>>;
            constexpr size_t N = definition_type::consume_amount;
            const std::string_view arg{argv[i]};
            const size_t start_idx  = i;
            result_type &slot       = std::get<I>(results);
            auto does_match_any_arg = match(arg);
            if (!does_match_any_arg)
            {
                return;
            }
            if constexpr (N == 0)
            {
                slot = true;
                return;
            }
            if constexpr (N == 1)
            {
                auto handler = std::get<I>(handlers);
                if (i + 1 >= argc)
                {
                    slot = std::unexpected(ngg::targ::arg_error::end_of_args);
                    return;
                }
                if constexpr (std::is_same_v<
                                  std::invoke_result_t<decltype(handler),
                                                       std::string_view>,
                                  std::nullopt_t>)
                {
                    slot =
                        std::unexpected(ngg::targ::arg_error::no_valid_handler);
                    return;
                }
                else
                {

                    const std::string_view next{argv[++i]};
                    if (auto opt_val = handler(arg); !opt_val)
                    {
                        slot =
                            std::unexpected(ngg::targ::arg_error::cannot_parse);
                        return;
                    }
                    else
                    {
                        slot = *opt_val;
                        return;
                    }
                }
            }
            if constexpr (N > 1)
            {
                std::array<typename definition_type::expected, N> ret = {
                    std::unexpected(ngg::targ::arg_error::no_value_provided)};
                for (size_t idx = 0; idx < N; idx++)
                {
                    auto handler = std::get<I>(handlers);
                    if (i + 1 >= argc)
                    {
                        ret[idx] =
                            std::unexpected(ngg::targ::arg_error::end_of_args);
                        continue;
                    }
                    if constexpr (std::is_same_v<
                                      std::invoke_result_t<decltype(handler),
                                                           std::string_view>,
                                      std::nullopt_t>)
                    {
                        ret[idx] = std::unexpected(
                            ngg::targ::arg_error::no_valid_handler);
                        continue;
                    }
                    else
                    {
                        const std::string_view next{argv[++i]};
                        if (auto opt_val = handler(next); !opt_val)
                        {
                            ret[idx] = std::unexpected(
                                ngg::targ::arg_error::cannot_parse);
                            continue;
                        }
                        else
                        {
                            ret[idx] = *opt_val;
                            continue;
                        }
                    }
                }
                slot = ret;
            }
        };
        for (size_t i = 0; i < argc; ++i)
        {
            (
                // hack because we cannot iterate over this kind of sequence
                // directly
                [&]<std::size_t... Is>(std::index_sequence<Is...>)
                {
                    (try_match_at(std::integral_constant<std::size_t, Is>{}, i),
                     ...);
                })(std::index_sequence_for<Definitions...>{});
        }
        return results;
    }

    const std::tuple<typename Definitions::names_type...> names;
    const std::tuple<typename Definitions::handler_type...> handlers;
};

struct test_handler
{
    consteval test_handler() {};
    constexpr auto operator()(std::string_view sv) const noexcept
        -> std::optional<size_t>
    {
        std::println("Called with {}", sv);
        return 42;
    }
};

auto main (int argc, char **argv) -> int
{
    // simulate runtime input
    std::string_view sv = "test";

    constexpr auto def1 = targ::define_value<size_t, 2>(test_handler{}, "test");
    constexpr auto def2 = targ::define_value<size_t, 1>("abc");
    constexpr auto def3 = targ::define_flag<char>("num", "n");
    constexpr auto mtable = match_table(def1, def2, def3);
    auto opt              = mtable.match(sv);
    if (opt)
    {
        std::println("Matched with {}", opt);
    }
    auto [n1, n2, n3] = mtable.match_all(sv, argc, argv);
    for (size_t idx; idx < n1.size(); idx++)
    {
        auto n = n1[idx];
        if (!n)
        {
            std::println("Error at {}: {}", idx, (size_t)n.error());
        }
        else
            std::println("Success at {}: {}", idx, *n);
    }
}
