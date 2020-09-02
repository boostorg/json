//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZER_IPP
#define BOOST_JSON_IMPL_SERIALIZER_IPP

#include <boost/json/serializer.hpp>
#include <boost/json/detail/format.hpp>
#include <boost/json/detail/sse2.hpp>
#include <ostream>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#endif

BOOST_JSON_NS_BEGIN

enum class serializer::state : char
{
    nul1, nul2, nul3, nul4,
    tru1, tru2, tru3, tru4,
    fal1, fal2, fal3, fal4, fal5,
    str1, str2, str3, str4, esc1,
    utf1, utf2, utf3, utf4, utf5,
    num,
    arr1, arr2, arr3, arr4,
    obj1, obj2, obj3, obj4, obj5, obj6
};

//----------------------------------------------------------

bool
serializer::
suspend(state st)
{
    st_.push(st);
    return false;
}

bool
serializer::
suspend(
    state st,
    array::const_iterator it,
    value const* jv)
{
    st_.push(jv);
    st_.push(it);
    st_.push(st);
    return false;
}

bool
serializer::
suspend(
    state st,
    object::const_iterator it,
    value const* jv)
{
    st_.push(jv);
    st_.push(it);
    st_.push(st);
    return false;
}

template<bool StackEmpty>
bool
serializer::
write_null(stream& ss0)
{
    local_stream ss(ss0);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::nul1: goto do_nul1;
        case state::nul2: goto do_nul2;
        case state::nul3: goto do_nul3;
        case state::nul4: goto do_nul4;
        }
    }
do_nul1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('n');
    else
        return suspend(state::nul1);
do_nul2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
        return suspend(state::nul2);
do_nul3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
        return suspend(state::nul3);
do_nul4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
        return suspend(state::nul4);
    return true;
}

template<bool StackEmpty>
bool
serializer::
write_true(stream& ss0)
{
    local_stream ss(ss0);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::tru1: goto do_tru1;
        case state::tru2: goto do_tru2;
        case state::tru3: goto do_tru3;
        case state::tru4: goto do_tru4;
        }
    }
do_tru1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('t');
    else
        return suspend(state::tru1);
do_tru2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('r');
    else
        return suspend(state::tru2);
do_tru3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
        return suspend(state::tru3);
do_tru4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('e');
    else
        return suspend(state::tru4);
    return true;
}

template<bool StackEmpty>
bool
serializer::
write_false(stream& ss0)
{
    local_stream ss(ss0);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
        case state::fal1: goto do_fal1;
        case state::fal2: goto do_fal2;
        case state::fal3: goto do_fal3;
        case state::fal4: goto do_fal4;
        case state::fal5: goto do_fal5;
        }
    }
do_fal1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('f');
    else
        return suspend(state::fal1);
do_fal2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('a');
    else
        return suspend(state::fal2);
do_fal3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
        return suspend(state::fal3);
do_fal4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('s');
    else
        return suspend(state::fal4);
do_fal5:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('e');
    else
        return suspend(state::fal5);
    return true;
}

