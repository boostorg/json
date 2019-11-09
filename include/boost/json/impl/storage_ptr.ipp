//
// Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_PTR_IPP
#define BOOST_JSON_IMPL_STORAGE_PTR_IPP

#include <boost/json/storage_ptr.hpp>

namespace boost {
namespace json {
namespace detail {

struct default_storage_impl
{
    static
    constexpr
    unsigned long long id()
    { 
        return 0x3b88990852d58ae4;
    }

    static
    constexpr
    bool need_free()
    {
        return true;
    }

    void*
    allocate(
        std::size_t n,
        std::size_t)
    {
        return ::operator new(n);
    }

    void
    deallocate(
        void* p,
        std::size_t,
        std::size_t) noexcept
    {
        ::operator delete(p);
    }
};

} // detail

//----------------------------------------------------------

storage_ptr::
storage_ptr() noexcept
    : p_([]
        {
        #ifdef __clang__
            [[clang::require_constant_initialization]] 
        #endif
            static scoped_storage<
                detail::default_storage_impl> impl;
            return impl.get();
        }())
{
}

} // json
} // boost

#endif
