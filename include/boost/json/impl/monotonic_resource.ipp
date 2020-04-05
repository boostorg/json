//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_MONOTONIC_RESOURCE_IPP
#define BOOST_JSON_IMPL_MONOTONIC_RESOURCE_IPP

#include <boost/json/monotonic_resource.hpp>

namespace boost {
namespace json {

struct monotonic_resource::block
{
    std::size_t const size;
    std::uintptr_t top;
    block* next;

    block(
        std::size_t size_,
        block* next_)
        : size(size_)
        , top(reinterpret_cast<
            std::uintptr_t>(this+1))
        , next(next_)
    {
    }

    void*
    alloc(
        std::size_t n,
        std::size_t align) noexcept
    {
        // must be power of 2
        BOOST_ASSERT(
            (align & (align - 1)) == 0);
        auto i = top;
        if((i & (align - 1)) != 0)
            i = (i | (align - 1)) + 1;
        if(i + n > (reinterpret_cast<
            std::uintptr_t>(this+1) + size))
            return nullptr;
        top = i + n;
        return reinterpret_cast<void*>(i);
    }
};

auto
monotonic_resource::
alloc_block(std::size_t size) ->
    block&
{
    auto const n = (
        size + sizeof(block) - 1) /
        sizeof(block);
    auto const bytes =
        (n + 1) * sizeof(block);
    auto& b = *::new(
        ::operator new(bytes)) block(
            n * sizeof(block), head_);
    head_ = &b;
    return b;
}

monotonic_resource::
~monotonic_resource()
{
    for(auto b = head_; b;)
    {
        auto next = b->next;
        ::operator delete(b);
        b = next;
    }
}

monotonic_resource::
monotonic_resource() noexcept
    : block_size_(64 * 1024)
{
}

monotonic_resource::
monotonic_resource(
    std::size_t block_size) noexcept
    : block_size_(block_size)
{
}

void*
monotonic_resource::
do_allocate(
    std::size_t n,
    std::size_t align)
{
    if(head_)
    {
        auto p = head_->alloc(n, align);
        if(p)
            return p;
    }
    if(n > block_size_ - 2 * align)
        alloc_block(n + 2 * align);
    else
        alloc_block(block_size_);
    auto p = head_->alloc(n, align);
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
    // nothing
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
