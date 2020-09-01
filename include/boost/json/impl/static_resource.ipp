//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_STATIC_RESOURCE_IPP
#define BOOST_JSON_IMPL_STATIC_RESOURCE_IPP

#include <boost/json/static_resource.hpp>
#include <memory>

#ifndef BOOST_JSON_STANDALONE
#include <boost/align/align.hpp>
#else
#include <memory>
#endif

namespace boost {
namespace json {

namespace detail {

#ifndef BOOST_JSON_STANDALONE
using boost::alignment::align;
#else
using std::align;
#endif

} // detail

static_resource::
~static_resource() noexcept
{
}

static_resource::
static_resource(
    void* buffer,
    std::size_t size) noexcept
    : p_(buffer)
    , n_(size)
{
}

void*
static_resource::
do_allocate(
    std::size_t n,
    std::size_t align)
{
    auto p = detail::align(
        align, n, p_, n_);
    if(! p)
        BOOST_THROW_EXCEPTION(
            std::bad_alloc());
    p_ = reinterpret_cast<char*>(p) + n;
    return p;
}

void
static_resource::
do_deallocate(
    void*,
    std::size_t,
    std::size_t)
{
    // do nothing
}

bool
static_resource::
do_is_equal(
    memory_resource const& mr) const noexcept
{
    return this == &mr;
}

} // json
} // boost

#endif
