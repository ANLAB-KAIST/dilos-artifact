/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef OSV_ILOG2_HH
#define OSV_ILOG2_HH

#include <cstdint>

constexpr unsigned ilog2_roundup_constexpr(std::uintmax_t n)
{
    return n <= 1 ? 0 : 1 + ilog2_roundup_constexpr((n >> 1) + (n & 1));
}

inline unsigned count_leading_zeros(unsigned n)
{
    return __builtin_clz(n);
}

inline unsigned count_leading_zeros(unsigned long n)
{
    return __builtin_clzl(n);
}

inline unsigned count_leading_zeros(unsigned long long n)
{
    return __builtin_clzll(n);
}

inline unsigned count_trailing_zeros(unsigned n)
{
    return __builtin_ctz(n);
}

inline unsigned count_trailing_zeros(unsigned long n)
{
    return __builtin_ctzl(n);
}

inline unsigned count_trailing_zeros(unsigned long long n)
{
    return __builtin_ctzll(n);
}

template <typename T>
inline
unsigned ilog2_roundup(T n)
{
    if (n <= 1) {
        return 0;
    }
    return sizeof(T)*8 - count_leading_zeros(n - 1);
}

template <typename T>
inline
unsigned ilog2(T n)
{
    if (n <= 1) {
        return 0;
    }
    return sizeof(T) * 8 - count_leading_zeros(n) - 1;
}

template <typename T>
inline constexpr
bool is_power_of_two(T n)
{
    return (n & (n - 1)) == 0;
}

#endif
