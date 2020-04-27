//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
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
#include <boost/json/detail/monotonic_resource.hpp>

namespace boost {
namespace json {

/** A fast memory resource using many large fixed size blocks.
*/
class monotonic_resource final
    : public memory_resource
{   
    struct initial_block
    {
        std::size_t size;
        unsigned char* base = nullptr;
        unsigned char* top;

        initial_block() = default;

        initial_block(
            void* buf,
            std::size_t n)
            : size(n)
            , base(static_cast<unsigned char*>(buf))
            , top(base)
        {
        }
    };

    struct alignas(detail::max_align()) block;

    std::size_t block_size_;
    block* head_ = nullptr;
    initial_block initial_;
    
    static const std::size_t min_block_size_ = 1024;
    static const std::size_t max_block_size_ = -1;

    template<typename Block>
    void*
    allocate_in_block(
        Block& blk,
        std::size_t n,
        std::size_t align);

    inline
    block&
    allocate_new_block(std::size_t size);
    
    inline
    std::size_t
    next_block_size(std::size_t);

    inline
    std::size_t
    closest_block_size(std::size_t);

    inline
    std::size_t
    grow_block_size(std::size_t size);

public:
    BOOST_JSON_DECL
    ~monotonic_resource();

    monotonic_resource(
        const monotonic_resource&) = delete;

    BOOST_JSON_DECL
    monotonic_resource(
        std::size_t initial_size = 1024) noexcept;

    BOOST_JSON_DECL
    monotonic_resource(
        void* buffer,
        std::size_t buffer_size) noexcept;

    monotonic_resource&
    operator=(
        const monotonic_resource&) = delete;

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
