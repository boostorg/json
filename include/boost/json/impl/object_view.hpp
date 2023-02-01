//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_OBJECT_VIEW_HPP
#define BOOST_JSON_IMPL_OBJECT_VIEW_HPP

#include <boost/json/object_view.hpp>
#include <boost/json/value.hpp>
#include <iterator>
#include <cmath>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

template<typename Map>
struct object_view::map_adaptor final : adaptor
{
    bool            empty(const void * data)   const override
    {
        return static_cast<const Map*>(data)->empty();
    }
    std::size_t      size(const void * data)   const override
    {
        return static_cast<const Map*>(data)->size();
    }
    std::size_t  max_size(const void * data)   const override
    {
        return static_cast<const Map*>(data)->max_size();
    }
    std::size_t  capacity(const void * data)   const override
    {
        return static_cast<const Map*>(data)->max_size();
    }
    value_view         at(const void * data, string_view key) const override
    {
        return static_cast<const Map*>(data)->at(key);
    }
    boost::optional<value_view> if_contains(const void * data, string_view key) const override
    {
        auto mp = static_cast<const Map*>(data);
        auto itr = mp->find(key);
        if (itr != mp->end())
            return value_view(std::get<1>(*itr));
        else
            return boost::none;
    }

    static const void * conv(typename Map::const_iterator iterator)
    {
        static_assert(sizeof(typename Map::const_iterator) == sizeof(const void *),
                      "Iterator must have the size of a void pointer.");

        return *reinterpret_cast<const void**>(&iterator);
    }

    static typename Map::const_iterator conv_in(const void * ptr)
    {
        static_assert(sizeof(typename Map::const_iterator) == sizeof(const void *),
                      "Iterator must have the size of a void pointer.");

        return *reinterpret_cast<typename Map::const_iterator*>(&ptr);
    }

    const void * begin(const void *data) const override
    {
        return conv(static_cast<const Map*>(data)->begin());
    }
    const void * end  (const void *data) const override
    {
        return conv(static_cast<const Map*>(data)->end());
    }
    const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return conv(std::next(conv_in(elem), change));
    }
    std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return std::distance(conv_in(itr2), conv_in(itr1));
    }
    key_value_view_pair dereference(const void  *data, const void * elem) const override
    {
        return key_value_view_pair(*conv_in(elem));
    }
    const void * find (const void *data, string_view key) const override
    {
        return conv(static_cast<const Map*>(data)->find(key));
    }
    bool contains (const void *data, string_view key) const override
    {
        auto mp = static_cast<const Map*>(data);
        return mp->find(key) != mp->end();
    };
    std::size_t  count (const void *data, string_view key) const override
    {
        return static_cast<const Map*>(data)->count(key);
    };
};


template<typename T>
object_view::object_view(
        const T & mp,
        typename std::enable_if<
            is_map_like<T>::value
            && (sizeof(typename T::const_iterator) == sizeof(void*))
            && std::is_convertible<typename T::mapped_type, value_view>::value>::type *)
                : data_(&mp)
{
    const static map_adaptor<T> adaptor;
    adaptor_ = &adaptor;
}
#if defined(BOOST_DESCRIBE_CXX14)

template<typename Class>
struct object_view::describe_adaptor final : adaptor
{
    using descr = describe::describe_members<Class, describe::mod_public | describe::mod_inherited>;
    bool            empty(const void * data)   const override
    {
        return mp11::mp_empty<descr>::value;
    }
    std::size_t      size(const void * data)   const override
    {
        return mp11::mp_size<descr>::value;
    }
    std::size_t  max_size(const void * data)   const override
    {
        return mp11::mp_size<descr>::value;
    }
    std::size_t  capacity(const void * data)   const override
    {
        return mp11::mp_size<descr>::value;
    }
    value_view         at(const void * data, string_view key) const override
    {
        value_view res;
        mp11::mp_for_each<descr>(
                [&](auto p)
                {
                   if (key == p.name)
                       res = value_view(static_cast<const Class*>(data)->*p.pointer);
                });
        return res;
    }
    boost::optional<value_view> if_contains(const void * data, string_view key) const override
    {
        boost::optional<value_view> res;
        mp11::mp_for_each<descr>(
                [&](auto p)
                {
                    if (key == p.name)
                        res = value_view(static_cast<const Class*>(data)->*p.pointer);
                });
        return res;
    }
    const void * begin(const void *data) const override
    {
        return reinterpret_cast<const void*>(std::intptr_t(0));
    }
    const void * end  (const void *data) const override
    {
        return reinterpret_cast<const void*>(std::intptr_t(mp11::mp_size<descr>::value));
    }
    const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return reinterpret_cast<const void*>(reinterpret_cast<std::intptr_t>(elem) + 1);
    }
    std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return reinterpret_cast<std::intptr_t>(itr2) - reinterpret_cast<std::intptr_t>(itr1);
    }
    key_value_view_pair dereference(const void  *data, const void * elem) const override
    {
        auto idx = reinterpret_cast<std::intptr_t>(elem);
        auto ths = reinterpret_cast<const Class *>(data);
        boost::optional<value_view> res;
        return mp11::mp_with_index<mp11::mp_size<descr>::value>(idx,
                [&](auto idx)
                {
                   using type = mp11::mp_at_c<descr, idx>;
                   return key_value_view_pair(type::name, ths->*type::pointer);
                });
    }
    const void * find (const void *data, string_view key) const override
    {
        std::intptr_t pos = 0;
        std::intptr_t res = mp11::mp_size<descr>::value;
        mp11::mp_for_each<descr>(
                [&](auto p)
                {
                    if (key == p.name)
                        res = pos;
                    pos ++;
                });

        return reinterpret_cast<const void*>(res);
    }
    bool contains (const void *data, string_view key) const override
    {
        bool res = false;
        mp11::mp_for_each<descr>(
                [&](auto p)
                {
                    if (key == p.name)
                        res = true;
                });
        return res;
    };
    std::size_t  count (const void *data, string_view key) const override
    {
        std::size_t res = 0u;
        mp11::mp_for_each<descr>(
                [&](auto p)
                {
                    if (key == p.name)
                        res++;
                });
        return res;
    };
};


template<typename T>
object_view::object_view(
        const T & cl,
        typename std::enable_if<
                is_described_class<T>::value/* &&
                mp11::mp_all_of<describe::describe_members<T, describe::mod_public | describe::mod_inherited>,
                        member_convertible_to_value_view>::value*/
                    >::type *)
        : data_(&cl)
{
    const static describe_adaptor<T> adaptor;
    adaptor_ = &adaptor;
}

} // namespace json
} // namespace boost
#endif

#endif
