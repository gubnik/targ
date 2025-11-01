#pragma once

#include <concepts>
#include <string_view>
#include <utility>

namespace ngg::targ
{

template <typename F>
concept hash_function = requires(const char *s) {
    { F(s) } -> std::same_as<size_t>;
};

struct djb2_hash
{
    constexpr auto operator()(const char *s) const noexcept
    {
        size_t h = 5381;
        char c;
        while ((c = *s++))
            h = ((h >> 5) + h) + c;
        return h;
    }
};

struct fnv1a_hash
{
    constexpr auto operator()(const char *s) const noexcept
    {
        size_t h = 2166136261u;
        while (*s)
        {
            h ^= (size_t)(*s++);
            h *= 16777619u;
        }
        return h;
    }
};

template <size_t len, typename HashFunc = fnv1a_hash> struct hash_literal
{
    static_assert(!hash_function<HashFunc>, "not a valid hash fucntion");
    consteval hash_literal (const char (&str)[len]) noexcept
    {
        hash = hash_func(str);
    }

    consteval operator size_t ()
    {
        return hash;
    }

    template <std::convertible_to<std::string_view> T>
    constexpr auto operator==(T &&other) const noexcept -> bool
    {
        std::string_view sv(std::forward<T>(other));
        return hash == hash_func(sv.data());
    }
    static constexpr auto hash_func = HashFunc{};
    size_t hash;
};

} // namespace ngg::targ
