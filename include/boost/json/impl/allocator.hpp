//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_ALLOCATOR_HPP
#define BOOST_JSON_IMPL_ALLOCATOR_HPP

#include <memory>
#include <utility>

namespace boost {
namespace json {

template<typename T>
allocator<T>::
allocator()
    : sp_(default_storage())
{
}

template<typename T>
allocator<T>::
allocator(allocator&& other) noexcept
    : allocator(other)
{
    // Postcondition: *this == other
}

template<typename T>
auto
allocator<T>::
operator=(allocator&& other) noexcept ->
    allocator&
{
    // Postcondition: *this == other
    sp_ = other.sp_;
    return *this;
}

template<typename T>
template<typename U>
allocator<T>::
allocator(allocator<U> const& other) noexcept
    : sp_(other.sp_)
{
}

template<typename T>
allocator<T>::
allocator(storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
}

template<typename T>
T*
allocator<T>::
allocate(size_t n)
{
    return static_cast<T*>(sp_->allocate(
        n * sizeof(T), alignof(T)));
}

template<typename T>
void
allocator<T>::
deallocate(T* p, size_t n) noexcept
{
    sp_->deallocate(
        p, n * sizeof(T), alignof(T));
}

template<typename T>
bool
allocator<T>::
operator==(allocator const& other) const noexcept
{
    return *sp_ == *other.sp_;
}

template<typename T>
bool
allocator<T>::
operator!=(allocator const& other) const noexcept
{
    return *sp_ != *other.sp_;
}

} // json
} // boost

#endif
