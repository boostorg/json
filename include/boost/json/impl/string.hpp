//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
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
        detail::string_impl& s;
        storage_ptr const& sp;

        ~cleanup()
        {
            s.destroy(sp);
        }
    };

    // We use the default storage because
    // the allocation is immediately freed.
    storage_ptr dsp;
    detail::string_impl tmp(
        first, last, dsp,
        iter_cat<InputIt>{});
    cleanup c{tmp, dsp};
    auto const off = pos - impl_.data();
    traits_type::copy(
        impl_.insert(off, tmp.size(), sp_),
        tmp.data(),
        tmp.size());
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
    detail::string_impl tmp(
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
        detail::string_impl& s;
        storage_ptr const& sp;

        ~cleanup()
        {
            s.destroy(sp);
        }
    };

    // We use the default storage because
    // the allocation is immediately freed.
    storage_ptr dsp;
    detail::string_impl tmp(
        first, last, dsp,
        std::input_iterator_tag{});
    cleanup c{tmp, dsp};
    traits_type::copy(
        impl_.append(tmp.size(), sp_),
        tmp.data(), tmp.size());
}

} // json
} // boost

#endif
