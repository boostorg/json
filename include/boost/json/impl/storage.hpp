//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_HPP
#define BOOST_JSON_IMPL_STORAGE_HPP

#include <boost/json/detail/storage_adaptor.hpp>
#include <boost/core/exchange.hpp>
#include <utility>

namespace boost {
namespace json {

#ifndef BOOST_JSON_TRACK_STORAGE

template<class Storage, class... Args>
std::shared_ptr<Storage>
make_storage(Args&&... args)
{
    return std::make_shared<Storage>(
        std::forward<Args>(args)...);
}

//------------------------------------------------------------------------------

#else

template<class T>
void
basic_storage_ptr<T>::
increment() const noexcept
{
    ++*pn_;
}

template<class T>
void
basic_storage_ptr<T>::
decrement() const noexcept
{
    if(--*pn_ > 0)
        return;
    delete pn_;
    delete pv_;
}

template<class T>
basic_storage_ptr<T>::
basic_storage_ptr(T* t) noexcept
    : pv_(t)
    , pn_(new count(1))
{
}

template<class T>
basic_storage_ptr<T>::
~basic_storage_ptr()
{
    if(pn_)
        decrement();
}

template<class T>
basic_storage_ptr<T>::
basic_storage_ptr(
    basic_storage_ptr&& other) noexcept
    : pv_(boost::exchange(other.pv_, nullptr))
    , pn_(boost::exchange(other.pn_, nullptr))
{
}

template<class T>
basic_storage_ptr<T>::
basic_storage_ptr(
    basic_storage_ptr const& other) noexcept
    : pv_(other.pv_)
    , pn_(other.pn_)
{
    if(pn_)
        increment();
}

template<class T>
template<class U, class>
basic_storage_ptr<T>::
basic_storage_ptr(
    basic_storage_ptr<U>&& sp) noexcept
    : pv_(boost::exchange(sp.pv_, nullptr))
    , pn_(boost::exchange(sp.pn_, nullptr))
{
}

template<class T>
template<class U, class>
basic_storage_ptr<T>::
basic_storage_ptr(
    basic_storage_ptr<U> const& sp) noexcept
    : pv_(sp.pv_)
    , pn_(sp.pn_)
{
    if(pn_)
        increment();
}

template<class T>
basic_storage_ptr<T>::
basic_storage_ptr(std::nullptr_t) noexcept
{
    if(pn_)
    {
        decrement();
        pv_ = nullptr;
        pn_ = nullptr;
    }
}

template<class T>
basic_storage_ptr<T>&
basic_storage_ptr<T>::
operator=(basic_storage_ptr&& other) noexcept
{
    BOOST_ASSERT(this != &other);
    if(pn_)
        decrement();
    pv_ = boost::exchange(other.pv_, nullptr);
    pn_ = boost::exchange(other.pn_, nullptr);
    return *this;
}

template<class T>
basic_storage_ptr<T>&
basic_storage_ptr<T>::
operator=(
    basic_storage_ptr const& other) noexcept
{
    if(other.pn_)
        other.increment();
    if(pn_)
        decrement();
    pv_ = other.pv_;
    pn_ = other.pn_;
    return *this;
}

template<class Storage, class... Args>
basic_storage_ptr<Storage>
make_storage(Args&&... args)
{
    return basic_storage_ptr<Storage>(
        new Storage(std::forward<Args>(args)...));
}

#endif

template<class Allocator>
storage_ptr
make_storage_adaptor(Allocator const& a)
{
    return make_storage<
        detail::storage_adaptor<Allocator>>(a);
}

} // json
} // boost

#endif
