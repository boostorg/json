//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_VALUE_HPP
#define BOOST_JSON_IMPL_VALUE_HPP

#include <boost/json/error.hpp>
#include <boost/throw_exception.hpp>
#include <limits>
#include <type_traits>

namespace boost {
namespace json {

//------------------------------------------------------------------------------

class value::pointer
{
    reference t_;

public:
    pointer(reference t)
        : t_(t)
    {
    }

    reference*
    operator->() noexcept
    {
        return &t_;
    }
};

//------------------------------------------------------------------------------

class value::const_pointer
{
    const_reference t_;

public:
    const_pointer(
        const_reference t)
        : t_(t)
    {
    }

    const_reference*
    operator->() noexcept
    {
        return &t_;
    }
};

//------------------------------------------------------------------------------

class value::iterator
{
    union
    {
        array::iterator arr_it_;
        object::iterator obj_it_;
    };
    bool arr_;

    friend class value;

public:
    using value_type = value::value_type;
    using pointer = value::pointer;
    using reference = value::reference;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    ~iterator()
    {
        if(! arr_)
            obj_it_.~iterator();
    }

    iterator() noexcept
        : arr_it_{}
        , arr_(true)
    {
    }

    iterator(iterator const& other) noexcept
        : arr_(other.arr_)
    {
        if(arr_)
            arr_it_ = other.arr_it_;
        else
            ::new(&obj_it_)
                object::iterator(other.obj_it_);
    }

    iterator(object::iterator it)
        : obj_it_(it)
        , arr_(false)
    {
    }

    iterator(array::iterator it)
        : arr_it_(it)
        , arr_(true)
    {
    }

    iterator&
    operator=(iterator const& other) noexcept
    {
        if(! arr_)
            obj_it_.~iterator();
        arr_ = other.arr_;
        if(arr_)
            arr_it_ = other.arr_it_;
        else
            ::new(&obj_it_)
                object::iterator(other.obj_it_);
        return *this;
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        if(arr_ != other.arr_)
            return false;
        if(arr_)
            return arr_it_ == other.arr_it_;
        return obj_it_ == other.obj_it_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return ! (*this == other);
    }

    bool
    operator==(
        const_iterator const& other) const noexcept;

    bool
    operator!=(
        const_iterator const& other) const noexcept;

    iterator&
    operator++() noexcept
    {
        if(arr_)
            ++arr_it_;
        else
            ++obj_it_;
        return *this;
    }

    iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    iterator&
    operator--() noexcept
    {
        if(arr_)
            --arr_it_;
        else
            --obj_it_;
        return *this;
    }

    iterator
    operator--(int) noexcept
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    reference
    operator*() const noexcept
    {
        if(arr_)
            return reference{
                "", *arr_it_ };
        return reference{
            obj_it_->first,
            obj_it_->second };
    }

    pointer
    operator->() const noexcept
    {
        return *(*this);
    }
};

//------------------------------------------------------------------------------

class value::const_iterator
{
    union
    {
        array::const_iterator arr_it_;
        object::const_iterator obj_it_;
    };
    bool arr_;

    friend class value;

public:
    using value_type = value::value_type;
    using pointer = value::const_pointer;
    using reference = value::const_reference;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    ~const_iterator()
    {
        if(! arr_)
            obj_it_.~const_iterator();
    }

    const_iterator() noexcept
        : arr_it_{}
        , arr_(true)
    {
    }

    const_iterator(
        const_iterator const& other) noexcept
        : arr_(other.arr_)
    {
        if(arr_)
            arr_it_ = other.arr_it_;
        else
            ::new(&obj_it_)
                object::const_iterator(
                    other.obj_it_);
    }

    const_iterator(
        object::const_iterator it)
        : obj_it_(it)
        , arr_(false)
    {
    }

    const_iterator(
        array::const_iterator it)
        : arr_it_(it)
        , arr_(true)
    {
    }

    const_iterator&
    operator=(const_iterator const& other) noexcept
    {
        if(! arr_)
            obj_it_.~const_iterator();
        arr_ = other.arr_;
        if(arr_)
            arr_it_ = other.arr_it_;
        else
            ::new(&obj_it_)
                object::const_iterator(
                    other.obj_it_);
        return *this;
    }

    bool
    operator==(
        const_iterator const& other) const noexcept
    {
        if(arr_ != other.arr_)
            return false;
        if(arr_)
            return arr_it_ == other.arr_it_;
        return obj_it_ == other.obj_it_;
    }

    bool
    operator!=(
        const_iterator const& other) const noexcept
    {
        return ! (*this == other);
    }

    bool
    operator==(
        iterator const& other) const noexcept;

    bool
    operator!=(
        iterator const& other) const noexcept;

    const_iterator&
    operator++() noexcept
    {
        if(arr_)
            ++arr_it_;
        else
            ++obj_it_;
        return *this;
    }

