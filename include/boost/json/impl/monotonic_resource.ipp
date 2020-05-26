//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_MONOTONIC_RESOURCE_IPP
#define BOOST_JSON_IMPL_MONOTONIC_RESOURCE_IPP

#include <boost/json/monotonic_resource.hpp>

#include <memory>

namespace boost {
namespace json {

// ensures that the alignment of base is
// the strictest fundamental alignment requirement
struct alignas(detail::max_align())
    monotonic_resource::block
{
    std::size_t size;
    block* next = nullptr;
    unsigned char* base;
    unsigned char* top;

    block(
        unsigned char* data,
        std::size_t n,
        block* head)
        : size(n)
        , next(head)
        , base(data)
        , top(data)
    {
    }
};

template<typename Block>
void*
monotonic_resource::
allocate_in_block(
    Block& b,
    std::size_t size,
    std::size_t align)
{
    const auto aligned = 
        detail::align_up(b.top, align);
    const auto next_top = aligned + size;
    if(next_top <= b.base + b.size)
    {
        b.top = next_top;
        return aligned;
    }
    return nullptr;
}

auto
monotonic_resource::
allocate_new_block(std::size_t size) ->
    block&
{  
    const auto bytes = size + sizeof(block);
    const auto data = static_cast<unsigned char*>(
        ::operator new(bytes));
    head_ = ::new(static_cast<void*>(data))
        block(sizeof(block) + data, size, head_);
    return *head_;
}

// returns the closest power of two
// that is greater than requested
std::size_t
monotonic_resource::
next_block_size(std::size_t requested)
{
    if(requested >= max_block_size_ >> 1)
        return max_block_size_;
    if(requested <= min_block_size_)
        return min_block_size_;
    std::size_t next = 1;
    while(requested)
    {
        requested >>= 1;
        next <<= 1;
    }
    return next;
}

// skips rounding if requested
// is already a power of two
std::size_t
monotonic_resource::
closest_block_size(std::size_t requested)
{
    if(requested & (requested - 1))
        return next_block_size(requested);
    return requested;
}

std::size_t
monotonic_resource::
grow_block_size(std::size_t size)
{
    // prevents overflow
    if(size >= max_block_size_ >> 1)
        return max_block_size_;
    return size << 1;
}

monotonic_resource::
~monotonic_resource() noexcept
{
    for(auto b = head_; b;)
    {
        auto next = b->next;
        ::operator delete(b);
        b = next;
    }
}

monotonic_resource::
monotonic_resource(
    std::size_t initial_size) noexcept
    : block_size_(closest_block_size(initial_size))
{
}

monotonic_resource::
monotonic_resource(
    void* buffer,
    std::size_t buffer_size) noexcept
    : block_size_(next_block_size(buffer_size))
    , initial_(buffer, buffer_size)
{
}

void*
monotonic_resource::
do_allocate(
    std::size_t n,
    std::size_t align)
{
    if(initial_.base)
        if(auto p = allocate_in_block(initial_, n, align))
            return p;
    if(head_)
        if(auto p = allocate_in_block(*head_, n, align))
            return p;
    auto new_block_size = block_size_;
    if(n > block_size_)
        new_block_size = closest_block_size(n);
    auto p = allocate_in_block(
        allocate_new_block(new_block_size), n, align);
    block_size_ = grow_block_size(new_block_size);
    BOOST_ASSERT(p);
    return p;
}

void
monotonic_resource::
do_deallocate(
    void*,
    std::size_t,
    std::size_t)
{
    // do nothing
}

bool
monotonic_resource::
do_is_equal(
    memory_resource const& mr) const noexcept
{
    return this == &mr;
}

} // json
} // boost

#endif
