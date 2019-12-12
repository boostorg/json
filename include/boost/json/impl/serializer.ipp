//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZER_IPP
#define BOOST_JSON_IMPL_SERIALIZER_IPP

#include <boost/json/serializer.hpp>
#include <boost/json/detail/format.hpp>
#include <boost/json/detail/sse2.hpp>
#include <ostream>

namespace boost {
namespace json {

enum class serializer::state : char
{
    none,   // no value
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

serializer::
serializer() noexcept
{
    // ensure room for \uXXXX escape plus one
    BOOST_STATIC_ASSERT(
        sizeof(serializer::buf_) >= 7);

    stack_.emplace(state::none);
}

void
serializer::
reset(value const& jv) noexcept
{
    stack_.clear();
    stack_.emplace(state::done);
    stack_.emplace(jv);
}

bool
serializer::
is_done() const noexcept
{
    return stack_->st == state::done;
}

std::size_t
serializer::
read(char* dest, std::size_t size)
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
    switch(stack_->st)
    {
    case state::none:
        BOOST_JSON_THROW(
            std::logic_error(
                "no value in serializer"));

    case state::val:
    {
loop_init:
        auto const& jv =
            *stack_->pjv;
        switch(jv.kind())
        {
        case kind::object:
            stack_.pop();
            stack_.emplace(
                *jv.if_object());
            goto loop;

        case kind::array:
            stack_.pop();
            stack_.emplace(
                *jv.if_array());
            goto loop;

        case kind::string:
            str_ = *jv.if_string();
            stack_->st = state::str1;
            goto loop_str;
    
        case kind::int64:
            if(p1 - p >= detail::max_number_chars)
            {
                p += detail::format_int64(
                    p,  *jv.if_int64());
                stack_.pop();
                goto loop;
            }
            str_ = { buf_, detail::format_int64(
                buf_, *jv.if_int64()) };
            stack_->st = state::lit;
            goto loop;

        case kind::uint64:
            if(p1 - p >= detail::max_number_chars)
            {
                p += detail::format_uint64(
                    p,  *jv.if_uint64());
                stack_.pop();
                goto loop;
            }
            str_ = { buf_, detail::format_uint64(
                buf_, *jv.if_uint64()) };
            stack_->st = state::lit;
            goto loop;

        case kind::double_:
            if(p1 - p >= detail::max_number_chars)
            {
                p += detail::format_double(
                    p,  *jv.if_double());
                stack_.pop();
                goto loop;
            }
            str_ = { buf_, detail::format_double(
                buf_, *jv.if_double()) };
            stack_->st = state::lit;
            goto loop;

        case kind::bool_:
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
                stack_->st = state::lit;
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
            stack_->st = state::lit;
            goto loop;

        default: // silences a warning
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
            stack_->st = state::lit;
            goto loop;
        }
    }

    case state::obj0:
    {
        if(p >= p1)
            goto finish;
        *p++ = '{';
        auto& e = stack_->obj;
        if(e.it == e.po->end())
        {
            stack_->st =
                state::obj3;
            goto loop;
        }
        stack_->st =
            state::obj1;
        str_ = e.it->key();
        stack_.emplace(
            state::str1);
        goto loop_str;
    }

    // key done
    case state::obj1:
    {
        if(p >= p1)
            goto finish;
        *p++ = ':';
        auto& e = stack_->obj;
        stack_->st =
            state::obj2;
        stack_.emplace(
            e.it->value());
        goto loop_init;
    }

    case state::obj2:
    {
        if(p >= p1)
            goto finish;
        auto& e = stack_->obj;
        ++e.it;
        if(e.it == e.po->end())
        {
            *p++ = '}';
            stack_.pop();
            goto loop;
        }
        *p++ = ',';
        stack_->st =
            state::obj1;
        str_ = e.it->key();
        stack_.emplace(
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
        auto& e = stack_->arr;
        if(e.it == e.pa->end())
        {
            stack_->st =
                state::arr2;
            goto loop;
        }
        stack_->st =
            state::arr1;
        stack_.emplace(*e.it);
        goto loop_init;
    }

    case state::arr1:
    {
        if(p >= p1)
            goto finish;
        auto& e = stack_->arr;
        ++e.it;
        if(e.it == e.pa->end())
        {
            *p++ = ']';
            stack_.pop();
            goto loop;
        }
        *p++ = ',';
        auto const& jv = *e.it;
        stack_.emplace(jv);
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
        stack_->st = state::str2;
        BOOST_FALLTHROUGH;

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
            // VFALCO We can raise this 6 in multiples,
            //        to balance the additional fixed
            //        overhead of additional outer loops
            //        when the amount of remaining output
            //        is small.
            //
            else if(d >= 6)
                sn = s + d / 6;
            else
                break;

            // fast loop
            char const* ss = s;
            s += detail::count_unescaped(s, sn - s);
            while(s < sn)
            {
                auto const ch = *s++;
                auto const c = esc[static_cast<
                    unsigned char>(ch)];
                if(! c)
                    continue;
                auto const n = (s - ss) - 1;
                std::memcpy(p, ss, n);
                ss = s;
                p += n;
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
            auto const n = s - ss;
            std::memcpy(p, ss, n);
            p += n;
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
                stack_->st =
                    state::str4;
                goto loop;
            }
        #if 0
            // VFALCO The current tuning makes this
            //        condition impossible to satisfy,
            //        see note above.
            //
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
        #else
            BOOST_ASSERT(p1 - p < 6);
        #endif

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
            stack_->st =
                state::str4;
            goto loop;
        }
        if(s < s1)
        {
            str_ = str_.substr(
                s - str_.data());
            goto finish;
        }
        stack_->st =
            state::str3;
        BOOST_FALLTHROUGH;
    }

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
                stack_->st =
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
        BOOST_ASSERT(p >= p1);
        goto finish;
    }

    case state::done:
        goto finish;
    }
finish:
    return p - p0;
}

//----------------------------------------------------------

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
            sr.read(s.data() + s.size(),
                s.capacity() - s.size())));
    }
    return s;
}

//[example_operator_lt__lt_
// Serialize a value into an output stream

std::ostream&
operator<<( std::ostream& os, value const& jv )
{
    // Create a serializer that is set to output our value.
    serializer sr( jv );

    // Loop until all output is produced.
    while( ! sr.is_done() )
    {
        // Use a local 4KB buffer.
        char buf[4096];

        // Try to fill up the local buffer.
        auto const n = sr.read(buf, sizeof(buf));

        // Write the valid portion of the buffer to the output stream.
        os.write(buf, n);
    }

    return os;
}
//]

} // json
} // boost

#endif
