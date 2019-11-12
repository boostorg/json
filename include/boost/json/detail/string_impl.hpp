//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STRING_IMPL_HPP
#define BOOST_JSON_DETAIL_STRING_IMPL_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <algorithm>
#include <cstdint>
#include <iterator>

namespace boost {
namespace json {
namespace detail {

struct string_impl
{
    static constexpr std::size_t mask_ = 0x0f;

    std::uint32_t size_;
    std::uint32_t capacity_;

    union
    {
        char* p;
        char buf[20]; // SBO
    };

    static
    constexpr
    std::size_t
    max_size() noexcept
    {
        return BOOST_JSON_MAX_STRING_SIZE;
    }

    string_impl() noexcept
        : size_(0)
        , capacity_(sizeof(buf) - 1)
    {
        buf[0] = 0;
    }

    template<class InputIt>
    string_impl(
        InputIt first,
        InputIt last,
        storage_ptr const& sp,
        std::random_access_iterator_tag)
        : string_impl(last - first, sp)
    {
        std::copy(
            first, last, data());
        data()[size()] = 0;
    }

    template<class InputIt>
    string_impl(
        InputIt first,
        InputIt last,
        storage_ptr const& sp,
        std::input_iterator_tag)
        : string_impl()
    {
        struct undo
        {
            string_impl& s;
            storage_ptr const& sp;
            bool commit;

            ~undo()
            {
                if(! commit)
                    s.destroy(sp);
            }
        };

        undo u{*this, sp, false};
        auto dest = data();
        size(1);
        *dest++ = *first++;
        while(first != last)
        {
            if(size() < capacity())
                size(size() + 1);
            else
                dest = append(1, sp);
            *dest++ = *first++;
        }
        *dest = 0;
        u.commit = true;
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

    std::size_t
    capacity() const noexcept
    {
        return capacity_;
    }

    void
    size(std::size_t n)
    {
        size_ = static_cast<
            std::uint32_t>(n);
    }

    BOOST_JSON_DECL
    string_impl(
        std::size_t new_size,
        storage_ptr const& sp);

    BOOST_JSON_DECL
    static
    std::uint32_t
    growth(
        std::size_t new_size,
        std::size_t capacity);

    BOOST_JSON_DECL
    void
    destroy(
        storage_ptr const& sp) noexcept;

    BOOST_JSON_DECL
    char*
    assign(
        std::size_t new_size,
        storage_ptr const& sp);

    BOOST_JSON_DECL
    char*
    append(
        std::size_t n,
        storage_ptr const& sp);

    BOOST_JSON_DECL
    char*
    insert(
        std::size_t pos,
        std::size_t n,
        storage_ptr const& sp);

    BOOST_JSON_DECL
    void
    unalloc(storage_ptr const& sp) noexcept;

    bool
    in_sbo() const noexcept
    {
        return capacity_ < sizeof(buf);
    }

    void
    term(std::size_t n) noexcept
    {
        size(n);
        data()[size_] = 0;
    }

    char*
    data() noexcept
    {
        if(in_sbo())
            return buf;
        return p;
    }

    char const*
    data() const noexcept
    {
        if(in_sbo())
            return buf;
        return p;
    }

    char*
    end() noexcept
    {
        return data() + size_;
    }

    char const*
    end() const noexcept
    {
        return data() + size_;
    }
};

} // detail
} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/string_impl.ipp>
#endif

#endif
