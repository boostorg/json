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

template<class T>
bool
writer::
suspend(state st, iterator_type<T const> it, T const* pt)
{
    st_.push(pt);
    st_.push(it);
    st_.push(st);
    return false;
}

template<class T, bool StackEmpty>
BOOST_FORCEINLINE
bool
write_impl(null_like_conversion_tag, writer& w, stream& ss)
{
#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4127 )
#endif
    if( StackEmpty || w.st_.empty() )
        return write_null(w, ss);
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
    return resume_buffer(w, ss);
}

template<class T, bool StackEmpty>
BOOST_FORCEINLINE
bool
write_impl(bool_conversion_tag, writer& w, stream& ss)
{
    BOOST_ASSERT( w.p_ );
    auto const t = *reinterpret_cast<T const*>(w.p_);

#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4127 )
#endif
    if( StackEmpty || w.st_.empty() )
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
    {
        if( t )
            return write_true(w, ss);
        else
            return write_false(w, ss);
    }

    return resume_buffer(w, ss);
}

template<class T, bool StackEmpty>
BOOST_FORCEINLINE
bool
write_impl(integral_conversion_tag, writer& w, stream& ss0)
{
#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4127 )
#endif
    if( StackEmpty || w.st_.empty() )
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
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
# pragma warning( disable : 4127 )
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
BOOST_FORCEINLINE
bool
write_impl(floating_point_conversion_tag, writer& w, stream& ss0)
{
#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4127 )
#endif
    if( StackEmpty || w.st_.empty() )
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
    {
        double d = *reinterpret_cast<T const*>(w.p_);
        return write_double(w, ss0, d);
    }

    return resume_buffer(w, ss0);
}

template<class T, bool StackEmpty>
BOOST_FORCEINLINE
bool
write_impl(string_like_conversion_tag, writer& w, stream& ss0)
{
#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4127 )
#endif
    if( StackEmpty || w.st_.empty() )
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
    {
        string_view const sv = *reinterpret_cast<T const*>(w.p_);
        w.cs0_ = { sv.data(), sv.size() };
        return write_string(w, ss0);
    }

    return resume_string(w, ss0);
}

template<class T, bool StackEmpty>
BOOST_FORCEINLINE
bool
write_impl(sequence_conversion_tag, writer& w, stream& ss0)
{
    using It = iterator_type<T const>;
    using Elem = value_type<T>;

    T const* pt;
    local_stream ss(ss0);
    It it;
    It end;
#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4127 )
#endif
    if(StackEmpty || w.st_.empty())
    {
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
        BOOST_ASSERT( w.p_ );
        pt = reinterpret_cast<T const*>(w.p_);
        it = std::begin(*pt);
        end = std::end(*pt);
    }
    else
    {
        writer::state st;
        w.st_.pop(st);
        w.st_.pop(it);
        w.st_.pop(pt);
        end = std::end(*pt);
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
        return w.suspend(writer::state::arr1, it, pt);
    if(it == end)
        goto do_arr4;
    for(;;)
    {
        w.p_ = std::addressof(*it);
do_arr2:
        if( !write_impl<Elem, StackEmpty>(w, ss) )
            return w.suspend(writer::state::arr2, it, pt);
        if(BOOST_JSON_UNLIKELY( ++it == end ))
            break;
do_arr3:
        if(BOOST_JSON_LIKELY(ss))
            ss.append(',');
        else
            return w.suspend(writer::state::arr3, it, pt);
    }
do_arr4:
    if(BOOST_JSON_LIKELY(ss))
        ss.append(']');
    else
        return w.suspend(writer::state::arr4, it, pt);
    return true;
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

} // namespace json
} // namespace boost

#endif // BOOST_JSON_IMPL_SERIALIZER_HPP