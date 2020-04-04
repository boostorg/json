//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DEFAULT_RESOURCE_HPP
#define BOOST_JSON_DEFAULT_RESOURCE_HPP

#include <boost/json/config.hpp>
#include <new>

namespace boost {
namespace json {
namespace detail {

/** A simple memory resource that uses operator new and delete.
*/
class default_resource final
    : public memory_resource
{
public:
    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override
    {
        return ::operator new(n);
    }

    void
    do_deallocate(
        void* p,
        std::size_t,
        std::size_t) override
    {
        ::operator delete(p);
    }

    bool
    do_is_equal(
        memory_resource const& mr) const noexcept
    {
        return dynamic_cast<
            default_resource const*>(&mr) != nullptr;
    }
};

} // detail
} // json
} // boost

#endif
