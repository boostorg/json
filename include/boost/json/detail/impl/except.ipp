//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_EXCEPT_IPP
#define BOOST_JSON_DETAIL_IMPL_EXCEPT_IPP

#include <boost/json/detail/except.hpp>
#ifndef BOOST_JSON_STANDALONE
# include <boost/version.hpp>
# include <boost/throw_exception.hpp>
#elif defined(BOOST_JSON_STANDALONE) && defined(BOOST_NO_EXCEPTIONS)
# include <exception>
#endif
#include <stdexcept>

BOOST_JSON_NS_BEGIN

#ifdef BOOST_JSON_STANDALONE

#ifdef BOOST_NO_EXCEPTIONS
// When exceptions are disabled
// in standalone, you must provide
// this function.
BOOST_NORETURN
void
throw_exception(std::exception const&);

#else
template<class E>
void
BOOST_NORETURN
throw_exception(E e)
{
    throw e;
}

#endif
#endif

namespace detail {

void
throw_bad_alloc(
    source_location const& loc)
{
    throw_exception(
        std::bad_alloc()
#if BOOST_VERSION >= 107300
        , loc
#endif
        );
}

void
throw_length_error(
    char const* what,
    source_location const& loc)
{
    throw_exception(
        std::length_error(what)
#if BOOST_VERSION >= 107300
        , loc
#endif
        );
}

void
throw_invalid_argument(
    char const* what,
    source_location const& loc)
{
    throw_exception(
        std::invalid_argument(what)
#if BOOST_VERSION >= 107300
        , loc
#endif
        );
}

void
throw_out_of_range(
    source_location const& loc)
{
    throw_exception(
        std::out_of_range(
            "out of range")
#if BOOST_VERSION >= 107300
        , loc
#endif
        );
}

void
throw_system_error(
    error_code const& ec,
    source_location const& loc)
{
    throw_exception(
        system_error(ec)
#if BOOST_VERSION >= 107300
        , loc
#endif
        );
}

} // detail
BOOST_JSON_NS_END

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/impl/except.ipp>
#endif

#endif
