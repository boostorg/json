//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_OBJECT_IMPL_HPP
#define BOOST_JSON_DETAIL_OBJECT_IMPL_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/string_view.hpp>
#include <cstddef>
#include <cstring>
#include <type_traits>

BOOST_JSON_NS_BEGIN

class value;
class key_value_pair;

namespace detail {

class unchecked_object;

class object_impl
{
    using index_t = std::uint32_t;
    static index_t const null_index =
        std::uint32_t(-1);
    using bucket_size_array = const std::size_t[34];

    BOOST_JSON_DECL
    void
    do_destroy(storage_ptr const& sp) noexcept;

public:
    using value_type =
        key_value_pair;

    object_impl() = default;

    inline
    object_impl(
        std::size_t capacity,
        std::size_t prime_index,
        std::uintptr_t salt,
        storage_ptr const& sp);

    inline
    object_impl(object_impl&& other) noexcept;

    void
    destroy(storage_ptr const& sp) noexcept
    {
        if( tab_ == nullptr ||
            sp.is_not_counted_and_deallocate_is_trivial())
            return;
        do_destroy(sp);
    }

    static
    constexpr
    std::size_t
    max_size() noexcept;

    std::size_t
    size() const noexcept
    {
        return tab_ ? tab_->size : 0;
    }

    std::size_t
    capacity() const noexcept
    {
        return tab_ ? tab_->capacity : 0;
    }

    std::uintptr_t
    salt() const noexcept
    {
        return tab_ ? tab_->salt :
            reinterpret_cast<std::uintptr_t>(this);
    }

    inline
    value_type*
    begin() const noexcept;

    inline
    value_type*
    end() const noexcept;

    inline
    value_type&
    get(index_t i) const noexcept;

    inline
    index_t
    index_of(value_type const& p) const noexcept;

    inline
    void
    clear() noexcept;

    inline
    void
    grow(std::size_t n) noexcept
    {
        if(n == 0)
            return;
        BOOST_ASSERT(
            n <= capacity() - size());
        tab_->size += n;
    }

    inline
    void
    shrink(std::size_t n) noexcept
    {
        if(n == 0)
            return;
        BOOST_ASSERT(n <= size());
        tab_->size -= n;
    }

    template<bool NeedDestroy>
    void
    build(unchecked_object&& uo) noexcept;

    inline
    void
    rebuild() noexcept;

    inline
    void
    remove(
        index_t& head,
        value_type& p) noexcept;

    static
    inline
    bucket_size_array&
    bucket_sizes() noexcept;

    static
    inline
    std::size_t
    bucket_index(
        std::size_t hash,
        std::size_t index) noexcept;

    inline
    index_t&
    bucket(std::size_t hash) const noexcept;

    inline
    index_t&
    bucket(string_view key) const noexcept;

    static
    inline
    index_t&
    next(value_type& e) noexcept;

    static
    inline
    index_t
    next(value_type const& e) noexcept;

    inline
    void
    swap(object_impl& rhs) noexcept;

    inline
    std::size_t
    digest(string_view key) const noexcept;

private:
    std::size_t
    buckets() const noexcept
    {
        return capacity();
    }

    inline
    index_t*
    bucket_begin() const noexcept;

    // VFALCO Don't we need to align this?
    struct
        //alignas(alignof(key_value_pair))
        table
    {
        std::size_t size;
        std::size_t const capacity;
        std::size_t const prime_index;
        std::uintptr_t const salt;
    };

    table* tab_ = nullptr;
};

//----------------------------------------------------------

class unchecked_object
{
    // each element is two values,
    // first one is a string key,
    // second one is the value.
    value* data_;
    std::size_t size_;
    storage_ptr const& sp_;

public:
    inline
    ~unchecked_object();

    unchecked_object(
        value* data,
        std::size_t size, // # of kv-pairs
        storage_ptr const& sp) noexcept
        : data_(data)
        , size_(size)
        , sp_(sp)
    {
    }

    unchecked_object(
        unchecked_object&& other) noexcept
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

    value*
    release() noexcept
    {
        auto const data = data_;
        data_ = nullptr;
        return data;
    }
};

//----------------------------------------------------------

BOOST_JSON_DECL
void
destroy(
    key_value_pair* p,
    std::size_t n) noexcept;

} // detail
BOOST_JSON_NS_END

// includes are at the bottom of <boost/json/value.hpp>

#endif
