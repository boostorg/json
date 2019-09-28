//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_STORAGE_HPP
#define BOOST_JSON_STORAGE_HPP

#include <boost/json/detail/config.hpp>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <type_traits>

namespace boost {
namespace json {

/** Abstract interface to a memory resource used with JSON.
*/
class storage
{
public:
    static std::size_t constexpr max_align =
        sizeof(max_align_t);

    virtual
    ~storage() = default;

    virtual
    void*
    allocate(
        std::size_t n,
        std::size_t align =
            max_align) = 0;

    virtual
    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t align =
            max_align) noexcept = 0;

    virtual
    bool
    is_equal(
        storage const& other) const noexcept = 0;

    friend
    bool
    operator==(
        storage const& lhs,
        storage const& rhs) noexcept
    {
        return &lhs == &rhs || lhs.is_equal(rhs);
    }

    friend
    bool
    operator!=(
        storage const& lhs,
        storage const& rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

//------------------------------------------------------------------------------

/** Manages a type-erased storage object and options for a set of JSON values.
*/
using storage_ptr = std::shared_ptr<storage>;

//------------------------------------------------------------------------------

/** Construct a new storage object
*/
template<class Storage, class... Args>
storage_ptr
make_storage_ptr(Args&&... args);

/** Construct a storage adaptor for the specified allocator
*/
template<class Allocator>
storage_ptr
make_storage_adaptor(Allocator const& a);

/** Return a pointer to the current default storage
*/
BOOST_JSON_DECL
storage_ptr
default_storage();

/** Set the current default storage

    This function may not be called concurrently,
    or concurrent with @ref default_storage.
*/
BOOST_JSON_DECL
void
default_storage(storage_ptr sp);

} // json
} // boost

#include <boost/json/impl/storage.hpp>
#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/storage.ipp>
#endif

#endif
