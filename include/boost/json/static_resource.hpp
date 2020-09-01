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

    This memory resource satisfies allocation requests
    by returning portions of memory from a fixed-size
    buffer specified upon construction. Ownership of
    the buffer is not transferred; the caller is
    responsible for ensuring that the lifetime of the
    buffer extends until the static resource is
    destroyed.
\n

    @par Example
*/
class static_resource final
    : public memory_resource
{   
    void* p_;
    std::size_t n_;

public:
    /// Copy constructor (deleted)
    static_resource(
        static_resource const&) = delete;

    /// Copy assignment (deleted)
    static_resource& operator=(
        static_resource const&) = delete;

    /// Destructor
    BOOST_JSON_DECL
    ~static_resource() noexcept;

    /** Constructor

        The resource is constructed to use the specified
        buffer.

        @par Complexity

        Constant.

        @par Exception Safety

        No-throw guarantee.

        @param buffer A pointer to valid storage of at
        least `size` bytes. Ownership is not transferred.

        @param size The number of valid bytes pointed
        to by `buffer`.
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
struct is_deallocate_trivial<
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
