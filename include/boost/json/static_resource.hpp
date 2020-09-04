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
#include <boost/json/memory_resource.hpp>
#include <cstddef>

BOOST_JSON_NS_BEGIN

//----------------------------------------------------------

/** A resource using a caller-owned buffer, with a trivial deallocate

    This memory resource is a special-purpose resource
    that releases allocated memory only when the resource
    is destroyed (or when @ref release is called).
    It has a trivial deallocate function. That is, the
    metafunction @ref is_deallocate_trivial returns `true`.
    It has a trivial deallocate function. That is, the
    metafunction @ref is_deallocate_trivial returns `true`.
\n
    The resource is be constructed from caller-owned buffer
    from which subsequent calls to allocate are apportioned.
    When a memory request cannot be satisfied from the
    free bytes remaining in the buffer, the allocation
    request fails with the exception `std::bad_alloc`.
\n
    @par Example

    This parses a JSON into a value which uses a local
    stack buffer, then prints the result.

    @code

    unsigned char buf[ 4000 ];
    static_resource mr( buf );

    // Parse the string, using our memory resource
    auto const jv = parse( "[1,2,3]", &mr );

    // Print the JSON
    std::cout << jv;

    @endcode

    @par Thread Safety
    Members of the same instance may not be
    called concurrently.
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

        This constructs the resource to use the specified
        buffer for subsequent calls to allocate. When the
        buffer is exhausted, calls to allocate will fail
        with the exception `std::bad_alloc` thrown.

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

   /** Constructor

        This constructs the resource to use the specified
        array for subsequent calls to allocate. When the
        array is exhausted, calls to allocate will fail
        with the exception `std::bad_alloc` thrown.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param buffer An array to use as the buffer.
        Ownership is not transferred; the caller is
        responsible for ensuring that the lifetime of
        the array extends until the resource is destroyed.
    */
    template<std::size_t N>
    explicit
    static_resource(
        unsigned char(&buffer)[N]) noexcept
        : static_resource(
            buffer, N)
    {
    }

#if defined(__cpp_lib_byte) || defined(BOOST_JSON_DOCS)
   /** Constructor

        This constructs the resource to use the specified
        array for subsequent calls to allocate. When the
        array is exhausted, calls to allocate will fail
        with the exception `std::bad_alloc` thrown.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param buffer An array to use as the buffer.
        Ownership is not transferred; the caller is
        responsible for ensuring that the lifetime of
        the array extends until the resource is destroyed.
    */
    template<std::size_t N>
    explicit
    static_resource(
        std::byte(&buffer)[N]) noexcept
        : static_resource(
            buffer, N)
    {
    }
#endif

#ifndef BOOST_JSON_DOCS
    // Safety net for accidental buffer overflows
    template<std::size_t N>
    static_resource(
        unsigned char(&buffer)[N], std::size_t n) noexcept
        : static_resource(&buffer[0], n)
    {
        // If this goes off, check your parameters
        // closely, chances are you passed an array
        // thinking it was a pointer.
        BOOST_ASSERT(n <= N);
    }

#ifdef __cpp_lib_byte
    // Safety net for accidental buffer overflows
    template<std::size_t N>
    static_resource(
        std::byte(&buffer)[N], std::size_t n) noexcept
        : static_resource(&buffer[0], n)
    {
        // If this goes off, check your parameters
        // closely, chances are you passed an array
        // thinking it was a pointer.
        BOOST_ASSERT(n <= N);
    }
#endif
#endif

protected:
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
        memory_resource const& mr
            ) const noexcept override;
};

template<>
struct is_deallocate_trivial<
    static_resource>
{
    static constexpr bool value = true;
};

BOOST_JSON_NS_END

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/static_resource.ipp>
#endif

#endif