template<bool StackEmpty>
bool
serializer::
write_string(stream& ss0)
{
    local_stream ss(ss0);
    local_const_stream cs(cs0_);
    if(! StackEmpty && ! st_.empty())
    {
        state st;
        st_.pop(st);
        switch(st)
        {
        default:
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
        return suspend(state::str1);

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
                    return suspend(state::str2);
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
        return suspend(state::str2);
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
                    buf_[0] = c;
                    return suspend(
                        state::esc1);
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
                    buf_[0] = hex[static_cast<
                        unsigned char>(ch) >> 4];
                    buf_[1] = hex[static_cast<
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
    return suspend(state::str3);

do_str4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('\x22'); // '"'
    else
        return suspend(state::str4);

do_esc1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(buf_[0]);
    else
        return suspend(state::esc1);
    goto do_str3;

do_utf1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
        return suspend(state::utf1);
do_utf2:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('0');
    else
        return suspend(state::utf2);
do_utf3:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('0');
    else
        return suspend(state::utf3);
do_utf4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(buf_[0]);
    else
        return suspend(state::utf4);
do_utf5:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(buf_[1]);
    else
        return suspend(state::utf5);
    goto do_str3;
}

template<bool StackEmpty>
bool
serializer::
write_number(stream& ss0)
{
    local_stream ss(ss0);
    if(StackEmpty || st_.empty())
    {
        switch(jv_->kind())
        {
        default:
        case kind::int64:
            if(BOOST_JSON_LIKELY(
                ss.remain() >=
                    detail::max_number_chars))
            {
                ss.advance(detail::format_int64(
                    ss.data(), jv_->get_int64()));
                return true;
            }
            cs0_ = { buf_, detail::format_int64(
                buf_, jv_->get_int64()) };
            break;

        case kind::uint64:
            if(BOOST_JSON_LIKELY(
                ss.remain() >=
                    detail::max_number_chars))
            {
                ss.advance(detail::format_uint64(
                    ss.data(), jv_->get_uint64()));
                return true;
            }
            cs0_ = { buf_, detail::format_uint64(
                buf_, jv_->get_uint64()) };
            break;

        case kind::double_:
            if(BOOST_JSON_LIKELY(
                ss.remain() >=
                    detail::max_number_chars))
            {
                ss.advance(detail::format_double(
                    ss.data(), jv_->get_double()));
                return true;
            }
            cs0_ = { buf_, detail::format_double(
                buf_, jv_->get_double()) };
            break;
        }
    }
    else
    {
        state st;
        st_.pop(st);
        BOOST_ASSERT(
            st == state::num);
    }
    auto const n = ss.remain();
    if(n < cs0_.remain())
    {
        ss.append(cs0_.data(), n);
        cs0_.skip(n);
        return suspend(state::num);
    }
    ss.append(
        cs0_.data(), cs0_.remain());
    return true;
}

template<bool StackEmpty>
bool
serializer::
write_array(stream& ss0)
{
    value const* jv;
    local_stream ss(ss0);
    array::const_iterator it;
    array::const_iterator end;
    if(StackEmpty || st_.empty())
    {
        jv = jv_;
        it = jv->get_array().begin();
        end = jv->get_array().end();
    }
    else
    {
        state st;
        st_.pop(st);
        st_.pop(it);
        st_.pop(jv);
        end = jv->get_array().end();
        switch(st)
        {
        default:
        case state::arr1: goto do_arr1;
        case state::arr2: goto do_arr2;
        case state::arr3: goto do_arr3;
        case state::arr4: goto do_arr4;
            break;
        }
    }
do_arr1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('[');
    else
        return suspend(
            state::arr1, it, jv);
    if(it == end)
        goto do_arr4;
    for(;;)
    {
do_arr2:
        jv_ = &*it;
        if(! write_value<StackEmpty>(ss))
            return suspend(
                state::arr2, it, jv);
        if(BOOST_JSON_UNLIKELY(
            ++it == end))
            break;
do_arr3:
        if(BOOST_JSON_LIKELY(ss))
            ss.append(',');
        else
            return suspend(
                state::arr3, it, jv);
    }
do_arr4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(']');
    else
        return suspend(
            state::arr4, it, jv);
    return true;
}

template<bool StackEmpty>
bool
serializer::
write_object(stream& ss0)
{
    value const* jv;
    local_stream ss(ss0);
    object::const_iterator it;
    object::const_iterator end;
    if(StackEmpty || st_.empty())
    {
        jv = jv_;
        it = jv->get_object().begin();
        end = jv->get_object().end();
    }
    else
    {
        state st;
        st_.pop(st);
        st_.pop(it);
        st_.pop(jv);
        end = jv->get_object().end();
        switch(st)
        {
        default:
        case state::obj1: goto do_obj1;
        case state::obj2: goto do_obj2;
        case state::obj3: goto do_obj3;
        case state::obj4: goto do_obj4;
        case state::obj5: goto do_obj5;
        case state::obj6: goto do_obj6;
            break;
        }
    }
do_obj1:
    if(BOOST_JSON_LIKELY(ss))
        ss.append('{');
    else
        return suspend(
            state::obj1, it, jv);
    if(BOOST_JSON_UNLIKELY(
        it == end))
        goto do_obj6;
    for(;;)
    {
        cs0_ = {
            it->key().data(),
            it->key().size() };
do_obj2:
        if(BOOST_JSON_UNLIKELY(
            ! write_string<StackEmpty>(ss)))
            return suspend(
                state::obj2, it, jv);
do_obj3:
        if(BOOST_JSON_LIKELY(ss))
            ss.append(':');
        else
            return suspend(
                state::obj3, it, jv);
do_obj4:
        jv_ = &it->value();
        if(BOOST_JSON_UNLIKELY(
            ! write_value<StackEmpty>(ss)))
            return suspend(
                state::obj4, it, jv);
        ++it;
        if(BOOST_JSON_UNLIKELY(it == end))
            break;
do_obj5:
        if(BOOST_JSON_LIKELY(ss))
            ss.append(',');
        else
            return suspend(
                state::obj5, it, jv);
    }
do_obj6:
    if(BOOST_JSON_LIKELY(ss))
    {
        ss.append('}');
        return true;
    }
    return suspend(
        state::obj6, it, jv);
}

template<bool StackEmpty>
bool
serializer::
write_value(stream& ss)
{
    if(StackEmpty || st_.empty())
    {
        auto const& jv(*jv_);
        switch(jv.kind())
        {
        default:
        case kind::object:
            return write_object<true>(ss);

        case kind::array:
            return write_array<true>(ss);

        case kind::string:
        {
            auto const& js = jv.get_string();
            cs0_ = { js.data(), js.size() };
            return write_string<true>(ss);
        }

        case kind::int64:
        case kind::uint64:
        case kind::double_:
            return write_number<true>(ss);

        case kind::bool_:
            if(jv.get_bool())
            {
                if(BOOST_JSON_LIKELY(
                    ss.remain() >= 4))
                {
                    ss.append("true", 4);
                    return true;
                }
                return write_true<true>(ss);
            }
            else
            {
                if(BOOST_JSON_LIKELY(
                    ss.remain() >= 5))
                {
                    ss.append("false", 5);
                    return true;
                }
                return write_false<true>(ss);
            }

        case kind::null:
            if(BOOST_JSON_LIKELY(
                ss.remain() >= 4))
            {
                ss.append("null", 4);
                return true;
            }
            return write_null<true>(ss);
        }
    }
    else
    {
        state st;
        st_.peek(st);
        switch(st)
        {
        default:
        case state::nul1: case state::nul2:
        case state::nul3: case state::nul4:
            return write_null<StackEmpty>(ss);

        case state::tru1: case state::tru2:
        case state::tru3: case state::tru4:
            return write_true<StackEmpty>(ss);

        case state::fal1: case state::fal2:
        case state::fal3: case state::fal4:
        case state::fal5:
            return write_false<StackEmpty>(ss);

        case state::str1: case state::str2:
        case state::str3: case state::str4:
        case state::esc1:
        case state::utf1: case state::utf2:
        case state::utf3: case state::utf4:
        case state::utf5:
            return write_string<StackEmpty>(ss);

        case state::num:
            return write_number<StackEmpty>(ss);

        case state::arr1: case state::arr2:
        case state::arr3: case state::arr4:
            return write_array<StackEmpty>(ss);

        case state::obj1: case state::obj2:
        case state::obj3: case state::obj4:
        case state::obj5: case state::obj6:
            return write_object<StackEmpty>(ss);
        }
    }
}

string_view
serializer::
read_some(
    char* dest, std::size_t size)
{
    stream ss(dest, size);
    if(st_.empty())
        write_value<true>(ss);
    else
        write_value<false>(ss);
    if(st_.empty())
    {
        done_ = true;
        jv_ = nullptr;
    }
    return string_view(
        dest, ss.used(dest));
}

//----------------------------------------------------------

serializer::
serializer() noexcept
{
    // ensure room for \uXXXX escape plus one
    BOOST_STATIC_ASSERT(
        sizeof(serializer::buf_) >= 7);
}

void
serializer::
reset(value const& jv) noexcept
{
    jv_ = &jv;
    st_.clear();
    done_ = false;
}

string_view
serializer::
read(char* dest, std::size_t size)
{
    if(! jv_)
    {
        static value const null;
        jv_ = &null;
    }
    return read_some(dest, size);
}

BOOST_JSON_NS_END

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
