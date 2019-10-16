//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_SERIALIZER_HPP
#define BOOST_JSON_SERIALIZER_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/number.hpp>
#include <boost/json/value.hpp>
#include <boost/json/detail/iterator.hpp>

namespace boost {
namespace json {

class serializer
{
    struct base
    {
        virtual ~base() = default;

        virtual
        bool
        is_done() const noexcept = 0;

        virtual
        std::size_t
        next(char* dest, std::size_t size) = 0;
    };

    class impl : public base
    {
        enum class state
        {
            next,
            init,
            key,
            value,
            literal,
            key_literal,
            string,
            done
        };

        value const& jv_;
        detail::const_iterator it_;
        string_view str_;
        char buf_[number::max_string_chars + 1];
        state state_ = state::init;
        bool last_;

    public:
        inline
        impl(value const& jv);

        inline
        bool
        is_done() const noexcept override;

        inline
        std::size_t
        next(char* dest, std::size_t size) override;
    };

    typename std::aligned_storage<
        sizeof(impl)>::type buf_;

    base&
    get_base() noexcept
    {
        return *reinterpret_cast<
            base*>(&buf_);
    }

    base const&
    get_base() const noexcept
    {
        return *reinterpret_cast<
            base const*>(&buf_);
    }

public:
    BOOST_JSON_DECL
    ~serializer();

    BOOST_JSON_DECL
    explicit
    serializer(value const& jv);

    bool
    is_done() const noexcept
    {
        return get_base().is_done();
    }

    std::size_t
    next(char* dest, std::size_t size)
    {
        return get_base().next(dest, size);
    }
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
