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
            if( t < (std::numeric_limits<std::int64_t>::min)() )
                goto do_double;
            else
                goto do_int64;
        }

        if( t > (std::numeric_limits<std::int64_t>::max)() )
        {
            // T is potentially signed, but definitely not negative, so this
            // comparison is safe
            if( t > (std::numeric_limits<std::uint64_t>::max)() )
                goto do_double;
            else
                goto do_uint64;
        }
#if defined(__clang__)
# pragma clang diagnostic pop
#elif defined(__GNUC__)
# pragma GCC diagnostic pop
#elif defined(_MSC_VER)
# pragma warning( pop )
#endif

do_int64:
        {
            std::int64_t i = t;
            w.p_ = &i;
            return write_int64(w, ss0);
        }

do_uint64:
        {
            std::uint64_t u = t;
            w.p_ = &u;
            return write_uint64(w, ss0);
        }

do_double:
        {
#if defined(_MSC_VER)
# pragma warning( push )
# pragma warning( disable : 4244 )
#endif
            double d = t;
            w.p_ = &d;
            return write_double(w, ss0);
#if defined(_MSC_VER)
# pragma warning( pop )
#endif
        }
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
        w.p_ = &d;
        return write_double(w, ss0);
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

} // namespace json
} // namespace boost

#endif // BOOST_JSON_IMPL_SERIALIZER_HPP
