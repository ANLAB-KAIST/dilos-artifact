/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef INITIALIZE_HH_
#define INITIALIZE_HH_

#include <type_traits>
#include <array>
#include <initializer_list>
#include <utility>
#include "index-list.hh"

// helper to find out the argument type of a function object
template <typename T>
struct first_argument_type_of;

// special case: free function
template <typename T, typename Ret>
struct first_argument_type_of<Ret (T)> {
    typedef T type;
};

// special case: member function pointer
template <typename T1, typename T2, typename Ret>
struct first_argument_type_of<Ret (T1::*)(T2)> {
    typedef T2 type;
};

// special case: member function pointer (const)
template <typename T1, typename T2, typename Ret>
struct first_argument_type_of<Ret (T1::*)(T2) const> {
    typedef T2 type;
};

// general case: class, look at operator()(T)
template <typename T>
struct first_argument_type_of {
    typedef typename first_argument_type_of<decltype(&T::operator())>::type type;
};

template <typename T>
using first_argument_no_ref = typename std::remove_reference<typename first_argument_type_of<T>::type>::type;

/// \file Initialization helper for C designated initializers

/// \fn T initialize(func f)
/// \brief C compatibility for designated initializers
///
/// Unfortunately C++ does not support designated initializers; so
/// this function helps fill its place by default-initializing an object
/// and then calling a user supplied function (usually a lambda) to complete
/// initialization.
template <typename func>
auto initialize_with(func f) -> first_argument_no_ref<func>
{
    first_argument_no_ref<func> val = {};
    f(val);
    return val;
}

template <typename val, size_t size>
std::array<val, size>
initialize_array(std::initializer_list<std::pair<size_t, val>> il)
{
    std::array<val, size> ret;
    for (auto ile : il) {
        ret[ile.first] = ile.second;
    }
    return ret;
}

// Compile-time array initialization from a template, instead of run-time
// function evaluation.  Useful with make_index_list<...>.
template <typename T, size_t N, typename IndexList, template <size_t K> class Initializer>
struct initialized_array;

template <typename T, size_t N, size_t... Indices, template <size_t K> class Initializer>
struct initialized_array<T, N, index_list<Indices...>, Initializer> : std::array<T, N> {
    constexpr initialized_array() : std::array<T, N>{ { Initializer<Indices>::value... } } {}
};

#endif /* INITIALIZE_HH_ */
