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

#include <boost/align/align.hpp>
#include <boost/static_assert.hpp>
#include <limits>
#include <memory>

namespace boost {
namespace json {
namespace detail {

struct stack::non_trivial
{
    non_trivial* next;
    std::size_t offset;

    virtual BOOST_JSON_DECL ~non_trivial() = 0;
    virtual non_trivial* relocate(void*) noexcept = 0;
    virtual void* get() = 0;
};

void
stack::
reserve_impl(std::size_t n)
{
    // caller checks this
    BOOST_ASSERT(n > cap_);

    auto const base = static_cast<unsigned char*>( sp_->allocate(n) );
    if(base_)
    {
        // copy trivials
        std::memcpy(base, base_, size_);

        // copy non-trivials
        non_trivial* src = head_;
        non_trivial* prev = nullptr;
        non_trivial* head = nullptr;
        while(src)
        {
            auto const next = src->next;
            auto const offset = src->offset;

            std::size_t const buf_offset =
                reinterpret_cast<unsigned char*>(src) - base_;
            auto dest = reinterpret_cast<non_trivial*>(
                src->relocate(base + buf_offset) );
            dest->offset = offset;
            dest->next = nullptr;

            if( prev )
                prev->next = dest;
            else
                head = dest;

            prev = dest;
            src = next;
        }
        head_ = head;

        if(base_ != buf_)
            sp_->deallocate(base_, cap_);
    }
    base_ = base;
    cap_ = n;
}

template<class T>
void
stack::
push_unchecked(T const& t)
{
    constexpr std::size_t n = sizeof(T);
    BOOST_STATIC_ASSERT( is_trivially_copy_assignable<T>::value );
    BOOST_ASSERT( n <= cap_ - size_ );
    std::memcpy( base_ + size_, &t, n );
    size_ += n;
}

template<class T>
void
stack::
peek(T& t)
{
    constexpr std::size_t n = sizeof(T);
    BOOST_STATIC_ASSERT( is_trivially_copy_assignable<T>::value );
    BOOST_ASSERT( size_ >= n );
    std::memcpy( &t, base_ + size_ - n, n );
}

//--------------------------------------

// trivial
template<class T>
void
stack::
push(T const& t, std::true_type)
{
    if( sizeof(T) > cap_ - size_ )
        reserve_impl( sizeof(T) + size_ );

    push_unchecked(t);
}

// non-trivial
template<class T>
void
stack::
push(T const& t, std::false_type)
{
    struct U
        : non_trivial
    {
        T t_;

        explicit U(T const& t)
            : t_(t)
        {
        }

        non_trivial*
        relocate(void* dest) noexcept override
        {
            auto result = ::new(dest) U(t_);
            this->~U();
            return result;
        }

        void*
        get() override
        {
            return &t_;
        }
    };

    BOOST_STATIC_ASSERT( ! is_trivially_copy_assignable<T>::value );

    constexpr std::size_t n = sizeof(U);

    void* ptr;
    std::size_t offset;
    do
    {
        std::size_t space = cap_ - size_;
        unsigned char* buf = base_ + size_;
        ptr = buf;
        if( alignment::align(alignof(U), n, ptr, space) )
        {
            offset = (reinterpret_cast<unsigned char*>(ptr) - buf) + n;
            break;
        }

        reserve_impl(size_ + n + alignof(U) - 1);
    }
    while(true);
    BOOST_ASSERT(
        (reinterpret_cast<unsigned char*>(ptr) + n - offset) ==
        (base_ + size_) );

    auto nt = ::new(ptr) U(t);
    nt->next = head_;
    nt->offset = offset;

    head_ = nt;
    size_ += offset;
}

// trivial
template<class T>
void
stack::
pop(T& t, std::true_type)
{
    BOOST_ASSERT( size_ >= sizeof(T) );
    peek(t);
    size_ -= sizeof(T);
}

// non-trivial
template<class T>
void
stack::
pop(T& t, std::false_type)
{
    auto next = head_->next;
    auto offset = head_->offset;

    t = std::move( *reinterpret_cast<T*>( head_->get() ) );
    head_->~non_trivial();

    head_ = next;
    size_ -= offset;
}

void
stack::
clear() noexcept
{
    while(head_)
    {
        auto const next = head_->next;
        head_->~non_trivial();
        head_ = next;
    }
    size_ = 0;
}

stack::
~stack()
{
    clear();
    if(base_ != buf_)
        sp_->deallocate(base_, cap_);
}

stack::
stack(
    storage_ptr sp,
    unsigned char* buf,
    std::size_t buf_size) noexcept
    : sp_(std::move(sp))
    , cap_(buf_size)
    , base_(buf)
    , buf_(buf)
{
}

} // detail
} // json
} // boost

#endif
