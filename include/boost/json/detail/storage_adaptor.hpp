//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_DETAIL_STORAGE_ADAPTOR_HPP
#define BOOST_JSON_DETAIL_STORAGE_ADAPTOR_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage.hpp>
#include <boost/align/align_up.hpp>
#include <boost/core/empty_value.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <atomic>
#include <cstddef>

#ifdef BOOST_NO_CXX11_ALLOCATOR
#include <boost/container/allocator_traits.hpp>
#else
#include <memory>
#endif

namespace boost {
namespace json {
namespace detail {

#ifdef BOOST_NO_CXX11_ALLOCATOR
template<class Allocator>
using allocator_of_char =
    typename boost::container::allocator_traits<
        Allocator>::template rebind_alloc<char>;

#else
template<class Allocator>
using allocator_of_char =
    typename std::allocator_traits<
        Allocator>::template rebind_alloc<char>;

#endif

template<class Allocator>
struct storage_adaptor
    : boost::empty_value<
        allocator_of_char<Allocator>>
    , storage
{
    // VFALCO This is all public because msvc friend bugs

    std::atomic<unsigned> count_;

    explicit
    storage_adaptor(Allocator const& alloc)
        : boost::empty_value<
            allocator_of_char<Allocator>>(
                boost::empty_init_t{}, alloc)
        , count_(1)
    {
    }

    ~storage_adaptor()
    {
    }

    void
    addref() noexcept override
    {
        ++count_;
    }

    void
    release() noexcept override
    {
        if(--count_ > 0)
            return;
        delete this;
    }

    void*
    allocate(
        std::size_t n,
        std::size_t align) override
    {
        auto const n1 =
            boost::alignment::align_up(n, align);
        BOOST_ASSERT(n1 >= n);
        return this->get().allocate(n1);
    }
    
    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t) noexcept override
    {
        this->get().deallocate(
            reinterpret_cast<
                char*>(p), n);
    }

    bool
    is_equal(
        storage const& other) const noexcept override
    {
        auto p = dynamic_cast<
            storage_adaptor const*>(&other);
        if(! p)
            return false;
        //return this->get() == p->get();
        // VFALCO We require pointer equality
        //        to prevent objects from different
        //        "documents" getting mixed together.
        return this == p;
    }
};

} // detail
} // json
} // boost

#endif
