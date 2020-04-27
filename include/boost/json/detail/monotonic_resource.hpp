//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_MONOTONIC_RESOURCE_HPP
#define BOOST_JSON_DETAIL_MONOTONIC_RESOURCE_HPP

#include <boost/json/detail/config.hpp>

#include <cstddef>

namespace boost {
namespace json {
namespace detail {
namespace empty_types {

struct class_type {};
enum unscoped_enumeration_type { };
enum class scoped_enumeration_type { };

} // empty_types

// [support.types] p5: The type max_­align_­t is a trivial
// standard-layout type whose alignment requirement
// is at least as great as that of every scalar type.
struct max_align_t
{
    // arithmetic types
    char a_;
    char16_t b_;
    char32_t c_;
    bool d_;
    short int e_;
    int f_;
    long int g_;
    long long int h_;
    wchar_t i_;
    float j_;
    double k_;
    long double l_;
    // enumeration types
    empty_types::unscoped_enumeration_type m_;
    empty_types::scoped_enumeration_type n_;
    // pointer types
    void* o_;
    char* p_;
    empty_types::class_type* q_;
    empty_types::unscoped_enumeration_type* r_;
    empty_types::scoped_enumeration_type* s_;
    void(*t_)();
    // pointer to member types
    char empty_types::class_type::* u_;
    void (empty_types::class_type::*v_)();
    // nullptr
    std::nullptr_t w_;
};

inline
constexpr
std::size_t
max_align()
{
#ifdef BOOST_JSON_STANDALONE
    return alignof(std::max_align_t);
#else
    return alignof(max_align_t);
#endif
}

inline
unsigned char*
align_up(
    unsigned char* ptr,
    std::size_t align)
{
    // alignment shall be a power of two
    BOOST_ASSERT(!(align & (align - 1)));
    return reinterpret_cast<unsigned char*>(
        (reinterpret_cast<std::uintptr_t>(ptr) + align - 1) & ~(align - 1));
}

} // detail
} // json
} // boost

#endif
