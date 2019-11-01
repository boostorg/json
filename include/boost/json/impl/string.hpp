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

#include <utility>
#include <string>

namespace boost {
namespace json {

//----------------------------------------------------------

template<class InputIt>
string::
impl::
impl(
    InputIt first,
    InputIt last,
    storage_ptr const& sp,
    std::random_access_iterator_tag)
    : impl(static_cast<size_type>(
        last - first), sp)
{
    std::copy(
        first, last, data());
    data()[size] = 0;
}

template<class InputIt>
string::
impl::
impl(
    InputIt first,
    InputIt last,
    storage_ptr const& sp,
    std::input_iterator_tag)
    : impl()
{
    struct undo
    {
        impl& s;
        storage_ptr const& sp;
        bool commit;

        ~undo()
        {
            if(! commit)
                s.destroy(sp);
        }
    };

    undo u{*this, sp, false};
    auto dest = data();
    size = 1;
    *dest++ = *first++;
    while(first != last)
    {
        if(size < capacity)
            ++size;
        else
            dest = append(1, sp);
        *dest++ = *first++;
    }
    *dest = 0;
    u.commit = true;
}

//----------------------------------------------------------

template<class InputIt, class>
string::
string(
    InputIt first,
    InputIt last,
    storage_ptr sp)
    : sp_(std::move(sp))
    , impl_(first, last, sp_,
        iter_cat<InputIt>{})
{
}

template<class InputIt, class>
string&
string::
assign(
    InputIt first,
    InputIt last)
{
    assign(first, last,
        iter_cat<InputIt>{});
    return *this;
}

template<class InputIt, class>
string&
string::
append(InputIt first, InputIt last)
{
    append(first, last,
        iter_cat<InputIt>{});
    return *this;
}

template<class InputIt, class>
auto
string::
insert(
    const_iterator pos,
    InputIt first,
    InputIt last) ->
        iterator
{
    struct cleanup
    {
        impl& s;
        storage_ptr const& sp;

        ~cleanup()
        {
            s.destroy(sp);
        }
    };

    // We use the global storage since
    // it is a temporary deallocated here.
    impl tmp(
        first, last,
        storage_ptr{},
        iter_cat<InputIt>{});
    cleanup c{tmp, storage_ptr{}};
    auto const off = pos - impl_.data();
    traits_type::copy(
        impl_.insert(off, tmp.size, sp_),
        tmp.data(),
        tmp.size);
    return impl_.data() + off;
}

//----------------------------------------------------------

template<class InputIt>
void
string::
assign(
    InputIt first,
    InputIt last,
    std::random_access_iterator_tag)
{
    auto dest = impl_.assign(static_cast<
        size_type>(last - first), sp_);
    while(first != last)
        *dest++ = *first++;
}

template<class InputIt>
void
string::
assign(
    InputIt first,
    InputIt last,
    std::input_iterator_tag)
{
    if(first == last)
    {
        impl_.term(0);
        return;
    }
    impl tmp(
        first, last, sp_,
        std::input_iterator_tag{});
    impl_.destroy(sp_);
    impl_ = tmp;
}

template<class InputIt>
void
string::
append(
    InputIt first,
    InputIt last,
    std::random_access_iterator_tag)
{
    auto const n = static_cast<
        size_type>(last - first);
    std::copy(first, last,
        impl_.append(n, sp_));
}

template<class InputIt>
void
string::
append(
    InputIt first,
    InputIt last,
    std::input_iterator_tag)
{
    struct cleanup
    {
        impl& s;
        storage_ptr const& sp;

        ~cleanup()
        {
            s.destroy(sp);
        }
    };

    // We use the global storage since
    // it is a temporary deallocated here.
    impl tmp(
        first, last,
        storage_ptr{},
        std::input_iterator_tag{});
    cleanup c{tmp, storage_ptr{}};
    traits_type::copy(
        impl_.append(tmp.size, sp_),
        tmp.data(), tmp.size);
}

} // json
} // boost

#endif
