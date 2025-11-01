#pragma once

#include "targ.hpp"
#include "targ_hash_literal.hpp"
#include "targ_type_traits.hpp"
#include <type_traits>

namespace ngg::targ::aliases
{

template <typename Tval, hash_literal... names> struct a
{
    using arg_type   = arg<names...>;
    using value_type = Tval;
};

template <hash_literal... names> struct a<bool, names...>
{
    using arg_type   = arg<names...>;
    using flag_type  = bool;
    using value_type = bool;
};

template <typename Tval, Tval DefaultValue, hash_literal... names>
    requires(!is_flag_arg<Tval>)
struct ad
{
    using arg_type                      = arg<names...>;
    using value_type                    = Tval;
    static constexpr Tval default_value = DefaultValue;
};

template <typename Tval, Tval MinValue, Tval MaxValue, hash_literal... names>
    requires(!is_flag_arg<Tval>)
struct ar
{
    using arg_type                  = arg<names...>;
    using value_type                = Tval;
    static constexpr Tval min_value = MinValue;
    static constexpr Tval max_value = MaxValue;
};

template <typename Tval, Tval DefaultValue, Tval MinValue, Tval MaxValue,
          hash_literal... names>
    requires(!is_flag_arg<Tval>)
struct adr
{
    using arg_type                      = arg<names...>;
    using value_type                    = Tval;
    static constexpr Tval default_value = DefaultValue;
    static constexpr Tval min_value     = MinValue;
    static constexpr Tval max_value     = MaxValue;
};

} // namespace ngg::targ::aliases
