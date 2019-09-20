//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_SERIALIZER_HPP
#define BOOST_JSON_SERIALIZER_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/json/iterator.hpp>
#include <boost/json/number.hpp>
#include <boost/json/value.hpp>
#include <boost/asio/buffer.hpp>

namespace boost {
namespace beast {
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
        next(net::mutable_buffer) = 0;
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
        const_iterator it_;
        string_view str_;
        char buf_[number::max_string_chars + 1];
        state state_ = state::init;
        bool last_;

    public:
        BOOST_BEAST_DECL
        impl(value const& jv);

        BOOST_BEAST_DECL
        bool
        is_done() const noexcept override;

        BOOST_BEAST_DECL
        std::size_t
        next(net::mutable_buffer) override;

    private:
        BOOST_BEAST_DECL
        void
        append(char c,
            net::mutable_buffer& b);

        BOOST_BEAST_DECL
        void
        append(
            char const* s, std::size_t n,
            net::mutable_buffer& b);
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
    BOOST_BEAST_DECL
    ~serializer();

    BOOST_BEAST_DECL
    explicit
    serializer(value const& jv);

    template<class MutableBufferSequence>
    std::size_t
    next(MutableBufferSequence const& buffers)
    {
        std::size_t bytes_transferred = 0;
        for(auto it = net::buffer_sequence_begin(buffers),
            end = net::buffer_sequence_end(buffers);
            it != end; ++it)
            bytes_transferred += next(
                net::mutable_buffer(*it));
        return bytes_transferred;
    }

    bool
    is_done() const noexcept
    {
        return get_base().is_done();
    }

    std::size_t
    next(net::mutable_buffer buffer)
    {
        return get_base().next(buffer);
    }
};

} // json
} // beast
} // boost

#ifdef BOOST_BEAST_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
