//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_JSON_IMPL_ARRAY_VIEW_HPP
#define BOOST_JSON_IMPL_ARRAY_VIEW_HPP

#include "boost/json/array_view.hpp"

namespace boost
{
namespace json
{

template<typename Container>
struct array_view::container_adaptor final : adaptor
{
    virtual bool            empty(const void * data)   const override
    {
        return static_cast<const Container*>(data)->empty();
    }
    virtual std::size_t      size(const void * data)   const override
    {
        return static_cast<const Container*>(data)->size();
    }
    virtual std::size_t  max_size(const void * data)   const override
    {
        return static_cast<const Container*>(data)->max_size();
    }
    virtual std::size_t  capacity(const void * data)   const override
    {
        return static_cast<const Container*>(data)->capacity();
    }
    virtual value_view         at(const void * data, std::size_t key) const override
    {
        return static_cast<const Container*>(data)->at(key);
    }
    virtual value_view     get_at(const void * data, std::size_t key) const override
    {
        return (*static_cast<const Container*>(data))[key];
    }
    virtual value_view      front(const void * data) const override
    {
        return static_cast<const Container*>(data)->front();
    }
    virtual value_view       back(const void * data) const override
    {
        return static_cast<const Container*>(data)->back();
    }
    virtual boost::optional<value_view> if_contains(const void * data, std::size_t pos) const override
    {
        auto c = static_cast<const Container*>(data);
        auto itr = std::find(c->begin(), c->end(), pos);
        if (pos < c->size())
            return value_view((*c)[pos]);
        else
            return boost::none;
    }
    virtual const void * begin(const void *data) const override
    {
        return static_cast<const Container*>(data)->data();
    }
    virtual const void * end  (const void *data) const override
    {
        return static_cast<const Container*>(data)->data() + static_cast<const Container*>(data)->size();
    }
    virtual const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return static_cast<const typename Container::value_type * >(elem) + change;
    }
    virtual std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return static_cast<const typename Container::value_type*>(itr2) -
               static_cast<const typename Container::value_type*>(itr1);
    }
    virtual value_view dereference(const void * data, const void * elem) const override
    {
        return value_view(*static_cast<const typename Container::value_type * >(elem));
    }
};


template<typename T, std::size_t Size>
struct array_view::raw_array_adaptor final : adaptor
{
    virtual bool            empty(const void * data)   const override
    {
        return Size == 0u;;
    }
    virtual std::size_t      size(const void * data)   const override
    {
        return Size;
    }
    virtual std::size_t  max_size(const void * data)   const override
    {
        return Size;
    }
    virtual std::size_t  capacity(const void * data)   const override
    {
        return Size;
    }
    virtual value_view         at(const void * data, std::size_t pos) const override
    {
        if (pos >= Size)
            throw_exception(std::out_of_range("out or array range"));
        return static_cast<const T*>(data) + pos;
    }
    virtual value_view     get_at(const void * data, std::size_t pos) const override
    {
        return pos[static_cast<const T*>(data)];
    }
    virtual value_view      front(const void * data) const override
    {
        return *static_cast<const T*>(data);
    }
    virtual value_view       back(const void * data) const override
    {
        return *(static_cast<const T*>(data) - (Size - 1));
    }
    virtual boost::optional<value_view> if_contains(const void * data, std::size_t pos) const override
    {
        auto c = static_cast<const T*>(data);
        auto itr = std::find(c, c->end(), pos);
        if (pos < c->size())
            return value_view((*c)[pos]);
        else
            return boost::none;
    }
    virtual const void * begin(const void *data) const override
    {
        return static_cast<const T*>(data);
    }
    virtual const void * end  (const void *data) const override
    {
        return static_cast<const T*>(data) + Size;
    }
    virtual const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return static_cast<const T *>(elem) + change;
    }
    virtual std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return static_cast<const T*>(itr2) -
               static_cast<const T*>(itr1);
    }
    virtual value_view dereference(const void * data, const void * elem) const override
    {
        return value_view(*static_cast<const T *>(elem));
    }
};


