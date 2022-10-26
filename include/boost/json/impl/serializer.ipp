//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZER_IPP
#define BOOST_JSON_IMPL_SERIALIZER_IPP

#include <boost/json/serializer.hpp>
#include <boost/json/detail/format.hpp>
#include <boost/json/detail/sse2.hpp>
#include <boost/static_assert.hpp>
#include <ostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#endif

namespace boost {
namespace json {

// ensure room for largest printed number
BOOST_STATIC_ASSERT(
    sizeof(detail::writer::temp_) >=
        detail::max_number_chars + 1);

// ensure room for \uXXXX escape plus one
BOOST_STATIC_ASSERT(
    sizeof(detail::writer::temp_) >= 7);

//----------------------------------------------------------

serializer::
~serializer() noexcept = default;

serializer::
serializer(
    storage_ptr sp,
    unsigned char* buf,
    std::size_t buf_size) noexcept
    : w_(
        std::move(sp),
        buf,
        buf_size)
{
}

//----------------------------------------------------------

string_view
serializer::
read_some(
    char* dest, std::size_t size)
{
    // If this goes off it means you forgot
    // to call reset() before seriailzing a
    // new value, or you never checked done()
    // to see if you should stop.
    BOOST_ASSERT(! done_);

    w_.prepare(dest, size);
    if(w_.stack.empty())
    {
        if((this->*init_)())
        {
            done_ = true;
            pt_ = nullptr;
        }
    }
    else
    {
        resume_fn fn;
        w_.stack.pop(fn);
        if(fn(w_))
        {
            done_ = true;
            pt_ = nullptr;
        }
    }
    return string_view(
        dest, w_.data() - dest);
}

//----------------------------------------------------------

serializer::
serializer() noexcept
{
}

void
serializer::
reset(value const* p) noexcept
{
    pt_ = p;
    init_ = &serializer::init_value;
    w_.stack.clear();
    done_ = false;
}

void
serializer::
reset(array const* p) noexcept
{
    pt_ = p;
    init_ = &serializer::init_array;
    w_.stack.clear();
    done_ = false;
}

void
serializer::
reset(object const* p) noexcept
{
    pt_ = p;
    init_ = &serializer::init_object;
    w_.stack.clear();
    done_ = false;
}

void
serializer::
reset(string const* p) noexcept
{
    pt_ = p;
    init_ = &serializer::init_string;
    w_.stack.clear();
    done_ = false;
}

void
serializer::
reset(string_view sv) noexcept
{
    pt_ = sv.data();
    pn_ = sv.size();
    init_ = &serializer::init_string_view;
    w_.stack.clear();
    done_ = false;
}

string_view
serializer::
read(char* dest, std::size_t size)
{
    if(! pt_)
    {
        static value const null;
        reset(&null);
    }
    return read_some(dest, size);
}

//------------------------------------------------

bool
serializer::
init_value()
{
    return write(w_,
        *static_cast<
            value const*>(pt_));
}

bool
serializer::
init_object()
{
    return write(w_,
        *static_cast<
            object const*>(pt_));
}

bool
serializer::
init_array()
{
    return write(w_,
        *static_cast<
            array const*>(pt_));
}

bool
serializer::
init_string()
{
    return write(w_,
        static_cast<
            string const*>(pt_)->subview());
}

bool
serializer::
init_string_view()
{
    return write(w_, string_view(
        static_cast<
            char const*>(pt_), pn_));
}

bool
serializer::
init_null()
{
    return write(w_, nullptr);
}

//------------------------------------------------

bool
serializer::
write(
    detail::writer& w,
    value const& jv)
{
    switch(jv.kind())
    {
    case kind::object:
        return write(w, jv.get_object());

    case kind::array:
        return write(w, jv.get_array());

    case kind::string:
        return write(w, jv.get_string().subview());

    case kind::int64:
        return write(w, jv.get_int64());

    case kind::uint64:
        return write(w, jv.get_uint64());

    case kind::double_:
        return write(w, jv.get_double());

    case kind::bool_:
        return write(w, jv.get_bool());

    default:
    case kind::null:
        break;
    }
    return write(w, nullptr);
}

bool
serializer::
write(
    detail::writer& w,
    object const& obj)
{
    enum state : char
    {
        obj1, obj2, obj3, obj4
    };

