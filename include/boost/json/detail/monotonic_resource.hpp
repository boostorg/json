//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_MONOTONIC_RESOURCE_HPP
#define BOOST_JSON_DETAIL_MONOTONIC_RESOURCE_HPP

#include <boost/json/detail/config.hpp>

#include <cstddef>

namespace boost {
namespace json {
namespace detail {

struct class_type {};
enum unscoped_enumeration_type { };
enum class scoped_enumeration_type { };

// [support.types] p5: The type max_align_t is a trivial
// standard-layout type whose alignment requirement
// is at least as great as that of every scalar type.
struct max_align_t
{
    // arithmetic types
    char a;
    char16_t b;
    char32_t c;
    bool d;
    short int e;
    int f;
    long int g;
    long long int h;
    wchar_t i;
    float j;
    double k;
    long double l;
    // enumeration types
    unscoped_enumeration_type m;
    scoped_enumeration_type n;
    // pointer types
    void* o;
    char* p;
    class_type* q;
    unscoped_enumeration_type* r;
    scoped_enumeration_type* s;
    void(*t)();
    // pointer to member types
    char class_type::* u;
    void (class_type::*v)();
    // nullptr
    std::nullptr_t w;
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
        (reinterpret_cast<std::uintptr_t>(
            ptr) + align - 1) & ~(align - 1));
}

} // detail
} // json
} // boost

#endif
