//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_BLOCK_STORAGE_HPP
#define BOOST_JSON_BLOCK_STORAGE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/detail/assert.hpp>
#include <boost/json/storage.hpp>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace boost {
namespace json {

/** A storage which uses a multiple fixed size blocks
*/
class block_storage final
    : public storage
{
    struct block
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
            BOOST_JSON_ASSERT(
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

    std::size_t const block_size_;
    block* head_ = nullptr;

public:
    ~block_storage()
    {
        std::allocator<block> a;
        auto b = head_;
        while(b)
        {
            auto next = b->next;
            a.deallocate(b,
                sizeof(*b) + b->size);
            b = next;
        }
    }

    explicit
    block_storage(
        std::size_t block_size = 64 * 1024)
        : block_size_(block_size)
    {
    }

private:
    block&
    alloc_block(std::size_t size)
    {
        std::allocator<block> a;
        auto const n = (
            size + sizeof(block) - 1) /
            sizeof(block);
        auto& b = *::new(
            a.allocate(n + 1)) block(
                n * sizeof(block), head_);
        head_ = &b;
        return b;
    }

    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override
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
        BOOST_JSON_ASSERT(p);
        return p;
    }

    void
    do_deallocate(
        void*,
        std::size_t,
        std::size_t) noexcept override
    {
    }
};

} // json
} // boost

#endif
