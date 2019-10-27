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
#include <cstddef>
#include <memory>
#include <stdexcept>

namespace boost {
namespace json {

/** A storage which uses a multiple fixed size blocks
*/
class block_storage : public storage
{
    struct block
    {
        std::size_t const size;
        std::size_t used;
        block* next;

        char*
        begin() noexcept
        {
            return reinterpret_cast<
                char*>(this+1);
        }

        char*
        end() noexcept
        {
            return begin() + size;
        }
    };

    std::size_t const block_size_;
    std::size_t refs_ = 0;
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
        std::size_t block_size = 256 * 1024)
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
            a.allocate(n + 1)) block{
                n * sizeof(block),
                0,
                head_};
        head_ = &b;
        return b;
    }

    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override
    {
        (void)align;
        // must be power of 2
        BOOST_JSON_ASSERT(
            (align & (align - 1)) == 0);
        // cannot exceed max alignment
        BOOST_JSON_ASSERT(
            align <= sizeof(std::max_align_t));
        auto const needed =
            [&]
            {
                if(n < block_size_)
                    return block_size_;
                return n + sizeof(block);
            };
        if(head_)
        {
            auto const avail =
                head_->size - head_->used;
            if(avail < n)
                alloc_block(needed());
        }
        else
        {
            alloc_block(needed());
        }
        ++refs_;
        auto p =
            head_->begin() + head_->used;
        head_->used += n;
        return p;
    }

    void
    do_deallocate(
        void*,
        std::size_t,
        std::size_t) noexcept override
    {
        if(--refs_ > 0)
            return;
        //clear();
    }
};

} // json
} // boost

#endif
