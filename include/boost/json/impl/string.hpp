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

#include <boost/core/ignore_unused.hpp>
#include <algorithm>
#include <utility>
#include <string>

namespace boost {
namespace json {

//----------------------------------------------------------

template<class InputIt>
void
string::
impl::
construct(
    InputIt first,
    InputIt last,
    storage_ptr const& sp,
    std::forward_iterator_tag)
{
    auto const n =
        std::distance(first, last);
    auto dest = construct(n, sp);
    while(first != last)
        *dest++ = *first++;
    *dest++ = 0;
    size = static_cast<
        impl_size_type>(n);
}

template<class InputIt>
void
string::
impl::
construct(
    InputIt first,
    InputIt last,
    storage_ptr const& sp,
    std::input_iterator_tag)
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
    auto dest = construct(1, sp);
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
{
    s_.construct(
        first, last, sp_,
        iter_cat<InputIt>{});
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

    impl tmp;
    tmp.construct(first, last,
        detail::global_storage(),
        iter_cat<InputIt>{});
    cleanup c{tmp,
        detail::global_storage()};
    auto const off = pos - s_.data();
    traits_type::copy(
        s_.insert(off, tmp.size, sp_),
        tmp.data(),
        tmp.size);
    return s_.data() + off;
}

//----------------------------------------------------------

template<class InputIt>
void
string::
assign(
    InputIt first,
    InputIt last,
    std::forward_iterator_tag)
{
    auto dest = s_.assign(
        std::distance(first, last), sp_);
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
        s_.term(0);
        return;
    }
    impl tmp;
    tmp.construct(
        first, last, sp_,
        std::input_iterator_tag{});
    s_.destroy(sp_);
    s_ = tmp;
}

template<class InputIt>
void
string::
append(
    InputIt first,
    InputIt last,
    std::forward_iterator_tag)
{
    auto const n =
        std::distance(first, last);
    std::copy(first, last, s_.append(n, sp_));
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

    impl tmp;
    tmp.construct(first, last,
        detail::global_storage(),
        std::input_iterator_tag{});
    cleanup c{tmp,
        detail::global_storage()};
    traits_type::copy(
        s_.append(tmp.size, sp_),
        tmp.data(), tmp.size);
}

} // json
} // boost

#endif
