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
#include <boost/json/value.hpp>
#include <boost/json/detail/format.hpp>
#include <boost/json/detail/static_stack.hpp>
#include <iosfwd>

namespace boost {
namespace json {

class serializer
{
    enum class state : char;

#ifndef GENERATING_DOCUMENTATION
    // The xsl has problems with anonymous unions
    struct nobj
    {
        object const* po;
        object::const_iterator it;
    };

    struct narr
    {
        array const* pa;
        array::const_iterator it;
    };

    struct node
    {
        union
        {
            value const* pjv;
            nobj obj;
            narr arr;
        };
        state st;

        inline
        explicit
        node(state st_) noexcept;

        inline
        explicit
        node(value const& jv) noexcept;

        inline
        explicit
        node(object const& o) noexcept;

        inline
        explicit
        node(array const& a) noexcept;
    };
#endif

    detail::static_stack<node, 16> stack_;

    string_view str_;
    unsigned char nbuf_;
    char buf_[detail::max_number_chars + 1];

public:
    explicit
    serializer(value const& jv)
    {
        // ensure room for \uXXXX escape plus one
        BOOST_JSON_STATIC_ASSERT(sizeof(buf_) >= 7);
        reset(jv);
    }

    BOOST_JSON_DECL
    bool
    is_done() const noexcept;

    BOOST_JSON_DECL
    void
    reset(value const& jv) noexcept;

    BOOST_JSON_DECL
    std::size_t
    read(char* dest, std::size_t size);
};

BOOST_JSON_DECL
string
to_string(value const& jv);

BOOST_JSON_DECL
std::ostream&
operator<<(
    std::ostream& os,
    value const& jv);

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/serializer.ipp>
#endif

#endif
