//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZER_HPP
#define BOOST_JSON_IMPL_SERIALIZER_HPP

namespace boost {
namespace json {

template<class T>
void
serializer::
reset(T const* t)
{
    BOOST_STATIC_ASSERT(
        ! std::is_pointer<T>::value);
    pt_ = t;
    init_ = &serializer::template init<T>;
    w_.stack.clear();
    done_ = false;
}

template<class T>
bool
serializer::
init()
{
    BOOST_STATIC_ASSERT(
        ! std::is_pointer<T>::value);
    return write(w_,
        *(reinterpret_cast<T const*>(pt_)));
}

template<class T>
bool
serializer::
write(
    detail::writer& w,
    T const& t)
{
    BOOST_STATIC_ASSERT(
        ! std::is_pointer<T>::value);

    enum state : char
    {
        arr1, arr2, arr3
    };

    state st;
    auto const end = t.end();
    typename T::const_iterator it{};
    if(w.stack.empty())
    {
        it = t.begin();
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
    w.stack.push(&t);
    w.push_resume(
        [](detail::writer& w)
        {
            T const* pt;
            w.stack.pop(pt);
            return write(w, *pt);
        });
    return false;
}

} // json
} // boost

#endif
