//
// Copyright (c) 2024 Dmitry Arkhipovwski (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//


#ifndef BOOST_JSON_DETAIL_IMPL_BASIC_PARSER_IPP
#define BOOST_JSON_DETAIL_IMPL_BASIC_PARSER_IPP

#include <boost/json/basic_parser.hpp>

namespace boost {
namespace json {
namespace detail {

void
parser_data::reserve()
{
    if(BOOST_JSON_LIKELY( !st.empty() ))
        return;
    // Reserve the largest stack we need,
    // to avoid reallocation during suspend.
    st.reserve(
        sizeof(parser_state) + // document parsing state
        (sizeof(parser_state) +
            sizeof(std::size_t)) * depth() + // array and object state + size
        sizeof(parser_state) + // value parsing state
        sizeof(std::size_t) + // string size
        sizeof(parser_state)); // comment state
}

//----------------------------------------------------------
//
// These functions are declared with the BOOST_NOINLINE
// attribute to avoid polluting the parsers hot-path.
// They return the canary value to indicate suspension
// or failure.
char const*
parser_data::continue_finish(parser_state s)
{
    if(BOOST_JSON_LIKELY( !ec && more ))
    {
        // suspend
        reserve();
        st.push_unchecked(s);
    }
    return sentinel();
}

char const*
parser_data::continue_finish(parser_state s, std::size_t n)
{
    if(BOOST_JSON_LIKELY( !ec && more ))
    {
        // suspend
        reserve();
        st.push_unchecked(n);
        st.push_unchecked(s);
    }
    return sentinel();
}


char const*
parser_data::fail(char const* p) noexcept
{
    BOOST_ASSERT( p != sentinel() );
    end = p;
    return sentinel();
}

char const*
parser_data::fail(char const* p, error ev, source_location const* loc) noexcept
{
    ec.assign(ev, loc);
    return fail(p);
}

char const*
parser_data::maybe_suspend(char const* p, parser_state s)
{
    if( p != sentinel() )
        end = p;
    if(BOOST_JSON_LIKELY(more))
    {
        // suspend
        reserve();
        st.push_unchecked(s);
    }
    return sentinel();
}

char const*
parser_data::maybe_suspend(char const* p, parser_state s, std::size_t n)
{
    if(BOOST_JSON_LIKELY(more))
    {
        // suspend
        reserve();
        st.push_unchecked(n);
        st.push_unchecked(s);
    }
    return fail(p);
}

char const*
parser_data::maybe_suspend(char const* p, parser_state s, number const& num)
{
    if(BOOST_JSON_LIKELY(more))
        return suspend(p, s, num);
    return fail(p);
}

char const*
parser_data::suspend(char const* p, parser_state s)
{
    reserve();
    st.push_unchecked(s);
    return fail(p);
}

char const*
parser_data::suspend(char const* p, parser_state s, number const& num)
{
    num_ = num;
    return suspend(p, s);
}

} // namespace detail
} // namespace json
} // namespace boost

#endif // BOOST_JSON_DETAIL_IMPL_BASIC_PARSER_IPP
