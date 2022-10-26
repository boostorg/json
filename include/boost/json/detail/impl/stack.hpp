//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_STACK_HPP
#define BOOST_JSON_DETAIL_IMPL_STACK_HPP

#include <boost/core/max_align.hpp>
#include <new>

namespace boost {
namespace json {
namespace detail {

//--------------------------------------
/*
    We put the non-trivial objects at
    the lo end of the buffer and the
    trivial objects at the hi end:

  base_        {free}     base_+cap_
    |<------>|<------>|<------>|
      size0_            size1_
*/
//--------------------------------------

struct stack::non_trivial
{
    non_trivial* next;

    virtual BOOST_JSON_DECL ~non_trivial() = 0;
    virtual non_trivial* copy(void*) noexcept = 0;
    virtual void* get() = 0;
};

template<class T>
void
stack::
push_unchecked(T const& t)
{
    BOOST_STATIC_ASSERT(
        std::is_trivial<T>::value);
    BOOST_ASSERT(
        sizeof(T) <= cap_ - (
            size0_ + size1_));
    size1_ += sizeof(T);
    std::memcpy(
        base_ + cap_ - size1_,
        &t,
        sizeof(T));
}

template<class T>
void
stack::
peek(T& t)
{
    BOOST_STATIC_ASSERT(
        std::is_trivial<T>::value);
    BOOST_ASSERT(
        size1_ >= sizeof(T));
    std::memcpy(
        &t,
        base_ + cap_ - size1_,
        sizeof(T));
}

//--------------------------------------

// trivial
template<class T>
void
stack::
push(T const& t, std::true_type)
{
    if(sizeof(T) > cap_ - (
            size0_ + size1_))
        reserve_impl(
            size0_ +
            sizeof(T) +
            size1_);
    push_unchecked(t);
}

// non-trivial
template<class T>
void
stack::
push(
    T const& t,
    std::false_type)
{
    struct alignas(core::max_align_t)
        U : non_trivial
    {
        T t_;

        explicit U(T const& t)
            : t_(t)
        {
        }

        non_trivial*
        copy(void* dest) noexcept override
        {
            return (::new(dest)
                U(t_)) + 1;
        }

        void*
        get() override
        {
            return &t_;
        }
    };
    BOOST_STATIC_ASSERT(
        ! std::is_trivial<T>::value);
    BOOST_ASSERT(
        alignof(U) <= alignof(
            core::max_align_t));
    BOOST_ASSERT((sizeof(U) % alignof(
        core::max_align_t)) == 0);
    if(sizeof(U) > cap_ - (
            size0_ + size1_))
        reserve_impl(
            size0_ +
            sizeof(U) +
            size1_);
    auto nt = ::new(
        base_ + size0_) U(t);
    nt->next = head_;
    head_ = nt;
    size0_ += sizeof(U);
}

// trivial
template<class T>
void
stack::
pop(T& t, std::true_type)
{
    BOOST_ASSERT(
        size1_ >= sizeof(T));
    peek(t);
    size1_ -= sizeof(T);
}

// non-trivial
template<class T>
void
stack::
pop(T& t, std::false_type)
{
    t = std::move(*reinterpret_cast<
        T*>(head_->get()));
    auto next = head_->next;
    head_->~non_trivial();
    head_ = next;
    if(head_)
        size0_ = reinterpret_cast<
            unsigned char const*>(head_) -
                base_;
    else
        size0_ = 0;
}

} // detail
} // json
} // boost

#endif