    const_iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    const_iterator&
    operator--() noexcept
    {
        if(arr_)
            --arr_it_;
        else
            --obj_it_;
        return *this;
    }

    const_iterator
    operator--(int) noexcept
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    reference
    operator*() const noexcept
    {
        if(arr_)
            return reference{
                "", *arr_it_ };
        return reference{
            obj_it_->first,
            obj_it_->second };
    }

    pointer
    operator->() const noexcept
    {
        return *(*this);
    }
};

//------------------------------------------------------------------------------

struct value::undo
{
    union
    {
        value old;
    };
    value* cur;

    explicit
    undo(value* cur_) noexcept
        : cur(cur_)
    {
        relocate(&old, *cur);
    }

    void
    commit() noexcept
    {
        old.~value();
        cur = nullptr;
    }

    ~undo()
    {
        if(cur)
            relocate(cur, old);
    }
};

//------------------------------------------------------------------------------

template<class M>
auto
value::
insert_or_assign(
    key_type key, M&& obj) ->
        std::pair<iterator, bool>
{
    BOOST_ASSERT(is_object());
    return obj_.insert_or_assign(
        key, std::forward<M>(obj));
}

template<class M>
auto
value::
insert_or_assign(
    const_iterator before,
    key_type key,
    M&& obj) ->
        std::pair<iterator, bool>
{
    BOOST_ASSERT(is_object());
    BOOST_ASSERT(! before.arr_);
    return obj_.insert_or_assign(
        before.obj_it_, key,
            std::forward<M>(obj));
}

template<class Arg>
auto
value::
emplace(key_type key, Arg&& arg) ->
    std::pair<iterator, bool>
{
    BOOST_ASSERT(is_object());
    return obj_.emplace(key,
        std::forward<Arg>(arg));
}

template<class Arg>
auto
value::
emplace(
    const_iterator before,
    key_type key, Arg&& arg) ->
        std::pair<iterator, bool>
{
    BOOST_ASSERT(is_object());
    BOOST_ASSERT(! before.arr_);
    return obj_.emplace(
        key, before.obj_it_,
            std::forward<Arg>(arg));
}

template<class Arg>
auto
value::
emplace(
    const_iterator before,
    Arg&& arg) ->
        iterator
{
    BOOST_ASSERT(is_array());
    BOOST_ASSERT(before.arr_);
    return arr_.emplace(before.arr_it_,
        std::forward<Arg>(arg));
}

template<class Arg>
value&
value::
emplace_back(Arg&& arg)
{
    BOOST_ASSERT(is_array());
    return *arr_.emplace(arr_.end(),
        std::forward<Arg>(arg));
}

//------------------------------------------------------------------------------

namespace detail {

template<class T, class = void>
struct is_range : std::false_type
{
};

template<class T>
struct is_range<T, boost::void_t<
    typename T::value_type,
    decltype(
        std::declval<T const&>().begin(),
        std::declval<T const&>().end()
    )>> : std::true_type
{
};

} // detail

//------------------------------------------------------------------------------
//
// assign to value
//

// range
template<class T
    ,class = typename std::enable_if<
        detail::is_range<T>::value
        && ! std::is_same<typename T::value_type, char>::value
        && has_to_json<typename T::value_type>::value
            >::type
>
void
to_json(T const& t, value& v)
{
    v.reset(json::kind::array);
    for(auto const& e : t)
        v.as_array().push_back(e);
}

// string
inline
void
to_json(string_view t, value& v)
{
    v.emplace_string().assign(
        t.data(), t.size());
}

// string
inline
void
to_json(char const* t, value& v)
{
    v.emplace_string() = t;
}

// number
template<class T
    ,class = typename std::enable_if<
        std::is_constructible<number, T>::value &&
        ! std::is_same<number, T>::value>::type
>
inline
void
to_json(T t, value& v)
{
    v.emplace_number() = t;
}

// bool
inline
void
to_json(bool b, value& v)
{
    v.emplace_bool() = b;
}

// null
inline
void
to_json(std::nullptr_t, value& v)
{
    v.emplace_null();
}

//------------------------------------------------------------------------------
//
// assign value to
//

// integer

template<typename T
    ,class = typename std::enable_if<
        std::is_integral<T>::value>::type
>
void
from_json(T& t, value const& v)
{
    if(v.is_int64())
    {
        auto const rhs = v.get_int64();
        if( rhs > (std::numeric_limits<T>::max)() ||
            rhs < (std::numeric_limits<T>::min)())
            throw system_error(
                error::integer_overflow);
        t = static_cast<T>(rhs);
    }
    else if(v.is_uint64())
    {
        auto const rhs = v.get_uint64();
        if(rhs > (std::numeric_limits<T>::max)())
            throw system_error(
                error::integer_overflow);
        t = static_cast<T>(rhs);
    }
    else
    {
        throw system_error(
            error::expected_number);
    }
}

} // json
} // boost

#endif
