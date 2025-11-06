#include "include/targ_matcher.hpp"
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
    // simulate runtime input
    std::string_view sv = "test";

    constexpr auto def1 = targ::define_value<size_t, 2>(test_handler{}, "test");
    constexpr auto def2 = targ::define_value<size_t, 1>("abc");
    constexpr auto def3 = targ::define_flag<char>("num", "n");
    constexpr auto mtable = ngg::targ::match_table(def1, def2, def3);
    auto opt              = mtable.match(sv);
    if (opt)
    {
        std::println("Matched with {}", opt);
    }
    auto [n1, n2, n3] = mtable.match_all(argc, argv);
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
