//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_IPP
#define BOOST_JSON_IMPL_STORAGE_IPP

#include <boost/json/storage.hpp>
#include <boost/assert.hpp>
#include <memory>

namespace boost {
namespace json {

//----------------------------------------------------------

namespace detail {

storage_ptr const&
global_storage() noexcept
{
    struct builtin : storage
    {
        void*
        allocate(
            std::size_t n,
            std::size_t) override
        {
            return std::allocator<
                char>().allocate(n);
        }

        void
        deallocate(
            void* p,
            std::size_t n,
            std::size_t) noexcept override
        {
            std::allocator<
                char>().deallocate(
                static_cast<char*>(p), n);
        }

        bool
        is_equal(storage const& other)
            const noexcept override
        {
            BOOST_ASSERT(dynamic_cast<
                builtin const*>(&other) == nullptr);
            return false;
        }
    };
    static storage_ptr const sp =
        make_storage<builtin>();
    return sp;
}

inline
storage_ptr&
raw_default_storage() noexcept
{
    static storage_ptr sp =
        global_storage();
    return sp;
}

} // detail

storage_ptr
default_storage() noexcept
{
    return detail::raw_default_storage();
}

void
default_storage(storage_ptr sp) noexcept
{
    detail::raw_default_storage() = std::move(sp);
}

//----------------------------------------------------------

void
storage::
addref() noexcept
{
    ++refs_;
}

void
storage::
release() noexcept
{
    if(--refs_ > 0)
        return;
    delete this;
}

storage::
storage() noexcept
    : refs_(1)
{
}

} // json
} // boost

#endif
