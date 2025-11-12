# ngg::targ

Minimal `C++23` library for defining and parsing CLI arguments from compile-time definitions.

# Install
```
cmake -B build
sudo make install
```

# Usage

`targ` works in 2 distinct steps:

## Step 1. Instantiate definitions

At compile time, create definitions for future parser

```cpp
// include for default implicit handlers
#include<targ/targ_default_handlers.hpp>

// flag, equal to boolean
constexpr auto funny_flag = ngg::targ::define_flag("--funny-flag, -F");

// value arg, using implicit automatic handler
constexpr auto number_arg = ngg::targ::define_value<size_t>("--num", "-n", "--number-of-things");

constexpr auto mtable = ngg::targ::match_table(funny_flag, number_arg);
```

At this step you supply the program with information about the args, such as:
- string representations of the arg
- type and number of values, if any
- optional handler functor
- repetition policy (not implemented yet)

# Step 2. Runtime parser

Since we know all about the args from compile time, the only runtime work being actually done is:
1. Comparing provided args to definitions
2. Calling handlers
3. Assembling result values

Matching function returns a tuple of parser results:

```cpp
// structure bindings
auto [funny, number] = mtable.match_all(argc, argv);
```

# Nuances

## Handlers

`targ` provides an option to explicitly provide a handler functor at definition time.
This object should satisfy the concept `arg_handler<Tval>` and will be called during runtime
execution of `match_table.match_all` function.

If no handler was explicitly provided, `targ` assumes an automatic handler if any available.
By default, automatic handlers are provided for `bool`, `std::string_view` and arithmetic types
and are exposed in a `<targ/targ_default_handlers.hpp>` header.

## Multiple values

For definitions which expect more than one value, the return type is a wrapper around std::array,
guaranteed to be:
1. Convertible to bool: when converted, return `true` if there is any value to be extracted, `false` otherwise
2. Iteratable via an input iterator
