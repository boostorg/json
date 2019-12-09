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

struct default_impl
{
    static
    constexpr
    std::uint64_t
    id = 0x3b88990852d58ae4;

    static
    constexpr
    bool
    need_free = true;

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

storage*
storage_ptr::
get() const noexcept
{
    BOOST_JSON_REQUIRE_CONST_INIT
    static scoped_storage<
        detail::default_impl> ss;
    return p_ ? p_ : &ss.impl_;
}

//----------------------------------------------------------

template<class Storage, class... Args>
storage_ptr
make_storage(Args&&... args)
{
    // If this generates an error, it means that your
    // type `Storage` does not meet the named requirements.
    //
    static_assert(is_storage<Storage>::value,
        "Storage requirements not met");
    return storage_ptr(new 
        detail::storage_impl<Storage>(true,
            std::forward<Args>(args)...));
}

} // json
} // boost

#endif