template<typename Tuple>
struct array_view::tuple_adaptor final : adaptor
{

    struct getter
    {
        const Tuple * tuple_;
        getter(const void * data) : tuple_(static_cast<const Tuple*>(data)) {}

        template<std::size_t Idx>
        value_view operator()(std::integral_constant<std::size_t, Idx>)
        {
            return value_view(std::get<Idx>(*tuple_));
        }
    };

    virtual bool            empty(const void * data)   const override
    {
        return std::tuple_size<Tuple>::value == 0u;
    }
    virtual std::size_t      size(const void * data)   const override
    {
        return std::tuple_size<Tuple>::value;
    }
    virtual std::size_t  max_size(const void * data)   const override
    {
        return std::tuple_size<Tuple>::value;
    }
    virtual std::size_t  capacity(const void * data)   const override
    {
        return std::tuple_size<Tuple>::value;
    }
    virtual value_view         at(const void * data, std::size_t pos) const override
    {
        if (pos >= std::tuple_size<Tuple>::value)
            throw_exception(std::out_of_range("to few tuplee elements"));

        return mp11::mp_with_index<std::tuple_size<Tuple>::value>(
                pos, getter(data));
    }
    virtual value_view     get_at(const void * data, std::size_t pos) const override
    {
        return mp11::mp_with_index<std::tuple_size<Tuple>::value>(
                pos, getter(data));
    }
    virtual value_view      front(const void * data) const override
    {
        return value_view(std::get<0>(*static_cast<const Tuple*>(data)));
    }
    virtual value_view       back(const void * data) const override
    {
        return value_view(std::get<std::tuple_size<Tuple>::value - 1>(*static_cast<const Tuple*>(data)));
    }
    virtual boost::optional<value_view> if_contains(const void * data, std::size_t pos) const override
    {
        if (pos >= std::tuple_size<Tuple>::value)
            return boost::none;
        else
            return mp11::mp_with_index<std::tuple_size<Tuple>::value>(
                    pos, getter(data));
    }
    virtual const void * begin(const void *data) const override
    {
        return reinterpret_cast<const void*>(std::intptr_t(0u));
    }
    virtual const void * end  (const void *data) const override
    {
        return reinterpret_cast<const void*>(std::intptr_t(std::tuple_size<Tuple>::value));
    }
    virtual const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return reinterpret_cast<const void*>(reinterpret_cast<std::intptr_t>(elem) + change);

    }
    virtual std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return reinterpret_cast<const std::intptr_t>(itr2) -
               reinterpret_cast<const std::intptr_t>(itr1);
    }
    virtual value_view dereference(const void* data, const void * elem) const override
    {
        return mp11::mp_with_index<std::tuple_size<Tuple>::value>(
                reinterpret_cast<std::intptr_t>(elem), getter(data));
    }
};


template<typename Range >
array_view::array_view(const Range & r,
                       typename std::enable_if<
                          std::is_constructible<value_view, decltype(*r.data())>::value
                       && !std::is_convertible<Range, string_view>::value,
                               decltype(r.size())>::type) noexcept : data_(&r)
{
    const static container_adaptor<Range> adaptor;
    adaptor_ = &adaptor;
}

template<typename Tuple >
array_view::array_view(
       const Tuple & r,
       typename std::enable_if<
               is_tuple_like<Tuple>::value &&
               mp11::mp_all_of_q<Tuple, mp11::mp_bind<std::is_constructible, value_view, mp11::_1>>::value>::type * ) noexcept
    : data_(&r)
{
    const static tuple_adaptor<Tuple> adaptor;
    adaptor_ = &adaptor;
}

template<typename T, std::size_t Size>
array_view::array_view(const T (&arr)[Size]) noexcept : data_(&arr)
{
    const static raw_array_adaptor<T, Size> adaptor;
    adaptor_ = &adaptor;
}

}
}

#endif //BOOST_JSON_IMPL_ARRAY_VIEW_HPP
