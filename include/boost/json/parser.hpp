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
#include <boost/json/detail/static_stack.hpp>
#include <boost/json/detail/string.hpp>
#include <new>
#include <string>
#include <type_traits>

namespace boost {
namespace json {

class parser final
    : public basic_parser
{
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

        template<class Arg>
        value&
        emplace_value(Arg&& arg)
        {
            auto const n = sizeof(value);
            if(n > capacity_ - size_)
                grow(n);
            auto& v = *::new(base_ + size_) value(
                std::forward<Arg>(arg), sp_);
            size_ += n;
            return v;
        }

        value&
        top_value() noexcept
        {
            return *reinterpret_cast<
                value*>(base_ + size_ -
                    sizeof(value));
        }

        void
        push(size_type u)
        {
            push_impl(u);
        }

        void
        push(string_view s)
        {
            if(s.size() > max_size_ - size_)
                BOOST_JSON_THROW(
                    std::length_error(
                        "size > max_size"));
            if(s.size() > capacity_ - size_)
                grow(s.size());
            std::memcpy(base_,
                s.data(), s.size());
            size_ += s.size();
        }

        void
        pop(size_type& u)
        {
            pop_impl(u);
        }

        unchecked_array
        pop_array(size_type size) noexcept
        {
            auto const n = sizeof(value) * size;
            BOOST_JSON_ASSERT(n <= size_);
            size_ -= n;
            return { reinterpret_cast<
                value*>(base_ + size_),
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
        grow(std::size_t n)
        {
            if(n > max_size_ - size_)
                BOOST_JSON_THROW(
                    std::length_error(
                        "size > max_size"));
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
            BOOST_JSON_ASSERT(
                size_ >= sizeof(T));
            size_ -= sizeof(T);
            std::memcpy(
                &t, base_ + size_, sizeof(T));
        }
    };

    stack st_;
    value jv_;
    size_type size_ = 0;
    size_type len_ = 0; // string

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
