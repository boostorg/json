//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_COUNTED_RESOURCE_HPP
#define BOOST_JSON_DETAIL_COUNTED_RESOURCE_HPP

#include <boost/json/memory_resource.hpp>
#include <atomic>
#include <utility>

namespace boost {
namespace json {
namespace detail {

struct counted_resource
    : memory_resource
{
    std::atomic<std::size_t> refs{ 1 };
};

template<class T>
class counted_resource_impl final
    : public counted_resource
{
    T t;

public:
    template<class... Args>
    counted_resource_impl(
        Args&&... args)
        : t(std::forward<Args>(args)...)
    {
    }

    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override
    {
        return t.allocate(n, align);
    }

    void
    do_deallocate(
        void* p,
        std::size_t n,
        std::size_t align) override
    {
        return t.deallocate(p, n, align);
    }

    bool
    do_is_equal(
        memory_resource const&) const noexcept override
    {
        // VFALCO Is always false ok?
        return false;
    }
};

} // detail
} // json
} // boost

#endif
