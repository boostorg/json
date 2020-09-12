//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_NULL_RESOURCE_IPP
#define BOOST_JSON_IMPL_NULL_RESOURCE_IPP

#include <boost/json/null_resource.hpp>
#include <boost/json/detail/except.hpp>

BOOST_JSON_NS_BEGIN

void*
null_resource::
do_allocate(
    std::size_t,
    std::size_t)
{
    detail::throw_bad_alloc(
        BOOST_CURRENT_LOCATION);
}

void
null_resource::
do_deallocate(
    void*,
    std::size_t,
    std::size_t)
{
    // do nothing
}

bool
null_resource::
do_is_equal(
    memory_resource const& mr) const noexcept
{
    return this == &mr;
}

BOOST_JSON_NS_END

#endif
