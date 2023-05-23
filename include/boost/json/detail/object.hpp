//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_OBJECT_HPP
#define BOOST_JSON_DETAIL_OBJECT_HPP

#include <boost/json/storage_ptr.hpp>
#include <boost/json/string_view.hpp>
#include <cstdlib>

namespace boost {
namespace json {

class object;
class value;
class key_value_pair;

namespace detail {

class unchecked_object
{
    // each element is two values,
    // first one is a string key,
    // second one is the value.
    value* data_;
    value* end_;
    storage_ptr const& sp_;
    bool ignore_duplicates_;

public:
    inline
    ~unchecked_object();

    inline
    unchecked_object(
        value* data,
        std::size_t size, // # of kv-pairs
        storage_ptr const& sp,
        bool ignore_duplicates) noexcept;

    unchecked_object(
        unchecked_object&& other) noexcept
        : data_(other.data_)
        , end_(other.end_)
        , sp_(other.sp_)
        , ignore_duplicates_(other.ignore_duplicates_)
    {
        other.data_ = other.end_ = nullptr;
    }

    storage_ptr const&
    storage() const noexcept
    {
        return sp_;
    }

    inline
    std::size_t
    size() const noexcept;

    bool
    ignore_duplicate_keys() const noexcept
    {
        return ignore_duplicates_;
    }

    value*
    front() noexcept
    {
        return data_;
    }

    inline
    void
    pop_front() noexcept;
};

template<class CharRange>
std::pair<key_value_pair*, std::size_t>
find_in_object(
    object const& obj,
    CharRange key) noexcept;

extern template
BOOST_JSON_DECL
std::pair<key_value_pair*, std::size_t>
find_in_object<string_view>(
    object const&,
    string_view key) noexcept;

template< bool SmallTable, bool IgnoreDuplicates >
void init_from_unchecked( object& obj, unchecked_object& uo );

} // detail
} // namespace json
} // namespace boost

#endif
