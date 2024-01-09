//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_JSON_IMPL_OBJECT_VIEW_IPP
#define BOOST_JSON_IMPL_OBJECT_VIEW_IPP

#include <boost/json/value_view.hpp>

namespace boost {
namespace json {

struct object_view::empty_adaptor final : adaptor
{
    bool            empty(const void * data)   const override
    {
        return true;
    }
    std::size_t      size(const void * data)   const override
    {
        return 0u;
    }
    std::size_t  max_size(const void * data)   const override
    {
        return 0u;
    }
    std::size_t  capacity(const void * data)   const override
    {
        return 0u;
    }
    value_view         at(const void * data, string_view key) const override
    {
        return value_view();
    }
    boost::optional<value_view> if_contains(const void * data, string_view key) const override
    {
        return boost::none;
    }
    const void * begin(const void *data) const override
    {
        return nullptr;
    }
    const void * end  (const void *data) const override
    {
        return nullptr;
    }
    const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return reinterpret_cast<const void*>(reinterpret_cast<std::intptr_t>(elem) + change) ;
    }
    std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return reinterpret_cast<std::intptr_t>(itr2) - reinterpret_cast<std::intptr_t>(itr1);
    }
    key_value_view_pair dereference(const void * data, const void * elem) const override
    {
        BOOST_ASSERT(!"Can't dereference empty value");
    }

    const void * find (const void *data, string_view key) const override
    {
        return nullptr;
    }
    bool contains (const void *data, string_view key) const override
    {
        return false;
    };
    std::size_t  count (const void *data, string_view key) const override
    {
        return 0u;
    };
};

struct object_view::object_adaptor final : adaptor
{
    bool            empty(const void * data)   const override
    {
        return static_cast<const json::object*>(data)->empty();
    }
    std::size_t      size(const void * data)   const override
    {
        return static_cast<const json::object*>(data)->size();
    }
    std::size_t  max_size(const void * data)   const override
    {
        return static_cast<const json::object*>(data)->max_size();
    }
    std::size_t  capacity(const void * data)   const override
    {
        return static_cast<const json::object*>(data)->capacity();
    }
    value_view         at(const void * data, string_view key) const override
    {
        return static_cast<const json::object*>(data)->at(key);
    }
    boost::optional<value_view> if_contains(const void * data, string_view key) const override
    {
        auto p = static_cast<const json::object*>(data)->if_contains(key);
        if (p != nullptr)
            return value_view(*p);
        else
            return boost::none;
    }
    const void * begin(const void *data) const override
    {
        return static_cast<const json::object*>(data)->begin();
    }
    const void * end  (const void *data) const override
    {
        return static_cast<const json::object*>(data)->end();
    }
    const void * next (const void* elem, std::ptrdiff_t change) const override
    {
        return static_cast<json::object::const_iterator>(elem) + change;
    }
    std::intptr_t distance(const void* itr1, const void * itr2) const override
    {
        return static_cast<json::object::const_iterator>(itr2) - static_cast<json::object::const_iterator>(itr1);
    }
    key_value_view_pair dereference(const void * data, const void * elem) const override
    {
        return key_value_view_pair(*static_cast<json::object::const_iterator>(elem));
    }
    const void * find (const void *data, string_view key) const override
    {
        return static_cast<const json::object*>(data)->find(key);
    }
    bool contains (const void *data, string_view key) const override
    {
        return static_cast<const json::object*>(data)->contains(key);
    };
    std::size_t  count (const void *data, string_view key) const override
    {
        return static_cast<const json::object*>(data)->count(key);
    };
};

const object_view::empty_adaptor object_view::empty_impl_{};
const object_view::object_adaptor object_view::object_impl_{};



object_view::object_view() noexcept : data_(nullptr), adaptor_(&empty_impl_)
{
}

object_view::object_view(const json::object & arr) noexcept : data_(&arr), adaptor_(&object_impl_)
{
}


boost::optional<value_view>
object_view::if_contains(string_view pos) const noexcept
{
    return adaptor_->if_contains(data_, pos);
}

key_value_view_pair object_view::const_iterator::operator*() const
{
    return adaptor_->dereference(data, elem);
}
key_value_view_pair object_view::const_iterator::operator[](std::size_t pos) const
{
    return adaptor_->dereference(data, adaptor_->next(elem, pos));
}


value_view
object_view::at(string_view pos) const
{
    return adaptor_->at(data_, pos);
}



bool
object_view::equal(object_view const& other) const noexcept
{
    if(size() != other.size())
        return false;
    auto const end_ = other.end();
    for(auto e : *this)
    {
        auto it = other.find(e.key());
        if(it == end_)
            return false;
        if((*it).value() != e.value())
            return false;
    }
    return true;}


} // namespace json
} // namespace boost


//----------------------------------------------------------
//
// std::hash specialization
//
//----------------------------------------------------------

std::size_t
std::hash<::boost::json::object_view>::operator()(
        ::boost::json::object_view const& ja) const noexcept
{
    return ::boost::hash< ::boost::json::object_view >()( ja );
}

//----------------------------------------------------------
#endif //BOOST_JSON_IMPL_OBJECT_VIEW_IPP
