//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STRING_HPP
#define BOOST_JSON_IMPL_STRING_HPP

namespace boost {
namespace json {

//------------------------------------------------------------------------------

struct string::impl
{
    size_type size;
    size_type capacity;

    static
    impl*
    construct(
        size_type capacity,
        storage_ptr const& sp)
    {
        auto s = ::new(sp->allocate(
            sizeof(impl) + capacity + 1,
            sizeof(impl))) impl;
        s->capacity = capacity;
        return s;
    }

    static
    impl*
    construct(
        size_type size,
        size_type capacity,
        storage_ptr const& sp)
    {
        auto s = construct(capacity, sp);
        s->term(size);
        return s;
    }

    static
    void
    destroy(
        impl* s,
        storage_ptr const& sp) noexcept
    {
        sp->deallocate(s,
            sizeof(impl) + s->capacity + 1,
            sizeof(impl));
    }

    char*
    data() noexcept
    {
        return reinterpret_cast<
            char*>(this+1);
    }

    char*
    end() noexcept
    {
        return data() + size;
    }

    void
    term() noexcept
    {
        data()[size] = 0;
    }

    void
    term(size_type n) noexcept
    {
        size = n;
        data()[size] = 0;
    }
};

//------------------------------------------------------------------------------

template<class InputIt, class>
string::
string(
    InputIt first,
    InputIt last,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    assign(first, last);
}

template<class InputIt, class>
string&
string::
assign(
    InputIt first,
    InputIt last)
{
    maybe_raw_resize(
        first, last, iter_cat<InputIt>{});
    char* p = s_->data();
    while(first != last)
        *p++ = *first++;
    *p = 0;
    return *this;
}

template<class InputIt, class>
string&
string::
append(InputIt, InputIt)
{
    return *this;
}

} // json
} // boost

#endif
