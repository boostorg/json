//
// Copyright (c) 2020 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_STATIC_RESOURCE_HPP
#define BOOST_JSON_STATIC_RESOURCE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>

namespace boost {
namespace json {

/** A memory resource using a caller-provided buffer
*/
class static_resource final
    : public memory_resource
{   
    char* buffer_;
    std::size_t size_;
    std::size_t used_;

public:
    static_resource(
        static_resource const&) = delete;
    static_resource& operator=(
        static_resource const&) = delete;

    /// Destructor
    BOOST_JSON_DECL
    ~static_resource() noexcept;

    /** Constructor.
    */
    BOOST_JSON_DECL
    static_resource(
        void* buffer,
        std::size_t size) noexcept;

protected:
    /** Allocate memory.
    */
    BOOST_JSON_DECL
    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override;

    /** Deallocate memory.
    */
    BOOST_JSON_DECL
    void
    do_deallocate(
        void* p,
        std::size_t n,
        std::size_t align) override;

    /** Compare for equality.
    */
    BOOST_JSON_DECL
    bool
    do_is_equal(
        memory_resource const& mr
            ) const noexcept override;
};

template<>
struct is_deallocate_null<
    static_resource>
{
    static constexpr bool value = true;
};

} // json
} // boost

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/static_resource.ipp>
#endif

#endif
