//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_MONOTONIC_RESOURCE_HPP
#define BOOST_JSON_MONOTONIC_RESOURCE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>

namespace boost {
namespace json {

/** A fast memory resource using many large fixed size blocks.
*/
class monotonic_resource final
    : public memory_resource
{
    struct block;
    std::size_t const block_size_ = 64 * 1024;
    block* head_ = nullptr;

    inline block& alloc_block(std::size_t size);

public:
    BOOST_JSON_DECL
    ~monotonic_resource();

    BOOST_JSON_DECL
    monotonic_resource() noexcept;

    BOOST_JSON_DECL
    monotonic_resource(
        std::size_t block_size) noexcept;

    BOOST_JSON_DECL
    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override;

    BOOST_JSON_DECL
    void
    do_deallocate(
        void* p,
        std::size_t n,
        std::size_t align) override;

    BOOST_JSON_DECL
    bool
    do_is_equal(
        memory_resource const& mr) const noexcept override;
};

template<>
struct is_deallocate_null<
    monotonic_resource>
{
    static
    constexpr
    bool
    deallocate_is_null() noexcept
    {
        return true;
    }
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/monotonic_resource.ipp>
#endif

#endif
