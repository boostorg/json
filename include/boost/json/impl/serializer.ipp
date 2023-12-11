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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#endif

namespace boost {
namespace json {
namespace detail {

enum class writer::state : char
{
    nul1, nul2, nul3, nul4,
    tru1, tru2, tru3, tru4,
    fal1, fal2, fal3, fal4, fal5,
    str1, str2, str3, esc1, utf1,
    utf2, utf3, utf4, utf5,
    num,
    arr1, arr2, arr3, arr4,
    obj1, obj2, obj3, obj4, obj5, obj6
};

writer::
writer(
    storage_ptr sp,
    unsigned char* buf,
    std::size_t buf_size,
    serialize_options const& opts) noexcept
    : st_(
        std::move(sp),
        buf,
        buf_size)
    , opts_(opts)
{
    // ensure room for \uXXXX escape plus one
    BOOST_STATIC_ASSERT(sizeof(buf_) >= 7);
}

bool
writer::
suspend(state st)
{
    st_.push(st);
    return false;
}

bool
writer::
suspend(
    state st,
    array::const_iterator it,
    array const* pa)
{
    st_.push(pa);
    st_.push(it);
    st_.push(st);
    return false;
}

bool
writer::
suspend(
    state st,
    object::const_iterator it,
    object const* po)
{
    st_.push(po);
    st_.push(it);
    st_.push(st);
    return false;
}

template<bool StackEmpty>
bool
write_null(writer& w, stream& ss0)
{
    local_stream ss(ss0);
    if(! StackEmpty && ! w.st_.empty())
    {
        writer::state st;
        w.st_.pop(st);
        switch(st)
        {
        default:
        case writer::state::nul1: goto do_nul1;
        case writer::state::nul2: goto do_nul2;
        case writer::state::nul3: goto do_nul3;
        case writer::state::nul4: goto do_nul4;
        }
    }
do_nul1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('n');
    else
        return w.suspend(writer::state::nul1);
do_nul2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
        return w.suspend(writer::state::nul2);
do_nul3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
        return w.suspend(writer::state::nul3);
do_nul4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
        return w.suspend(writer::state::nul4);
    return true;
}

template<bool StackEmpty>
bool
write_true(writer& w, stream& ss0)
{
    local_stream ss(ss0);
    if(! StackEmpty && ! w.st_.empty())
    {
        writer::state st;
        w.st_.pop(st);
        switch(st)
        {
        default:
        case writer::state::tru1: goto do_tru1;
        case writer::state::tru2: goto do_tru2;
        case writer::state::tru3: goto do_tru3;
        case writer::state::tru4: goto do_tru4;
        }
    }
do_tru1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('t');
    else
        return w.suspend(writer::state::tru1);
do_tru2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('r');
    else
        return w.suspend(writer::state::tru2);
do_tru3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
        return w.suspend(writer::state::tru3);
do_tru4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('e');
    else
        return w.suspend(writer::state::tru4);
    return true;
}

template<bool StackEmpty>
bool
write_false(writer& w, stream& ss0)
{
    local_stream ss(ss0);
    if(! StackEmpty && ! w.st_.empty())
    {
        writer::state st;
        w.st_.pop(st);
        switch(st)
        {
        default:
        case writer::state::fal1: goto do_fal1;
        case writer::state::fal2: goto do_fal2;
        case writer::state::fal3: goto do_fal3;
        case writer::state::fal4: goto do_fal4;
        case writer::state::fal5: goto do_fal5;
        }
    }
do_fal1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('f');
    else
        return w.suspend(writer::state::fal1);
do_fal2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('a');
    else
        return w.suspend(writer::state::fal2);
do_fal3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
        return w.suspend(writer::state::fal3);
do_fal4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('s');
    else
        return w.suspend(writer::state::fal4);
do_fal5:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('e');
    else
        return w.suspend(writer::state::fal5);
    return true;
}

template<bool StackEmpty>
bool
write_string(writer& w, stream& ss0)
{
    local_stream ss(ss0);
    local_const_stream cs(w.cs0_);
    if(! StackEmpty && ! w.st_.empty())
    {
        writer::state st;
        w.st_.pop(st);
        switch(st)
        {
        default:
        case writer::state::str1: goto do_str1;
        case writer::state::str2: goto do_str2;
        case writer::state::str3: goto do_str3;
        case writer::state::esc1: goto do_esc1;
        case writer::state::utf1: goto do_utf1;
        case writer::state::utf2: goto do_utf2;
        case writer::state::utf3: goto do_utf3;
        case writer::state::utf4: goto do_utf4;
        case writer::state::utf5: goto do_utf5;
        }
    }
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

    // opening quote
do_str1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('\x22'); // '"'
    else
        return w.suspend(writer::state::str1);

    // fast loop,
    // copy unescaped
do_str2:
    if(BOOST_JSON_LIKELY(ss))
    {
        std::size_t n = cs.remain();
        if(BOOST_JSON_LIKELY(n > 0))
        {
            if(ss.remain() > n)
                n = detail::count_unescaped(
                    cs.data(), n);
            else
                n = detail::count_unescaped(
                    cs.data(), ss.remain());
            if(n > 0)
            {
                ss.append(cs.data(), n);
                cs.skip(n);
                if(! ss)
                    return w.suspend(writer::state::str2);
            }
        }
        else
        {
            ss.append('\x22'); // '"'
            return true;
        }
    }
    else
    {
        return w.suspend(writer::state::str2);
    }

    // slow loop,
    // handle escapes
do_str3:
    while(BOOST_JSON_LIKELY(ss))
    {
        if(BOOST_JSON_LIKELY(cs))
        {
            auto const ch = *cs;
            auto const c = esc[static_cast<
                unsigned char>(ch)];
            ++cs;
            if(! c)
            {
                ss.append(ch);
            }
            else if(c != 'u')
            {
                ss.append('\\');
                if(BOOST_JSON_LIKELY(ss))
                {
                    ss.append(c);
                }
                else
                {
                    w.buf_[0] = c;
                    return w.suspend(
                        writer::state::esc1);
                }
            }
            else
            {
                if(BOOST_JSON_LIKELY(
                    ss.remain() >= 6))
                {
                    ss.append("\\u00", 4);
                    ss.append(hex[static_cast<
                        unsigned char>(ch) >> 4]);
                    ss.append(hex[static_cast<
                        unsigned char>(ch) & 15]);
                }
                else
                {
                    ss.append('\\');
                    w.buf_[0] = hex[static_cast<
                        unsigned char>(ch) >> 4];
                    w.buf_[1] = hex[static_cast<
                        unsigned char>(ch) & 15];
                    goto do_utf1;
                }
            }
        }
        else
        {
            ss.append('\x22'); // '"'
            return true;
        }
    }
    return w.suspend(writer::state::str3);

do_esc1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(w.buf_[0]);
    else
        return w.suspend(writer::state::esc1);
    goto do_str3;

do_utf1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
        return w.suspend(writer::state::utf1);
do_utf2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('0');
    else
        return w.suspend(writer::state::utf2);
do_utf3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('0');
    else
        return w.suspend(writer::state::utf3);
do_utf4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(w.buf_[0]);
    else
        return w.suspend(writer::state::utf4);
do_utf5:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(w.buf_[1]);
    else
        return w.suspend(writer::state::utf5);
    goto do_str3;
}

template<bool StackEmpty>
bool
write_number(writer& w, stream& ss0)
{
    BOOST_ASSERT( w.p_ );
    auto const pv = reinterpret_cast<value const*>(w.p_);
    local_stream ss(ss0);
    if(StackEmpty || w.st_.empty())
    {
        switch(pv->kind())
        {
        default:
        case kind::int64:
            if(BOOST_JSON_LIKELY(
                ss.remain() >=
                    detail::max_number_chars))
            {
                ss.advance(detail::format_int64(
                    ss.data(), pv->get_int64()));
                return true;
            }
            w.cs0_ = { w.buf_, detail::format_int64(
                w.buf_, pv->get_int64()) };
            break;

        case kind::uint64:
            if(BOOST_JSON_LIKELY(
                ss.remain() >=
                    detail::max_number_chars))
            {
                ss.advance(detail::format_uint64(
                    ss.data(), pv->get_uint64()));
                return true;
            }
            w.cs0_ = { w.buf_, detail::format_uint64(
                w.buf_, pv->get_uint64()) };
            break;

        case kind::double_:
            if(BOOST_JSON_LIKELY(
                ss.remain() >=
                    detail::max_number_chars))
            {
                ss.advance(
                    detail::format_double(
                        ss.data(),
                        pv->get_double(),
                        w.opts_.allow_infinity_and_nan));
                return true;
            }
            w.cs0_ = { w.buf_, detail::format_double(
                w.buf_, pv->get_double(), w.opts_.allow_infinity_and_nan) };
            break;
        }
    }
    else
    {
        writer::state st;
        w.st_.pop(st);
        BOOST_ASSERT(
            st == writer::state::num);
    }
    auto const n = ss.remain();
    if(n < w.cs0_.remain())
    {
        ss.append(w.cs0_.data(), n);
        w.cs0_.skip(n);
        return w.suspend(writer::state::num);
    }
    ss.append(
        w.cs0_.data(), w.cs0_.remain());
    return true;
}

template<bool StackEmpty>
bool
write_value(writer& w, stream& ss);

template<bool StackEmpty>
bool
write_array(writer& w, stream& ss0)
{
    array const* pa;
    local_stream ss(ss0);
    array::const_iterator it;
    array::const_iterator end;
    if(StackEmpty || w.st_.empty())
    {
        BOOST_ASSERT( w.p_ );
        pa = reinterpret_cast<array const*>(w.p_);
        it = pa->begin();
        end = pa->end();
    }
    else
    {
        writer::state st;
        w.st_.pop(st);
        w.st_.pop(it);
        w.st_.pop(pa);
        end = pa->end();
        switch(st)
        {
        default:
        case writer::state::arr1: goto do_arr1;
        case writer::state::arr2: goto do_arr2;
        case writer::state::arr3: goto do_arr3;
        case writer::state::arr4: goto do_arr4;
            break;
        }
    }
do_arr1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('[');
    else
        return w.suspend(
            writer::state::arr1, it, pa);
    if(it == end)
        goto do_arr4;
    for(;;)
    {
do_arr2:
        w.p_ = &*it;
        if( !write_value<StackEmpty>(w, ss) )
            return w.suspend(
                writer::state::arr2, it, pa);
        if(BOOST_JSON_UNLIKELY(
            ++it == end))
            break;
do_arr3:
        if(BOOST_JSON_LIKELY(ss))
            ss.append(',');
        else
            return w.suspend(
                writer::state::arr3, it, pa);
    }
do_arr4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(']');
    else
        return w.suspend(
            writer::state::arr4, it, pa);
    return true;
}

template<bool StackEmpty>
bool
write_object(writer& w, stream& ss0)
{
    object const* po;
    local_stream ss(ss0);
    object::const_iterator it;
    object::const_iterator end;
    if(StackEmpty || w.st_.empty())
    {
        BOOST_ASSERT( w.p_ );
        po = reinterpret_cast<object const*>(w.p_);
        it = po->begin();
        end = po->end();
    }
    else
    {
        writer::state st;
        w.st_.pop(st);
        w.st_.pop(it);
        w.st_.pop(po);
        end = po->end();
        switch(st)
        {
        default:
        case writer::state::obj1: goto do_obj1;
        case writer::state::obj2: goto do_obj2;
        case writer::state::obj3: goto do_obj3;
        case writer::state::obj4: goto do_obj4;
        case writer::state::obj5: goto do_obj5;
        case writer::state::obj6: goto do_obj6;
            break;
        }
    }
do_obj1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('{');
    else
        return w.suspend(
            writer::state::obj1, it, po);
    if(BOOST_JSON_UNLIKELY(
        it == end))
        goto do_obj6;
    for(;;)
    {
        w.cs0_ = {
            it->key().data(),
            it->key().size() };
do_obj2:
        if(BOOST_JSON_UNLIKELY( !write_string<StackEmpty>(w, ss) ))
            return w.suspend(
                writer::state::obj2, it, po);
do_obj3:
        if(BOOST_JSON_LIKELY(ss))
            ss.append(':');
        else
            return w.suspend(
                writer::state::obj3, it, po);
do_obj4:
        w.p_ = &it->value();
        if(BOOST_JSON_UNLIKELY( !write_value<StackEmpty>(w, ss) ))
            return w.suspend(
                writer::state::obj4, it, po);
        ++it;
        if(BOOST_JSON_UNLIKELY(it == end))
            break;
do_obj5:
        if(BOOST_JSON_LIKELY(ss))
            ss.append(',');
        else
            return w.suspend(
                writer::state::obj5, it, po);
    }
do_obj6:
    if(BOOST_JSON_LIKELY(ss))
    {
        ss.append('}');
        return true;
    }
    return w.suspend(
        writer::state::obj6, it, po);
}

template<bool StackEmpty>
bool
write_value(writer& w, stream& ss)
{
    if(StackEmpty || w.st_.empty())
    {
        BOOST_ASSERT( w.p_ );
        auto const pv = reinterpret_cast<value const*>(w.p_);
        switch(pv->kind())
        {
        default:
        case kind::object:
            w.p_ = &pv->get_object();
            return write_object<true>(w, ss);

        case kind::array:
            w.p_ = &pv->get_array();
            return write_array<true>(w, ss);

        case kind::string:
        {
            auto const& js = pv->get_string();
            w.cs0_ = { js.data(), js.size() };
            return write_string<true>(w, ss);
        }

        case kind::int64:
        case kind::uint64:
        case kind::double_:
            return write_number<true>(w, ss);

        case kind::bool_:
            if(pv->get_bool())
            {
                if(BOOST_JSON_LIKELY(
                    ss.remain() >= 4))
                {
                    ss.append("true", 4);
                    return true;
                }
                return write_true<true>(w, ss);
            }
            else
            {
                if(BOOST_JSON_LIKELY(
                    ss.remain() >= 5))
                {
                    ss.append("false", 5);
                    return true;
                }
                return write_false<true>(w, ss);
            }

        case kind::null:
            if(BOOST_JSON_LIKELY(
                ss.remain() >= 4))
            {
                ss.append("null", 4);
                return true;
            }
            return write_null<true>(w, ss);
        }
    }
    else
    {
        writer::state st;
        w.st_.peek(st);
        switch(st)
        {
        default:
        case writer::state::nul1: case writer::state::nul2:
        case writer::state::nul3: case writer::state::nul4:
            return write_null<StackEmpty>(w, ss);

        case writer::state::tru1: case writer::state::tru2:
        case writer::state::tru3: case writer::state::tru4:
            return write_true<StackEmpty>(w, ss);

        case writer::state::fal1: case writer::state::fal2:
        case writer::state::fal3: case writer::state::fal4:
        case writer::state::fal5:
            return write_false<StackEmpty>(w, ss);

        case writer::state::str1: case writer::state::str2:
        case writer::state::str3: case writer::state::esc1:
        case writer::state::utf1: case writer::state::utf2:
        case writer::state::utf3: case writer::state::utf4:
        case writer::state::utf5:
            return write_string<StackEmpty>(w, ss);

        case writer::state::num:
            return write_number<StackEmpty>(w, ss);

        case writer::state::arr1: case writer::state::arr2:
        case writer::state::arr3: case writer::state::arr4:
            return write_array<StackEmpty>(w, ss);

        case writer::state::obj1: case writer::state::obj2:
        case writer::state::obj3: case writer::state::obj4:
        case writer::state::obj5: case writer::state::obj6:
            return write_object<StackEmpty>(w, ss);
        }
    }
}

} // namespace detail

serializer::
serializer(serialize_options const& opts) noexcept
    : serializer({}, nullptr, 0, opts)
{}

serializer::
serializer(
    storage_ptr sp,
    unsigned char* buf,
    std::size_t buf_size,
    serialize_options const& opts) noexcept
    : detail::writer(std::move(sp), buf, buf_size, opts)
{}

void
serializer::
reset(value const* p) noexcept
{
    p_ = p;
    fn0_ = &detail::write_value<true>;
    fn1_ = &detail::write_value<false>;
    st_.clear();
    done_ = false;
}

void
serializer::
reset(array const* p) noexcept
{
    p_ = p;
    fn0_ = &detail::write_array<true>;
    fn1_ = &detail::write_array<false>;
    st_.clear();
    done_ = false;
}

void
serializer::
reset(object const* p) noexcept
{
    p_ = p;
    fn0_ = &detail::write_object<true>;
    fn1_ = &detail::write_object<false>;
    st_.clear();
    done_ = false;
}

void
serializer::
reset(string const* p) noexcept
{
    cs0_ = { p->data(), p->size() };
    fn0_ = &detail::write_string<true>;
    fn1_ = &detail::write_string<false>;
    st_.clear();
    done_ = false;
}

void
serializer::
reset(string_view sv) noexcept
{
    cs0_ = { sv.data(), sv.size() };
    fn0_ = &detail::write_string<true>;
    fn1_ = &detail::write_string<false>;
    st_.clear();
    done_ = false;
}

string_view
serializer::
read(char* dest, std::size_t size)
{
    if(! fn0_)
    {
        static value const null;
        reset(&null);
    }

    // If this goes off it means you forgot
    // to call reset() before seriailzing a
    // new value, or you never checked done()
    // to see if you should stop.
    BOOST_ASSERT(! done_);

    detail::stream ss(dest, size);
    if(st_.empty())
        fn0_(*this, ss);
    else
        fn1_(*this, ss);
    if(st_.empty())
    {
        done_ = true;
        fn0_ = nullptr;
        p_ = nullptr;
    }
    return string_view(
        dest, ss.used(dest));
}

} // namespace json
} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
