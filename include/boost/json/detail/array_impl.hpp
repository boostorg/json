//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_ARRAY_IMPL_HPP
#define BOOST_JSON_DETAIL_ARRAY_IMPL_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <cstdint>
#include <cstring>

namespace boost {
namespace json {

class value;

namespace detail {

class array_impl
{
    struct table
    {
        std::uint32_t size;
        std::uint32_t capacity;
    };

    table* tab_ = nullptr;

public:
    static
    constexpr
    unsigned long min_capacity = 16;

    array_impl() = default;
    array_impl(array_impl const&) = default;
    array_impl& operator=(
        array_impl const&) = default;

    inline
    array_impl(
        std::size_t capacity,
        storage_ptr const& sp);

    inline
    array_impl&
    operator=(
        array_impl&& other) noexcept;

    static
    constexpr
    std::size_t
    max_size() noexcept
    {
        return BOOST_JSON_MAX_ARRAY_SIZE;
    }

    value*
    data() const noexcept
    {
        return tab_ ?
            reinterpret_cast<
                value*>(tab_ + 1) :
            nullptr;
    }

    std::size_t
    size() const noexcept
    {
        return tab_ ?
            tab_->size : 0;
    }

    void
    size(std::size_t n) noexcept
    {
        if(tab_)
            tab_->size = static_cast<
                std::uint32_t>(n);
        else
            BOOST_ASSERT(n == 0);
    }

    std::size_t
    capacity() const noexcept
    {
        return tab_ ?
            tab_->capacity : 0;
    }

    inline
    std::size_t
    index_of(value const*) const noexcept;

    inline
    array_impl(
        array_impl&& other) noexcept;

    inline
    void
    swap(array_impl& rhs) noexcept;

    inline
    void
    destroy_impl(storage_ptr const& sp) noexcept;

    BOOST_JSON_DECL
    void
    destroy(storage_ptr const& sp) noexcept;
};

//----------------------------------------------------------

class unchecked_array
{
    value* data_;
    std::size_t size_;
    storage_ptr const& sp_;

public:
    inline
    ~unchecked_array();

    unchecked_array(
        value* data,
        std::size_t size,
        storage_ptr const& sp) noexcept
        : data_(data)
        , size_(size)
        , sp_(sp)
    {
    }

    unchecked_array(
        unchecked_array&& other) noexcept
        : data_(other.data_)
        , size_(other.size_)
        , sp_(other.sp_)
    {
        other.data_ = nullptr;
    }

    storage_ptr const&
    storage() const noexcept
    {
        return sp_;
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

    inline
    void
    relocate(value* dest) noexcept;
};

} // detail
} // json
} // boost

// includes are at the bottom of <boost/json/value.hpp>
 
#endif
