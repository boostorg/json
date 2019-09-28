//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_TEST_STORAGE_HPP
#define BOOST_JSON_TEST_STORAGE_HPP

#include <boost/json/storage.hpp>
#include <cstddef>
#include <memory>

namespace boost {
namespace json {

struct fail_storage : storage
{
    std::size_t fail_max = 1;
    std::size_t fail = 0;

    void*
    allocate(
        std::size_t n,
        std::size_t) override
    {
        if(++fail == fail_max)
        {
            ++fail_max;
            fail = 0;
            throw std::bad_alloc{};
        }
        return std::allocator<
            char>{}.allocate(n);
    }

    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t) noexcept override
    {
        auto cp =
            reinterpret_cast<char*>(p);
        return std::allocator<
            char>{}.deallocate(cp, n);
    }
    bool
    is_equal(
        storage const& other
            ) const noexcept override
    {
        auto p = dynamic_cast<
            fail_storage const*>(&other);
        if(! p)
            return false;
        return this == p;
    }
};

struct unique_storage : storage
{
    void*
    allocate(
        std::size_t n,
        std::size_t) override
    {
        return std::allocator<
            char>{}.allocate(n);
    }

    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t) noexcept override
    {
        auto cp =
            reinterpret_cast<char*>(p);
        return std::allocator<
            char>{}.deallocate(cp, n);
    }
    bool
    is_equal(
        storage const& other
            ) const noexcept override
    {
        auto p = dynamic_cast<
            unique_storage const*>(&other);
        if(! p)
            return false;
        return this == p;
    }
};

} // json
} // boost

#endif
