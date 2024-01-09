//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_ARRAY_VIE_HPP
#define BOOST_JSON_DETAIL_IMPL_ARRAY_VIE_HPP

#include <boost/json/array_view.hpp>
#include <boost/json/value_view.hpp>
#include <boost/container_hash/hash.hpp>

namespace boost {
namespace json {

struct array_view::empty_adaptor final : adaptor
{
    virtual bool            empty(const void * data)   const override
    {
        return true;
    }
    virtual std::size_t      size(const void * data)   const override
    {
        return 0u;
    }
    virtual std::size_t  max_size(const void * data)   const override
    {
        return 0u;
    }
    virtual std::size_t  capacity(const void * data)   const override
    {
        return 0u;
    }
    virtual value_view         at(const void * data, std::size_t key) const override
    {
        return value_view();
    }
    virtual value_view     get_at(const void * data, std::size_t key) const override
    {
        return value_view();
    }
    virtual value_view      front(const void * data) const override
    {
        return value_view();
    }
    virtual value_view       back(const void * data) const override
    {
        return value_view();
    }
    virtual boost::optional<value_view> if_contains(const void * data, std::size_t key) const override
    {
        return boost::none;
    }
    virtual const void * begin(const void *data) const override
    {
        return nullptr;
    }
    virtual const void * end  (const void *data) const override
    {
        return nullptr;
    }
    virtual const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return reinterpret_cast<const void*>(reinterpret_cast<std::intptr_t>(elem) + change) ;
    }
    virtual std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return reinterpret_cast<std::intptr_t>(itr2) - reinterpret_cast<std::intptr_t>(itr1);
    }
    virtual value_view dereference(const void * data, const void * elem) const override
    {
        BOOST_ASSERT(!"Can't dereference empty value");
    }
};

struct array_view::array_adaptor final : adaptor
{
    virtual bool            empty(const void * data)   const override
    {
        return static_cast<const json::array*>(data)->empty();
    }
    virtual std::size_t      size(const void * data)   const override
    {
        return static_cast<const json::array*>(data)->size();
    }
    virtual std::size_t  max_size(const void * data)   const override
    {
        return static_cast<const json::array*>(data)->max_size();
    }
    virtual std::size_t  capacity(const void * data)   const override
    {
        return static_cast<const json::array*>(data)->capacity();
    }
    virtual value_view         at(const void * data, std::size_t key) const override
    {
        return static_cast<const json::array*>(data)->at(key);
    }
    virtual value_view     get_at(const void * data, std::size_t key) const override
    {
        return (*static_cast<const json::array*>(data))[key];
    }
    virtual value_view      front(const void * data) const override
    {
        return static_cast<const json::array*>(data)->front();
    }
    virtual value_view       back(const void * data) const override
    {
        return static_cast<const json::array*>(data)->back();
    }
    virtual boost::optional<value_view> if_contains(const void * data, std::size_t pos) const override
    {
        auto p = static_cast<const json::array*>(data)->if_contains(pos);
        if (p != nullptr)
            return value_view(*p);
        else
            return boost::none;
    }
    virtual const void * begin(const void *data) const override
    {
        return static_cast<const json::array*>(data)->begin();
    }
    virtual const void * end  (const void *data) const override
    {
        return static_cast<const json::array*>(data)->end();
    }
    virtual const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return static_cast<json::array::const_iterator>(elem) + change;
    }
    virtual std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return static_cast<json::array::const_iterator>(itr2) - static_cast<json::array::const_iterator>(itr1);
    }
    virtual value_view dereference(const void * data, const void * elem) const override
    {
        return value_view(*static_cast<json::array::const_iterator>(elem));
    }
};

const array_view::empty_adaptor array_view::empty_impl_{};
const array_view::array_adaptor array_view::array_impl_{};


array_view::array_view() noexcept : data_(nullptr), adaptor_(&empty_impl_)
{
}

array_view::array_view(const json::array & arr) noexcept : data_(&arr), adaptor_(&array_impl_)
{
}


const boost::optional<value_view>
array_view::if_contains(std::size_t pos) const noexcept
{
    return adaptor_->if_contains(data_, pos);
}

value_view array_view::const_iterator::operator*() const
{
    return adaptor_->dereference(data, elem);
}
value_view array_view::const_iterator::operator[](std::size_t pos) const
{
    return adaptor_->dereference(data, adaptor_->next(elem, pos));
}


value_view
array_view::at(std::size_t pos) const
{
    return adaptor_->at(data_, pos);
}


value_view
array_view::operator[](std::size_t pos) const noexcept
{
    return adaptor_->get_at(data_, pos);
}

value_view
array_view::front() const noexcept
{
    return adaptor_->front(data_);
}

value_view
array_view::back() const noexcept
{
    return adaptor_->back(data_);
}

bool
array_view::equal(array_view const& other) const noexcept
{
    return std::equal(begin(), end(), other.begin(), other.end());
}


} // namespace json
} // namespace boost


//----------------------------------------------------------
//
// std::hash specialization
//
//----------------------------------------------------------

std::size_t
std::hash<::boost::json::array_view>::operator()(
        ::boost::json::array_view const& ja) const noexcept
{
    return ::boost::hash< ::boost::json::array_view >()( ja );
}

//----------------------------------------------------------

#endif
