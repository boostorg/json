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
    val,    // initialize value
    obj0,   // begin object
    obj1,   // key
    obj2,   // ':'
    obj3,   // end object
    arr0,   // begin array
    arr1,   // array
    arr2,   // end array
    str1,   // '"'
    str2,   // escaped string
    str3,   // '"'
    str4,   // literal (ctrl or utf16 escape)
    lit,    // literal string ({number}, true, false, null)
    done
};

//----------------------------------------------------------

serializer::
node::
node(state st_) noexcept
{
    st = st_;
}

serializer::
node::
node(value const& jv) noexcept
{
    pjv = &jv;
    st = state::val;
}

serializer::
node::
node(object const& o) noexcept
{
    obj.po = &o;
    ::new(&obj.it) object::
        const_iterator(o.begin());
    st = state::obj0;
}

serializer::
node::
node(array const& a) noexcept
{
    arr.pa = &a;
    ::new(&arr.it) array::
        const_iterator(a.begin());
    st = state::arr0;
}

//----------------------------------------------------------

void
serializer::
reset(value const& jv) noexcept
{
    stack_.clear();
    stack_.emplace_front(state::done);
    stack_.emplace_front(jv);
}

bool
serializer::
is_done() const noexcept
{
    return stack_.front().st == state::done;
}

