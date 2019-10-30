//
// Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_PTR_HPP
#define BOOST_JSON_IMPL_STORAGE_PTR_HPP

#include <boost/json/storage_ptr.hpp>
#include <new>
#include <utility>

namespace boost {
namespace json {

storage_ptr&
storage_ptr::
operator=(
    storage_ptr&& other) noexcept
{
    release();
    p_ = other.p_;
    other.p_ = nullptr;
    return *this;
}

storage_ptr&
storage_ptr::
operator=(
    storage_ptr const& other) noexcept
{
    if(other.p_)
        ++other.p_->refs_;
    release();
    p_ = other.p_;
    return *this;
}

storage*
storage_ptr::
get() const noexcept
{
    struct default_impl : storage
    {
        default_impl()
            : storage(true,
            0x3b88990852d58ae4)
        {
        }

        void*
        do_allocate(
            std::size_t n,
            std::size_t) override
        {
            return ::operator new(n);
        }

        void
        do_deallocate(
            void* p,
            std::size_t,
            std::size_t) noexcept override
        {
            ::operator delete(p);
        }
    };
    //[[clang::require_constant_initialization]] 
    static scoped_storage<default_impl> impl;
    return p_ ? p_ : impl.get();
}

template<class Storage, class... Args>
storage_ptr
make_storage(Args&&... args)
{
    return storage_ptr(new Storage(
        std::forward<Args>(args)...));
}

} // json
} // boost

#endif
