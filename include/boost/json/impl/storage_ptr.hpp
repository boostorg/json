//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_PTR_HPP
#define BOOST_JSON_IMPL_STORAGE_PTR_HPP

#include <new>
#include <utility>

namespace boost {
namespace json {

//----------------------------------------------------------

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

template<class T>
struct counted_storage_impl : storage
{
    T t;

    template<class... Args>
    constexpr
    explicit
    counted_storage_impl(Args&&... args)
        : storage(
            T::id(),
            T::need_free(),
            true)
        , t(std::forward<Args>(args)...)
    {
    }

    void*
    allocate(
        std::size_t n,
        std::size_t align) override
    {
        return t.allocate(n, align);
    }

    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t align) noexcept override
    {
        t.deallocate(p, n, align);
    }
};

} // detail

//----------------------------------------------------------

storage*
storage_ptr::
get() const noexcept
{
#ifdef __clang__
    [[clang::require_constant_initialization]] 
#endif
    static scoped_storage<
        detail::default_storage_impl> impl;
    return p_ ? p_ : impl.get();
}

//----------------------------------------------------------

template<class Storage, class... Args>
storage_ptr
make_storage(Args&&... args)
{
    return storage_ptr(new 
        detail::counted_storage_impl<Storage>(
            std::forward<Args>(args)...));
}

} // json
} // boost

#endif
