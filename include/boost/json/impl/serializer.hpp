//
// Copyright (c) 2024 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZER_HPP
#define BOOST_JSON_IMPL_SERIALIZER_HPP

#include <boost/json/conversion.hpp>

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
    lit,
    arr1, arr2, arr3, arr4,
    obj1, obj2, obj3, obj4, obj5, obj6
};

bool
writer::
suspend(state st)
{
    st_.push(st);
    return false;
}

template<writer::state State>
bool
write_literal(writer& w, stream& ss)
{
    if( State == writer::state::nul1 )
    {
        if(BOOST_JSON_LIKELY( ss.remain() >= 4 ))
        {
            ss.append("null", 4);
            return true;
        }
        else
            goto do_nul1;
    }
    else if( State == writer::state::tru1 )
    {
        if(BOOST_JSON_LIKELY( ss.remain() >= 4 ))
        {
            ss.append("true", 4);
            return true;
        }
        else
            goto do_tru1;
    }
    else if( State == writer::state::fal1 )
    {
        if(BOOST_JSON_LIKELY( ss.remain() >= 5 ))
        {
            ss.append("false", 5);
            return true;
        }
        else
            goto do_fal1;
    }
    else
    {
        BOOST_JSON_UNREACHABLE();
    }

do_nul1:
    w.cs0_ = { "null", 4 };
    if(BOOST_JSON_LIKELY(ss))
        ss.append('n');
    else
        return w.suspend(writer::state::lit);
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
    {
        w.cs0_.skip(1);
        return w.suspend(writer::state::lit);
    }
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
    {
        w.cs0_.skip(2);
        return w.suspend(writer::state::lit);
    }
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
    {
        w.cs0_.skip(3);
        return w.suspend(writer::state::lit);
    }
    return true;

do_tru1:
    w.cs0_ = { "true", 4 };
    if(BOOST_JSON_LIKELY(ss))
        ss.append('t');
    else
        return w.suspend(writer::state::lit);
    if(BOOST_JSON_LIKELY(ss))
        ss.append('r');
    else
    {
        w.cs0_.skip(1);
        return w.suspend(writer::state::lit);
    }
    if(BOOST_JSON_LIKELY(ss))
        ss.append('u');
    else
    {
        w.cs0_.skip(2);
        return w.suspend(writer::state::lit);
    }
    if(BOOST_JSON_LIKELY(ss))
        ss.append('e');
    {
        w.cs0_.skip(3);
        return w.suspend(writer::state::lit);
    }
    return true;

do_fal1:
    w.cs0_ = { "false", 5 };
    if(BOOST_JSON_LIKELY(ss))
        ss.append('f');
    else
        return w.suspend(writer::state::lit);
    if(BOOST_JSON_LIKELY(ss))
        ss.append('a');
    else
    {
        w.cs0_.skip(1);
        return w.suspend(writer::state::lit);
    }
    if(BOOST_JSON_LIKELY(ss))
        ss.append('l');
    else
    {
        w.cs0_.skip(2);
        return w.suspend(writer::state::lit);
    }
    if(BOOST_JSON_LIKELY(ss))
        ss.append('s');
    else
    {
        w.cs0_.skip(3);
        return w.suspend(writer::state::lit);
    }
    if(BOOST_JSON_LIKELY(ss))
        ss.append('e');
    else
    {
        w.cs0_.skip(4);
        return w.suspend(writer::state::lit);
    }
    return true;
}

template<class T, bool StackEmpty>
bool
write_impl(null_like_conversion_tag, writer& w, stream& ss)
{
    if( StackEmpty || w.st_.empty() )
        return write_literal<writer::state::nul1>(w, ss);

    return resume_buffer(w, ss);
}

template<class T, bool StackEmpty>
bool
write_impl(bool_conversion_tag, writer& w, stream& ss)
{
    BOOST_ASSERT( w.p_ );
    auto const t = *reinterpret_cast<T const*>(w.p_);

    if( StackEmpty || w.st_.empty() )
    {
        if( t )
            return write_literal<writer::state::tru1>(w, ss);
        else
            return write_literal<writer::state::fal1>(w, ss);
    }

    return resume_buffer(w, ss);
}

template<class T, bool StackEmpty>
bool
write_impl(integral_conversion_tag, writer& w, stream& ss0)
{
    if( StackEmpty || w.st_.empty() )
    {
        auto const& t = *reinterpret_cast<T const*>(w.p_);

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wsign-compare"
#elif defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC  diagnostic ignored "-Wsign-compare"
#elif defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4018 )
#endif
        if( t < 0 )
        {
            // T is obviously signed, so this comparison is safe
            if( t >= (std::numeric_limits<std::int64_t>::min)() )
            {
                std::int64_t i = t;
                return write_int64(w, ss0, i);
            }
        }
        else if( t <= (std::numeric_limits<std::uint64_t>::max)() )
        {
            std::uint64_t u = t;
            return write_uint64(w, ss0, u);
        }
#if defined(__clang__)
# pragma clang diagnostic pop
#elif defined(__GNUC__)
# pragma GCC diagnostic pop
#elif defined(_MSC_VER)
# pragma warning( pop )
#endif

#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4244 )
#endif
        double d = t;
        return write_double(w, ss0, d);
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
    }

    return resume_buffer(w, ss0);
}

template<class T, bool StackEmpty>
bool
write_impl(floating_point_conversion_tag, writer& w, stream& ss0)
{
    if( StackEmpty || w.st_.empty() )
    {
        double d = *reinterpret_cast<T const*>(w.p_);
        return write_double(w, ss0, d);
    }

    return resume_buffer(w, ss0);
}

template<class T, bool StackEmpty>
bool
write_impl(writer& w, stream& ss)
{
    using cat = detail::generic_conversion_category<T>;
    return write_impl<T, StackEmpty>( cat(), w, ss );
}

} // namespace detail

template<class T>
void
serializer::reset(T const* p) noexcept
{
    BOOST_STATIC_ASSERT( !std::is_pointer<T>::value );
    BOOST_STATIC_ASSERT( std::is_object<T>::value );

    p_ = p;
    fn0_ = &detail::write_impl<T, true>;
    fn1_ = &detail::write_impl<T, false>;
    st_.clear();
    done_ = false;
}

void
serializer::reset(std::nullptr_t) noexcept
{
    p_ = nullptr;
    fn0_ = &detail::write_impl<std::nullptr_t, true>;
    fn1_ = &detail::write_impl<std::nullptr_t, false>;
    st_.clear();
    done_ = false;
}

} // namespace json
} // namespace boost

#endif // BOOST_JSON_IMPL_SERIALIZER_HPP