std::size_t
serializer::
next(char* dest, std::size_t size)
{
    static constexpr char hex[] = "0123456789abcdef";
    static constexpr char esc[] =
        "uuuuuuuubtnufruuuuuuuuuuuuuuuuuu"
        "\0\0\"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\\\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
    auto const dist =
        []( char const* first,
            char const* last)
        {
            return static_cast<
                std::size_t>(last - first);
        };
    auto const p0 = dest;
    auto const p1 = dest + size;
    auto p = p0;
loop:
    switch(stack_.front().st)
    {
    case state::val:
    {
loop_init:
        auto const& jv =
            *stack_.front().pjv;
        switch(jv.kind())
        {
        case kind::object:
            stack_.pop();
            stack_.emplace_front(
                *jv.if_object());
            goto loop;

        case kind::array:
            stack_.pop();
            stack_.emplace_front(
                *jv.if_array());
            goto loop;

        case kind::string:
            str_ = *jv.if_string();
            stack_.front().st = state::str1;
            goto loop_str;
    
        case kind::number:
            if(p1 - p >= number::max_string_chars)
            {
                p += jv.if_number()->print(
                    p, p1 - p).size();
                stack_.pop();
                goto loop;
            }
            str_ = jv.if_number()->print(
                buf_, sizeof(buf_));
            stack_.front().st = state::lit;
            goto loop;
    
        case kind::boolean:
            if(*jv.if_bool())
            {
                if(p1 - p >= 4)
                {
                    *p++ = 't';
                    *p++ = 'r';
                    *p++ = 'u';
                    *p++ = 'e';
                    stack_.pop();
                    goto loop;
                }
                str_ = {"true", 4U};
                stack_.front().st = state::lit;
                goto loop;
            }
            if(p1 - p >= 5)
            {
                *p++ = 'f';
                *p++ = 'a';
                *p++ = 'l';
                *p++ = 's';
                *p++ = 'e';
                stack_.pop();
                goto loop;
            }
            str_ = {"false", 5U};
            stack_.front().st = state::lit;
            goto loop;
   
        case kind::null:
            if(p1 - p >= 4)
            {
                *p++ = 'n';
                *p++ = 'u';
                *p++ = 'l';
                *p++ = 'l';
                stack_.pop();
                goto loop;
            }
            str_ = {"null", 4U};
            stack_.front().st = state::lit;
            goto loop;
        }
    }

    case state::obj0:
    {
        if(p >= p1)
            goto finish;
        *p++ = '{';
        auto& e = stack_.front().obj;
        if(e.it == e.po->end())
        {
            stack_.front().st =
                state::obj3;
            goto loop;
        }
        stack_.front().st =
            state::obj1;
        str_ = e.it->first;
        stack_.emplace_front(
            state::str1);
        goto loop_str;
    }

    // key done
    case state::obj1:
    {
        if(p >= p1)
            goto finish;
        *p++ = ':';
        auto& e = stack_.front().obj;
        stack_.front().st =
            state::obj2;
        stack_.emplace_front(
            e.it->second);
        goto loop_init;
    }

    case state::obj2:
    {
        if(p >= p1)
            goto finish;
        auto& e = stack_.front().obj;
        ++e.it;
        if(e.it == e.po->end())
        {
            *p++ = '}';
            stack_.pop();
            goto loop;
        }
        *p++ = ',';
        stack_.front().st =
            state::obj1;
        str_ = e.it->first;
        stack_.emplace_front(
            state::str1);
        goto loop_str;
    }

    case state::obj3:
        if(p >= p1)
            goto finish;
        *p++ = '}';
        stack_.pop();
        goto loop;

    case state::arr0:
    {
        if(p >= p1)
            goto finish;
        *p++ = '[';
        auto& e = stack_.front().arr;
        if(e.it == e.pa->end())
        {
            stack_.front().st =
                state::arr2;
            goto loop;
        }
        stack_.front().st =
            state::arr1;
        stack_.emplace_front(*e.it);
        goto loop_init;
    }

    case state::arr1:
    {
        if(p >= p1)
            goto finish;
        auto& e = stack_.front().arr;
        ++e.it;
        if(e.it == e.pa->end())
        {
            *p++ = ']';
            stack_.pop();
            goto loop;
        }
        *p++ = ',';
        auto const& jv = *e.it;
        stack_.emplace_front(jv);
        goto loop_init;
    }

    case state::arr2:
        if(p >= p1)
            goto finish;
        *p++ = ']';
        stack_.pop();
        goto loop;

    case state::str1:
loop_str:
        if(p >= p1)
            goto finish;
        *p++ = '\"';
        stack_.front().st = state::str2;
        BOOST_FALLTHROUGH;
#if 1
    case state::str2:
    {
        auto s = str_.data();
        auto const s1 =
            s + str_.size();
        while(s < s1)
        {
            char const* sn;
            auto const d = dist(p, p1);
            if(d >= 6 * dist(s, s1))
                sn = s1;
            else if(d >= 6)
                sn = s + d / 6;
            else
                break;

            // fast loop
            while(s < sn)
            {
                auto const ch = *s++;
                auto const c = esc[static_cast<
                    unsigned char>(ch)];
                if(! c)
                {
                    *p++ = ch;
                    continue;
                }
                *p++ = '\\';
                *p++ = c;
                if(c != 'u')
                    continue;
                *p++ = '0';
                *p++ = '0';
                *p++ = hex[static_cast<
                    unsigned char>(ch) >> 4];
                *p++ = hex[static_cast<
                    unsigned char>(ch) & 15];
            }
        }
        while(p < p1 && s < s1)
        {
            auto const ch = *s++;
            auto const c = esc[static_cast<
                unsigned char>(ch)];
            if(! c)
            {
                *p++ = ch;
                continue;
            }
            if(c != 'u')
            {
                if(p1 - p >= 2)
                {
                    *p++ = '\\';
                    *p++ = c;
                    continue;
                }
                buf_[1] = '\\';
                buf_[0] = c;
                nbuf_ = 2;
                str_ = str_.substr(
                    s - str_.data());
                stack_.front().st =
                    state::str4;
                goto loop;
            }
            if(p1 - p >= 6)
            {
                *p++ = '\\';
                *p++ = 'u';
                *p++ = '0';
                *p++ = '0';
                *p++ = hex[static_cast<
                    unsigned char>(ch) >> 4];
                *p++ = hex[static_cast<
                    unsigned char>(ch) & 15];
                continue;
            }
            buf_[5] = '\\';
            buf_[4] = 'u';
            buf_[3] = '0';
            buf_[2] = '0';
            buf_[1] = hex[static_cast<
                unsigned char>(ch) >> 4];
            buf_[0] = hex[static_cast<
                unsigned char>(ch) & 15];
            nbuf_ = 6;
            str_ = str_.substr(
                s - str_.data());
            stack_.front().st =
                state::str4;
            goto loop;
        }
        if(s < s1)
        {
            str_ = str_.substr(
                s - str_.data());
            goto finish;
        }
        stack_.front().st =
            state::str3;
        BOOST_FALLTHROUGH;
    }
#else
    case state::str2:
    {
        auto s = str_.data();
        auto const s1 =
            s + str_.size();
        if(static_cast<unsigned long>(
            p1 - p) >= 6 * str_.size())
        {
            // fast loop
            while(s < s1)
            {
                auto const ch = *s++;
                auto const c = esc[static_cast<
                    unsigned char>(ch)];
                if(! c)
                {
                    *p++ = ch;
                    continue;
                }
                *p++ = '\\';
                *p++ = c;
                if(c != 'u')
                    continue;
                *p++ = '0';
                *p++ = '0';
                *p++ = hex[static_cast<
                    unsigned char>(ch) >> 4];
                *p++ = hex[static_cast<
                    unsigned char>(ch) & 15];
            }
        }
        else
        {
            while(p < p1 && s < s1)
            {
                auto const ch = *s++;
                auto const c = esc[static_cast<
                    unsigned char>(ch)];
                if(! c)
                {
                    *p++ = ch;
                    continue;
                }
                if(c != 'u')
                {
                    if(p + 2 <= p1)
                    {
                        *p++ = '\\';
                        *p++ = c;
                        continue;
                    }
                    buf_[1] = '\\';
                    buf_[0] = c;
                    nbuf_ = 2;
                    str_ = str_.substr(
                        s - str_.data());
                    stack_.front().st =
                        state::str4;
                    goto loop;
                }
                if(p + 6 <= p1)
                {
                    *p++ = '\\';
                    *p++ = 'u';
                    *p++ = '0';
                    *p++ = '0';
                    *p++ = hex[static_cast<
                        unsigned char>(ch) >> 4];
                    *p++ = hex[static_cast<
                        unsigned char>(ch) & 15];
                    continue;
                }
                buf_[5] = '\\';
                buf_[4] = 'u';
                buf_[3] = '0';
                buf_[2] = '0';
                buf_[1] = hex[static_cast<
                    unsigned char>(ch) >> 4];
                buf_[0] = hex[static_cast<
                    unsigned char>(ch) & 15];
                nbuf_ = 6;
                str_ = str_.substr(
                    s - str_.data());
                stack_.front().st =
                    state::str4;
                goto loop;
            }
        }
        if(s < s1)
        {
            str_ = str_.substr(
                s - str_.data());
            goto finish;
        }
        stack_.front().st =
            state::str3;
        BOOST_FALLTHROUGH;
    }
#endif

    case state::str3:
        if(p >= p1)
            goto finish;
        *p++ = '\"';
        stack_.pop();
        goto loop;

    case state::str4:
        while(p < p1)
        {
            *p++ = buf_[--nbuf_];
            if(! nbuf_)
            {
                stack_.front().st =
                    state::str2;
                goto loop;
            }
        }
        goto finish;

    case state::lit:
    {
        auto const n =
            str_.copy(p, p1 - p);
        p += n;
        if(n == str_.size())
        {
            stack_.pop();
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

string
to_string(
    json::value const& jv)
{
    string s;
    serializer sr(jv);
    while(! sr.is_done())
    {
        if(s.size() >= s.capacity())
            s.reserve(s.capacity() + 1);
        s.grow(static_cast<
            string::size_type>(
            sr.next(s.data() + s.size(),
                s.capacity() - s.size())));
    }
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
