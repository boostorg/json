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

template<class T, bool StackEmpty>
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
