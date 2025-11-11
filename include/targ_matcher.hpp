#pragma once

#include "targ_definition.hpp"
#include <array>
#include <cassert>
#include <expected>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ngg::targ
{

template <definition_instance Def, size_t N> struct multiarg
{
    using value_type = typename Def::value_type;
    struct enumerated
    {
        typename Def::expected expected;
        size_t idx;
    };

    using inner_type = std::array<typename Def::expected, N>;
    constexpr multiarg (inner_type &&vals) noexcept : values(vals)
    {
    }

    multiarg &operator=(inner_type &&vals) noexcept
    {
        values        = std::move(vals);
        current_index = 0;
        return *this;
    }

    constexpr operator bool () const noexcept
    {
        return current_index < N;
    }

    constexpr auto next () noexcept
    {
        return enumerated{std::move(values[current_index++]), current_index};
    }

    constexpr auto begin ()
    {
        return iterator(*this, 0);
    }

    constexpr auto end ()
    {
        return iterator(*this, N);
    }

    struct iterator
    {
        constexpr iterator (const multiarg<Def, N> &parent, size_t idx)
            : parent(&parent), index(idx)
        {
        }

        constexpr auto
        operator<=>(const iterator &other) const noexcept = default;

        constexpr auto operator*() noexcept
        {
            assert(index < N);
            return enumerated{parent->values[index], index};
        }

        constexpr auto operator++() noexcept
        {
            index++;
            return *this;
        }

        constexpr auto operator++(int) noexcept
        {
            auto temp = *this;
            ++(*this);
            return temp;
        }

        const multiarg<Def, N> *parent;
        size_t index;
    };

  private:
    size_t current_index = 0;
    inner_type values;
};

template <definition_instance... Definitions> struct match_table
{
    consteval match_table (Definitions... defs) noexcept
        : names(defs.arg_names...), handlers(defs.handler...)
    {
    }

    template <size_t I>
    constexpr auto match (std::string_view sv) const noexcept -> bool
    {
        const auto &curr_names = std::get<I>(names);
        for (const auto &name : curr_names)
            if (name == sv)
                return true;
        return false;
    }

    template <typename Tval, size_t N>
    static consteval auto filled_array (Tval default_value) noexcept
    {
        auto retval = std::array<Tval, N>{};
        for (auto &it : retval)
            it = default_value;
        return retval;
    }

    template <typename D>
    using element_t = std::conditional_t<
        (D::consume_amount == 0), bool,
        std::conditional_t<(D::consume_amount == 1), typename D::expected,
                           multiarg<D, D::consume_amount>>>;

    [[nodiscard]]
    constexpr auto match_all (int argc, char **argv) const noexcept
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
                    std::unexpected(arg_error::no_value_provided)};
            }
            else
            {
                return filled_array<typename D::expected, D::consume_amount>(
                    std::unexpected(arg_error::no_value_provided));
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
            auto does_match_any_arg = match<I>(arg);
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
                    slot = std::unexpected(arg_error::end_of_args);
                    return;
                }
                if constexpr (std::is_same_v<decltype(handler), nullhandler_t>)
                {
                    slot = std::unexpected(arg_error::no_valid_handler);
                    return;
                }
                else
                {

                    const std::string_view next{argv[++i]};
                    if (auto opt_val = handler(next); !opt_val)
                    {
                        slot = std::unexpected(arg_error::cannot_parse);
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
                auto ret = filled_array<typename definition_type::expected, N>(
                    std::unexpected(arg_error::no_value_provided));
                for (size_t idx = 0; idx < N; idx++)
                {
                    auto handler = std::get<I>(handlers);
                    if (i + 1 >= argc)
                    {
                        ret[idx] = std::unexpected(arg_error::end_of_args);
                        continue;
                    }
                    if constexpr (std::is_same_v<decltype(handler),
                                                 nullhandler_t>)
                    {
                        ret[idx] = std::unexpected(arg_error::no_valid_handler);
                        continue;
                    }
                    else
                    {
                        const std::string_view next{argv[++i]};
                        if (auto opt_val = handler(next); !opt_val)
                        {
                            ret[idx] = std::unexpected(arg_error::cannot_parse);
                            continue;
                        }
                        else
                        {
                            ret[idx] = *opt_val;
                            continue;
                        }
                    }
                }
                slot = std::move(ret);
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
} // namespace ngg::targ
