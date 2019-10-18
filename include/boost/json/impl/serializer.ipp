//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZER_IPP
#define BOOST_JSON_IMPL_SERIALIZER_IPP

#include <boost/json/serializer.hpp>
#include <boost/static_assert.hpp>

namespace boost {
namespace json {

enum class serializer::state : char
{
    inc,    // increment iterator
    val,    // value

    key1,   // '"'
    key2,   // escaped string
    key3,   // '"'
    key4,   // ':'
    key5,   // literal (ctrl or utf16 escape)

    str1,   // '"'
    str2,   // escaped string
    str3,   // '"'
    str4,   // ','
    str5,   // literal (ctrl or utf16 escape)

    lit,    // literal string ({number}, true, false, null)

    done
};

serializer::
serializer(
    value const& jv)
    : it_(jv)
    , state_(state::val)

{
    // ensure room for \uXXXX escape plus one
    BOOST_STATIC_ASSERT(sizeof(buf_) >= 7);
}

bool
serializer::
is_done() const noexcept
{
    return state_ == state::done;
}

std::size_t
serializer::
next(char* dest, std::size_t size)
{
    auto const p0 = dest;
    auto const p1 = dest + size;
    auto p = p0;
loop:
    switch(state_)
    {
    case state::inc:
    {
        ++it_;
        if(it_ == detail::end)
        {
            state_ = state::done;
            goto loop;
        }
        auto const& e = *it_;
        if(e.end)
        {
            if(e.value.is_object())
            {
                str_ = {"},", e.last ? 1U : 2U};
                state_ = state::lit;
                goto loop;
            }
            BOOST_ASSERT(e.value.is_array());
            str_ = {"],", e.last ? 1U : 2U};
            state_ = state::lit;
            goto loop;
        }
        if(e.has_key)
        {
            key_ = it_->key;
            state_ = state::key1;
            goto loop;
        }
        BOOST_FALLTHROUGH;
    }

    case state::val:
    {
        auto const& e = *it_;
        switch(e.value.kind())
        {
        case kind::object:
            if(p >= p1)
                goto finish;
            *p++ = '{';
            state_ = state::inc;
            goto loop;
            break;

        case kind::array:
            if(p >= p1)
                goto finish;
            *p++ = '[';
            state_ = state::inc;
            goto loop;

        case kind::string:
            str_ = *e.value.if_string();
            state_ = state::str1;
            goto loop;

        case kind::number:
            str_ = e.value.if_number()->print(
                buf_, sizeof(buf_));
            if(! e.last)
            {
                buf_[str_.size()] = ',';
                str_ = {str_.data(),
                    str_.size() + 1};
            }
            state_ = state::lit;
            goto loop;

        case kind::boolean:
            if(*e.value.if_bool())
                str_ = { "true,",
                    e.last ? 4U : 5U };
            else
                str_ = { "false,",
                    e.last ? 5U : 6U };
            state_ = state::lit;
            goto loop;

        case kind::null:
            str_ = { "null,",
                e.last ? 4U : 5U };
            state_ = state::lit;
            goto loop;
        }
    }

    //---

    case state::key1:
        if(p >= p1)
            goto finish;
        *p++ = '\"';
        state_ = state::key2;
        BOOST_FALLTHROUGH;

    case state::key2:
    {
        auto const n =
            key_.copy(p, p1 - p);
        p += n;
        if(n < key_.size())
        {
            BOOST_ASSERT(p >= p1);
            key_ = key_.substr(n);
            goto finish;
        }
        state_ = state::key3;
        BOOST_FALLTHROUGH;
    }

    case state::key3:
        if(p >= p1)
            goto finish;
        *p++ = '\"';
        state_ = state::key4;
        BOOST_FALLTHROUGH;

    case state::key4:
        if(p >= p1)
            goto finish;
        *p++ = ':';
        state_ = state::val;
        goto loop;

    case state::key5:
        if(p >= p1)
            goto finish;
        break;

    //---

    case state::str1:
        if(p >= p1)
            goto finish;
        *p++ = '\"';
        state_ = state::str2;
        BOOST_FALLTHROUGH;

    case state::str2:
    {
        auto s = str_.data();
        auto const s1 =
            s + str_.size();
        while(p < p1 && s < s1)
        {
            auto ch = *s++;
            if(ch == '\\')
            {
                if(p + 2 <= p1)
                {
                    *p++ = '\\';
                    *p++ = '\\';
                    continue;
                }
                str_ = str_.substr(
                    s - str_.data());
                buf_[1] = '\\';
                buf_[0] = '\\';
                nbuf_ = 2;
                state_ = state::str5;
                goto loop;
            }
            else if(ch == '\"')
            {
                if(p + 2 <= p1)
                {
                    *p++ = '\\';
                    *p++ = '\"';
                    continue;
                }
                str_ = str_.substr(
                    s - str_.data());
                buf_[1] = '\\';
                buf_[0] = '\"';
                nbuf_ = 2;
                state_ = state::str5;
                goto loop;
            }
            else if(ch >= 32)
            {
                *p++ = ch;
                continue;
            }
            else
            {
                str_ = str_.substr(
                    s - str_.data());
                buf_[5] = '\\';
                buf_[4] = 'u';
                buf_[3] = '0';
                buf_[2] = '0';
                buf_[1] = "0123456789abcdef"[ch >>  4];
                buf_[0] = "0123456789abcdef"[ch &  15];
                nbuf_ = 6;
                state_ = state::str5;
                goto loop;
            }
        }
        if(s < s1)
        {
            str_ = str_.substr(
                s - str_.data());
            goto finish;
        }
        state_ = state::str3;
        BOOST_FALLTHROUGH;
    }

    case state::str3:
        if(p >= p1)
            goto finish;
        *p++ = '\"';
        if(it_->last)
        {
            state_ = state::inc;
            goto loop;
        }
        state_ = state::str4;
        BOOST_FALLTHROUGH;

    case state::str4:
        if(p >= p1)
            goto finish;
        *p++ = ',';
        state_ = state::inc;
        goto loop;

    case state::str5:
        while(p < p1)
        {
            *p++ = buf_[--nbuf_];
            if(! nbuf_)
            {
                state_ = state::str2;
                goto loop;
            }
        }
        goto finish;

    //---

    case state::lit:
    {
        auto const n =
            str_.copy(p, p1 - p);
        p += n;
        if(n == str_.size())
        {
            state_ = state::inc;
            goto loop;
        }
        str_ = str_.substr(n);
        BOOST_ASSERT(p >= p1);
        goto finish;
    }

    case state::done:
        goto finish;
    }
finish:
    return p - p0;
}

} // json
} // boost

#endif
