#include <targ/targ.hpp>
#include <targ/targ_aliases.hpp>
#include <targ/targ_default_handlers.hpp>

#include <print>
#include <string>
#include <string_view>

// enables a, ar, adr aliases for arg definitions
using namespace ngg::targ::aliases;

// ar == argument in range
using digit_arg = ar<size_t, 0, 10, "-d", "--digit">;

// ad == argument with default (value)
using num_arg = ad<size_t, 1, "-n">;

// a == simple argument with no constraints
// specialization from default handlers
using str_flag = a<std::string_view, "-s", "--string", "--provided-string">;

// a (with bool) == argument flag (accepts no value)
using flag_arg = a<bool, "-f", "--flag">;

// example: copying & owning string
template <>
constexpr auto
ngg::targ::handle_arg<std::string>(std::string_view arg_str) noexcept
    -> std::optional<std::string>
{
    return std::string{arg_str};
}

// manual arg type definition
struct custom_arg
{
    // accepts any number of string literals
    using arg_type = ngg::targ::arg<"-c", "c", "--custom">;

    // accepts any type if it has a handler specialization
    using value_type = double;

    // default value that will be used if arg was not provided
    static constexpr value_type default_value = 42.42;

    // min and max value require the value_type to be comparable

    // minimal value an arg can take
    static constexpr value_type min_value = -21;

    // maximal value an arg can take
    static constexpr value_type max_value = 21;

    // if defined and is equal to value_type the arg becomes a flag
    // flags don't require and values and are assumed to be toggled
    // bool specialization is provided by default
    using flag_type = void;
};

auto main (int argc, char **argv) -> int
{
    // returns a tuple of std::expected<ArgType, ngg::targ::arg_error>
    // args are pre-hashed for comparacement at compile time
    // where possible, additional validation logic is removed at compile time
    auto [digit, num, flag, string, custom] =
        ngg::targ::match_all<digit_arg, num_arg, flag_arg, str_flag,
                             custom_arg>(argc, argv);
    if (digit)
        std::println("digit: {}", *digit);
    if (num)
        std::println("num: {}", *num);
    if (flag)
        std::println("flag: {}", *flag);
    if (string)
        std::println("string: {}", *string);
    if (custom)
        std::println("custom: {}", *custom);
}
