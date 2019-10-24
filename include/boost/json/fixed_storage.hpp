//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_FIXED_STORAGE_HPP
#define BOOST_JSON_FIXED_STORAGE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage.hpp>
#include <stdexcept>

namespace boost {
namespace json {

/** A storage which uses a single fixed size allocation
*/
class fixed_storage : public storage
{
    char* const base_;
    std::size_t const size_;
    std::size_t used_ = 0;
    std::size_t refs_ = 0;

public:
    ~fixed_storage()
    {
        delete[] base_;
    }

    explicit
    fixed_storage(
        std::size_t bytes)
        : base_(new char[bytes])
        , size_(bytes)
    {
    }

protected:
    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override
    {
        // must be power of 2
        BOOST_JSON_ASSERT(
            (align & (align - 1)) == 0);
        auto offset = used_;
        if(offset & (align - 1))
        {
            offset |= (align - 1);
            ++offset;
        }
        if( offset > size_ ||
            n > size_ - offset)
            BOOST_JSON_THROW(std::bad_alloc());
        ++refs_;
        used_ = offset + n;
        return base_ + offset;
    }

    void
    do_deallocate(
        void*,
        std::size_t,
        std::size_t) noexcept override
    {
        if(--refs_ > 0)
            return;
        used_ = 0;
    }
};

} // json
} // boost

#endif
