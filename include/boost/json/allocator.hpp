//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_ALLOCATOR_HPP
#define BOOST_JSON_ALLOCATOR_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage.hpp>
#include <type_traits>

namespace boost {
namespace beast {
namespace json {

template<typename T>
class allocator
{
    storage_ptr sp_;

    template<typename U>
    friend class allocator;

public:
    using pointer = T*;
    using reference = T&;
    using const_pointer = T const*;
    using const_reference = T const&;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<typename U>
    struct rebind
    {
        using other = allocator<U>;
    };

    allocator(allocator const&) = default;
    allocator& operator=(allocator const&) = default;

    allocator();

    allocator(allocator&&) noexcept;

    allocator& operator=(allocator&&) noexcept;

    template<typename U>
    allocator(allocator<U> const& other) noexcept;

    explicit
    allocator(storage_ptr sp) noexcept;

    T*
    allocate(size_t n);

    void
    deallocate(T* t, size_t n) noexcept;

    bool
    operator==(allocator const&) const noexcept;

    bool
    operator!=(allocator const&) const noexcept;

    storage_ptr
    get_storage() const & noexcept
    {
        return sp_;
    }

    storage_ptr
    get_storage() && noexcept
    {
        return std::move(sp_);
    }
};

} // json
} // beast
} // boost

#include <boost/json/impl/allocator.hpp>

#endif
