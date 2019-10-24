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
#include <ostream>

namespace boost {
namespace json {

enum class serializer::state : char
{
    inc,    // increment iterator
    val,    // value

    str1,   // '"'
    str2,   // escaped string
    str3,   // '"'
    str4,   // ':' or ','
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
    BOOST_JSON_STATIC_ASSERT(sizeof(buf_) >= 7);
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
        if(it_ == detail::const_iterator::end())
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
            BOOST_JSON_ASSERT(e.value.is_array());
            str_ = {"],", e.last ? 1U : 2U};
            state_ = state::lit;
            goto loop;
        }
        if(e.has_key)
        {
            key_ = true;
            str_ = it_->key;
            state_ = state::str1;
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
            key_ = false;
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
            else if(static_cast<
                unsigned char>(ch) >= 32)
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
                static constexpr char hex[] =
                    "0123456789abcdef";
                buf_[1] = hex[static_cast<
                    unsigned char>(ch) >> 4];
                buf_[0] = hex[static_cast<
                    unsigned char>(ch) & 15];
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
        if(it_->last && ! key_)
        {
            state_ = state::inc;
            goto loop;
        }
        state_ = state::str4;
        BOOST_FALLTHROUGH;

    case state::str4:
        if(p >= p1)
            goto finish;
        if(key_)
        {
            *p++ = ':';
            state_ = state::val;
        }
        else
        {
            *p++ = ',';
            state_ = state::inc;
        }
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
        BOOST_JSON_ASSERT(p >= p1);
        goto finish;
    }

    case state::done:
        goto finish;
    }
finish:
    return p - p0;
}

std::string
to_string(
    json::value const& jv)
{
    std::string s;
    std::size_t len = 0;
    s.resize(1024);
    json::serializer p(jv);
    for(;;)
    {
        auto const used = p.next(
            &s[len], s.size() - len);
        len += used;
        s.resize(len);
        if(p.is_done())
            break;
        s.resize((len * 3) / 2);
    }
    s.shrink_to_fit();
    return s;
}


std::ostream&
operator<<(std::ostream& os, value const& jv)
{
    serializer sr(jv);
    while(! sr.is_done())
    {
        char buf[4096];
        auto const n =
            sr.next(buf, sizeof(buf));
        os.write(buf, n);
    }
    return os;
}

} // json
} // boost

#endif
