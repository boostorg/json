//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_PARSER_HPP
#define BOOST_JSON_PARSER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/basic_parser.hpp>
#include <boost/json/storage.hpp>
#include <boost/json/value.hpp>
#include <boost/json/string.hpp>
#include <boost/json/detail/except.hpp>
#include <boost/json/detail/static_stack.hpp>
#include <boost/json/detail/string.hpp>
#include <new>
#include <string>
#include <type_traits>
#include <stddef.h>

namespace boost {
namespace json {

class parser final
    : public basic_parser
{
    struct level
    {
        std::int32_t size;
        bool obj;
        char pad[3];
    };

    class stack
    {
        storage_ptr sp_;  // for the values
        storage_ptr ssp_; // for the stack
        storage_ptr vsp_;
        std::size_t size_ = 0;
        std::size_t capacity_ = 0;
        char* base_ = nullptr;

        size_t
        capacity() const noexcept
        {
            return capacity_;
        }

        static
        std::size_t
        alignup(std::size_t n) noexcept
        {
            return sizeof(max_align_t) *
                ((n + sizeof(max_align_t) - 1) /
                 sizeof(max_align_t));
        }

    public:
        stack() = default;

        ~stack()
        {
            if(base_)
                sp_->deallocate(
                    base_, capacity_);
        }

        explicit
        stack(storage_ptr sp) noexcept
            : sp_(std::move(sp))
        {
        }

        bool
        empty() const noexcept
        {
            return size_ == 0;
        }

        template<class Arg>
        value&
        emplace_value(Arg&& arg)
        {
            BOOST_JSON_ASSERT(
                size_ >= sizeof(value));
            auto& v = *::new(base_ + size_
                - alignup(sizeof(value))) value(
                std::forward<Arg>(arg), sp_);
            return v;
        }

        template<class Arg>
        object::value_type&
        emplace_pair(Arg&& arg)
        {
            std::size_t len;
            pop_impl(len);
            size_ -= alignup(len) - len;
            auto key = pop_string(len);
            auto const n = sizeof(object::value_type);
            // size for n was placeheld
            BOOST_JSON_ASSERT(size_ >= n);
            auto& kvp = *::new(
                base_ + size_ - alignup(n))
                object::value_type(key,
                    std::forward<Arg>(arg), sp_);
            return kvp;
        }

        void
        placeholder(
            std::size_t bytes)
        {
            BOOST_JSON_ASSERT(
                alignup(bytes) == bytes);
            BOOST_JSON_ASSERT(
                alignup(size_) == size_);
            prepare(bytes);
            size_ += bytes;
        }

        void
        unreserve(
            std::size_t bytes)
        {
            BOOST_JSON_ASSERT(
                alignup(bytes) == bytes);
            BOOST_JSON_ASSERT(
                alignup(size_) == size_);
            BOOST_JSON_ASSERT(
                bytes <= size_);
            size_ -= bytes;
        }

        void
        align()
        {
            auto const n = alignup(size_);
            if(n > capacity_)
                grow(n - size_);
            size_ = n;
        }

        void
        push(std::size_t u)
        {
            push_impl(u);
        }

        void
        push(level lev)
        {
            push_impl(lev);
        }

        void
        push(string_view s)
        {
            prepare(s.size());
            std::memcpy(base_ + size_,
                s.data(), s.size());
            size_ += s.size();
        }

        void
        pop(level& lev)
        {
            pop_impl(lev);
        }

        unchecked_array
        pop_array(size_type size) noexcept
        {
            BOOST_JSON_ASSERT(
                alignup(size_) == size_);
            if(size == 0)
                return { nullptr, 0, sp_ };
            auto const n =
                sizeof(value) * size;
            BOOST_JSON_ASSERT(
                alignup(n) == n);
            BOOST_JSON_ASSERT(n <= size_);
            size_ -= n;
            return { reinterpret_cast<
                value*>(base_ + size_),
                    size, sp_ };
        }

        unchecked_object
        pop_object(size_type size) noexcept
        {
            BOOST_JSON_ASSERT(
                alignup(size_) == size_);
            if(size == 0)
                return { nullptr, 0, sp_ };
            auto const n = sizeof(
                object::value_type) * size;
            BOOST_JSON_ASSERT(
                alignup(n) == n);
            BOOST_JSON_ASSERT(n <= size_);
            size_ -= n;
            return { reinterpret_cast<
                object::value_type*>(base_ + size_),
                    size, sp_ };
        }

        string_view
        pop_string(std::size_t len) noexcept
        {
            BOOST_JSON_ASSERT(len <= size_);
            size_ -= len;
            return { base_ + size_, len };
        }

    private:
        static
        constexpr
        std::size_t
        min_capacity_ = 256;

        static
        constexpr
        std::size_t
        max_size_ = std::size_t(-1);

        void
        prepare(std::size_t n)
        {
            if(n > capacity_ - size_)
                grow(n);
        }

        void
        grow(std::size_t n)
        {
            if(n > max_size_ - size_)
                BOOST_JSON_THROW(
                    detail::stack_overflow_exception());
            auto new_capacity = size_ + n;
            if( new_capacity < min_capacity_)
                new_capacity = min_capacity_;
            // 2x growth
            auto const hint = (capacity_ * 2) & ~1;
            if( new_capacity < hint)
                new_capacity = hint;
            auto base = reinterpret_cast<
                char*>(ssp_->allocate(new_capacity));
            if(base_)
            {
                std::memcpy(base, base_, size_);
                ssp_->deallocate(base_, size_);
            }
            base_ = base;
            capacity_ = new_capacity;
        }

        template<class T>
        void
        push_impl(T t)
        {
            BOOST_JSON_ASSERT((sizeof(T) %
                sizeof(max_align_t)) == 0);
            BOOST_JSON_ASSERT((size_ %
                sizeof(max_align_t)) == 0);
            auto const n = sizeof(T);
            if(n > capacity_ - size_)
                grow(n);
            ::new(base_ + size_) T(t);
            size_ += n;
        }

        template<class T>
        void
        pop_impl(T& t) noexcept
        {
            BOOST_JSON_ASSERT((sizeof(T) %
                sizeof(max_align_t)) == 0);
            BOOST_JSON_ASSERT((size_ %
                sizeof(max_align_t)) == 0);
            auto const n = sizeof(T);
            BOOST_JSON_ASSERT(size_ >= n);
            size_ -= n;
            std::memcpy(
                &t, base_ + size_, sizeof(T));
        }
    };

    stack st_;
    value jv_;
    level lev_;
    size_type key_ = 0; // key len
    size_type str_ = 0; // str len

public:
    BOOST_JSON_DECL
    virtual
    ~parser();

    BOOST_JSON_DECL
    parser();

    BOOST_JSON_DECL
    parser(storage_ptr sp);

    BOOST_JSON_DECL
    value const&
    get() const noexcept;

    BOOST_JSON_DECL
    value
    release() noexcept;

private:
    BOOST_JSON_DECL
    void
    destroy();

    BOOST_JSON_DECL
    void
    on_document_begin(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_document_end(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_object_begin(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_object_end(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_array_begin(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_array_end(
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_key_part(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_key(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_string_part(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_string(
        string_view s,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_int64(
        int64_t i,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_uint64(
        uint64_t u,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_double(
        double d,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_bool(
        bool b,
        error_code& ec) override;

    BOOST_JSON_DECL
    void
    on_null(error_code&) override;
};

//----------------------------------------------------------

BOOST_JSON_DECL
value
parse(
    string_view s,
    storage_ptr sp,
    error_code& ec);

inline
value
parse(
    string_view s,
    error_code& ec)
{
    return parse(s,
        storage_ptr{}, ec);
}

BOOST_JSON_DECL
value
parse(
    string_view s,
    storage_ptr sp);

inline
value
parse(string_view s)
{
    return parse(s, storage_ptr{});
}

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/parser.ipp>
#endif

#endif
