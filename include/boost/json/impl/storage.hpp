//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_HPP
#define BOOST_JSON_IMPL_STORAGE_HPP

#include <utility>

namespace boost {
namespace json {
namespace detail {

template<class T>
struct storage_impl : storage
{
    T t;

    template<class... Args>
    constexpr
    explicit
    storage_impl(
        bool counted,
        Args&&... args)
        : storage(
            T::id,
            T::need_free,
            counted)
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
} // json
} // boost

#endif
