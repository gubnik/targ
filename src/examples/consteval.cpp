#include "targ/targ_default_handlers.hpp"
#include "targ/targ_definition.hpp"
#include "targ/targ_matcher.hpp"
#include <print>

namespace targ = ngg::targ;
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
    constexpr auto def1   = targ::define_value<size_t, 2>("test");
    constexpr auto def2   = targ::define_value<size_t>("abc");
    constexpr auto def3   = targ::define_flag("num", "n");
    constexpr auto mtable = ngg::targ::match_table(def1, def2, def3);
    auto [arr1, n2, n3]   = mtable.match_all(argc, argv);
    for (const auto &[n, idx] : arr1)
    {
        if (!n)
        {
            std::println("Error for test at {}: {}", idx, n.error());
        }
        else
            std::println("Success for test at {}: {}", idx, *n);
    }
    if (!n2)
    {
        std::println("Error for abc : {}", n2.error());
    }
    else
        std::println("Success for abc: {}", *n2);
}
