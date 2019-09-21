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
#include <boost/json/iterator.hpp>
#include <boost/json/number.hpp>
#include <boost/json/value.hpp>
#include <boost/asio/buffer.hpp>

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
        next(boost::asio::mutable_buffer) = 0;
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
        BOOST_JSON_DECL
        impl(value const& jv);

        BOOST_JSON_DECL
        bool
        is_done() const noexcept override;

        BOOST_JSON_DECL
        std::size_t
        next(boost::asio::mutable_buffer) override;

    private:
        BOOST_JSON_DECL
        void
        append(char c,
            boost::asio::mutable_buffer& b);

        BOOST_JSON_DECL
        void
        append(
            char const* s, std::size_t n,
            boost::asio::mutable_buffer& b);
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

    template<class MutableBufferSequence>
    std::size_t
    next(MutableBufferSequence const& buffers)
    {
        std::size_t bytes_transferred = 0;
        for(auto it = boost::asio::buffer_sequence_begin(buffers),
            end = boost::asio::buffer_sequence_end(buffers);
            it != end; ++it)
            bytes_transferred += next(
                boost::asio::mutable_buffer(*it));
        return bytes_transferred;
    }

    bool
    is_done() const noexcept
    {
        return get_base().is_done();
    }

    std::size_t
    next(boost::asio::mutable_buffer buffer)
    {
        return get_base().next(buffer);
    }
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