    state st;
    object::const_iterator it{};
    auto const end = obj.end();
    if(w.stack.empty())
    {
        it = obj.begin();
    }
    else
    {
        w.stack.pop(it);
        w.stack.pop(st);
        if(! w.do_resume())
            goto suspend;
        switch(st)
        {
        default:
        case state::obj1: goto do_obj1;
        case state::obj2: goto do_obj2;
        case state::obj3: goto do_obj3;
        case state::obj4: goto do_obj4;
        }
    }

do_obj1:
    if(! w.append('{'))
    {
        st = obj1;
        goto suspend;
    }
    if(it == end)
        goto do_obj4;
    for(;;)
    {
        // key
        if(! write(w, string_view(
            it->key().data(), it->key().size())))
        {
            st = obj2;
            goto suspend;
        }
do_obj2:
        if(! w.append(':'))
        {
            st = obj2;
            goto suspend;
        }
        if(! write(w, (*it++).value()))
        {
            st = obj3;
            goto suspend;
        }
do_obj3:
        if(it == end)
            break;
        if(! w.append(','))
        {
            st = obj3;
            goto suspend;
        }
    }
do_obj4:
    if(! w.append('}'))
    {
        st = obj4;
        goto suspend;
    }
    return true;

suspend:
    w.stack.push(st);
    w.stack.push(it);
    w.stack.push(&obj);
    w.push_resume(
        [](detail::writer& w)
        {
            object const* po;
            w.stack.pop(po);
            return write(w, *po);
        });
    return false;
}

bool
serializer::
write(
    detail::writer& w,
    array const& arr)
{
    enum state : char
    {
        arr1, arr2, arr3
    };

    state st;
    auto const end = arr.end();
    array::const_iterator it{};
    if(w.stack.empty())
    {
        it = arr.begin();
    }
    else
    {
        w.stack.pop(it);
        w.stack.pop(st);
        if(! w.do_resume())
            goto suspend;
        switch(st)
        {
        default:
        case state::arr1: goto do_arr1;
        case state::arr2: goto do_arr2;
        case state::arr3: goto do_arr3;
        }
    }

do_arr1:
    if(! w.append('['))
    {
        st = arr1;
        goto suspend;
    }
    if(it == end)
        goto do_arr3;
    for(;;)
    {
        if(! write(w, *it++))
        {
            st = arr2;
            goto suspend;
        }
    do_arr2:
        if(it == end)
            break;
        if(! w.append(','))
        {
            st = arr2;
            goto suspend;
        }
    }
do_arr3:
    if(! w.append(']'))
    {
        st = arr3;
        goto suspend;
    }
    return true;

suspend:
    w.stack.push(st);
    w.stack.push(it);
    w.stack.push(&arr);
    w.push_resume(
        [](detail::writer& w)
        {
            array const* pa;
            w.stack.pop(pa);
            return write(w, *pa);
        });
    return false;
}

bool
serializer::
write(
    detail::writer& w,
    string_view s)
{
    static constexpr char
    string_hex[] = "0123456789abcdef";

    static constexpr char
    string_esc[] =
        "uuuuuuuubtnufruuuuuuuuuuuuuuuuuu"
        "\0\0\"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\\\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

    enum state : char
    {
        str1, str2, str3, str4, esc1,
        utf1, utf2, utf3, utf4, utf5
    };

    char const* p = s.data();
    std::size_t n = s.size();

    state st;
    if(! w.stack.empty())
    {
        w.stack.pop(st);
        switch(st)
        {
        case state::str1: goto do_str1;
        case state::str2: goto do_str2;
        case state::str3: goto do_str3;
        case state::str4: goto do_str4;
        case state::esc1: goto do_esc1;
        case state::utf1: goto do_utf1;
        case state::utf2: goto do_utf2;
        case state::utf3: goto do_utf3;
        case state::utf4: goto do_utf4;
        case state::utf5: goto do_utf5;
        }
    }

do_str1:
    if(! w.append('\"'))
    {
        st = str1;
        goto suspend;
    }

do_str2:
    // handle the first contiguous
    // run of unescaped characters.
    if(! w.empty())
    {
        if(n > 0)
        {
            auto const avail = w.available();
            std::size_t n1;
            if(avail >= n)
                n1 = detail::count_unescaped(p, n);
            else
                n1 = detail::count_unescaped(p, avail);
            if(n1 > 0)
            {
                w.append_unsafe(p, n1);
                p += n1;
                n -= n1;
                if(w.empty())
                {
                    st = str2;
                    goto suspend;
                }
            }
        }
        else
        {
            // done
            w.append_unsafe('\"');
            return true;
        }
    }
    else
    {
        st = str2;
        goto suspend;
    }

do_str3:
    // loop over escaped and unescaped characters
    while(! w.empty())
    {
        if(n > 0)
        {
            auto const ch = *p;
            auto const c = string_esc[
                static_cast<unsigned char>(ch)];
            ++p;
            --n;
            if(! c)
            {
                w.append_unsafe(ch);
            }
            else if(c != 'u')
            {
                w.append_unsafe('\\');
                if(! w.append(c))
                {
                    w.temp_[0] = c;
                    st = esc1;
                    goto suspend;
                }
            }
            else
            {
                if(w.available() >= 6)
                {
                    w.append_unsafe("\\u00", 4);
                    w.append_unsafe(string_hex[static_cast<
                        unsigned char>(ch) >> 4]);
                    w.append_unsafe(string_hex[static_cast<
                        unsigned char>(ch) & 15]);
                }
                else
                {
                    w.append_unsafe('\\');
                    w.temp_[0] = string_hex[static_cast<
                        unsigned char>(ch) >> 4];
                    w.temp_[1] = string_hex[static_cast<
                        unsigned char>(ch) & 15];
                    goto do_utf1;
                }
            }
        }
        else
        {
            // done
            w.append_unsafe('\"');
            return true;
        }
    }
    st = str3;
    goto suspend;

do_str4:
    if(! w.append('\"'))
    {
        st = str4;
        goto suspend;
    }

do_esc1:
    if(! w.append(w.temp_[0]))
    {
        st = esc1;
        goto suspend;
    }
    goto do_str3;

do_utf1:
    if(! w.append('u'))
    {
        st = utf1;
        goto suspend;
    }
do_utf2:
    if(! w.append('0'))
    {
        st = utf2;
        goto suspend;
    }
do_utf3:
    if(! w.append('0'))
    {
        st = utf3;
        goto suspend;
    }
do_utf4:
    if(! w.append(w.temp_[0]))
    {
        st = utf4;
        goto suspend;
    }
do_utf5:
    if(! w.append(w.temp_[1]))
    {
        st = utf5;
        goto suspend;
    }
    goto do_str3;

suspend:
    w.stack.push(st);
    w.stack.push(n);
    w.stack.push(p);
    w.push_resume(
        [](detail::writer& w)
        {
            char const* p;
            std::size_t n;

            w.stack.pop(p);
            w.stack.pop(n);
            return write(w,
                string_view(p, n));
        });
    return false;
}

bool
serializer::
write(
    detail::writer& w,
    char const* s,
    std::size_t n)
{
    auto const avail = w.available();
    if(avail >= n)
    {
        w.append_unsafe(s, n);
        return true;
    }

    // partial output
    w.append_unsafe(s, avail);
    s += avail;
    n -= avail;

    // suspend
    w.stack.push(n);
    w.stack.push(s);
    w.push_resume(
        [](detail::writer& w) -> bool
        {
            char const* s;
            std::size_t n;
            w.stack.pop(s);
            w.stack.pop(n);
            return write(w, s, n);
        });
    return false;
}

bool
serializer::
write(
    detail::writer& w,
    double v)
{
    auto s = detail::write_double(
        w.temp_, sizeof(w.temp_), v);
    return write(w, s.data(), s.size());
}

bool
serializer::
write(
    detail::writer& w,
    std::int64_t v)
{
    auto s = detail::write_int64(
        w.temp_, sizeof(w.temp_), v);
    return write(w, s.data(), s.size());
}

bool
serializer::
write(
    detail::writer& w,
    std::uint64_t v)
{
    auto s = detail::write_uint64(
        w.temp_, sizeof(w.temp_), v);
    return write(w, s.data(), s.size());
}

bool
serializer::
write(
    detail::writer& w,
    bool b)
{
    if(b)
        return write(w, "true", 4);
    return write(w, "false", 5);
}

bool
serializer::
write(detail::writer& w, std::nullptr_t)
{
    return write(w, "null", 4);
}

//------------------------------------------------

} // namespace json
} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
