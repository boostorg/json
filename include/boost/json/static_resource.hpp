//
// Copyright (c) 2020 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_STATIC_RESOURCE_HPP
#define BOOST_JSON_STATIC_RESOURCE_HPP

#include <boost/container/pmr/memory_resource.hpp>
#include <boost/json/detail/config.hpp>
#include <boost/json/is_deallocate_trivial.hpp>
#include <cstddef>

namespace boost {
namespace json {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4275) // non dll-interface class used as base for dll-interface class
#endif

//----------------------------------------------------------

/** A resource using a caller-owned buffer, with a trivial deallocate.

    This memory resource is a special-purpose resource that releases allocated
    memory only when the resource is destroyed (or when @ref release is
    called). It has a trivial deallocate function; that is, the metafunction
    @ref is_deallocate_trivial returns `true`.

    The resource is constructed from a caller-owned buffer from which
    subsequent calls to allocate are apportioned. When a memory request cannot
    be satisfied from the free bytes remaining in the buffer, the allocation
    request fails with the exception `std::bad_alloc`.

    @par Example
    This parses a JSON text into a value which uses a local stack buffer, then
    prints the result.

    @code
    unsigned char buf[ 4000 ];
    static_resource mr( buf );

    // Parse the string, using our memory resource
    value const jv = parse( "[1,2,3]", &mr );

    // Print the JSON
    std::cout << jv;
    @endcode

    @par Thread Safety
    Members of the same instance may not be called concurrently.

    @see https://en.wikipedia.org/wiki/Region-based_memory_management
*/
class
    BOOST_JSON_DECL
    BOOST_SYMBOL_VISIBLE
static_resource final
    : public container::pmr::memory_resource
{
    void* p_;
    std::size_t n_;
    std::size_t size_;

public:
    /** Assignment operator.

        The type is neither copyable nor movable, so this operator is deleted.
    */
    static_resource& operator=(
        static_resource const&) = delete;

    /** Constructors.

        These construct the resource to use the specified buffer for subsequent
        calls to allocate. When the buffer is exhausted, allocate will throw
        `std::bad_alloc`.

        Ownership of `buffer` is not transferred; the caller is responsible for
        ensuring that its lifetime extends until the resource is destroyed.

        Overload **(5)** is the copy constructor. The type is neither copyable
        nor movable, so this overload is deleted.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param buffer The buffer to use.
        @param size The number of valid bytes pointed to by `buffer`.

        @{
    */
    static_resource(
        unsigned char* buffer,
        std::size_t size) noexcept;

#if defined(__cpp_lib_byte) || defined(BOOST_JSON_DOCS)
    static_resource(
        std::byte* buffer,
        std::size_t size) noexcept
        : static_resource(reinterpret_cast<
            unsigned char*>(buffer), size)
    {
    }
#endif

    /** Overload

        @tparam N The size of `buffer`.
        @param buffer
    */
    template<std::size_t N>
    explicit
    static_resource(
        unsigned char(&buffer)[N]) noexcept
        : static_resource(&buffer[0], N)
    {
    }

#if defined(__cpp_lib_byte) || defined(BOOST_JSON_DOCS)
    /** Overload

        @tparam N
        @param buffer
    */
    template<std::size_t N>
    explicit
    static_resource(
        std::byte(&buffer)[N]) noexcept
        : static_resource(&buffer[0], N)
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

    /// Overload
    static_resource(
        static_resource const&) = delete;
    /// @}

    /** Release all allocated memory.

        This function resets the buffer provided upon construction so that all
        of the valid bytes are available for subsequent allocation.

        @par Complexity
        Constant

        @par Exception Safety
        No-throw guarantee.
    */
    void
    release() noexcept;

protected:
#ifndef BOOST_JSON_DOCS
    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override;

    void
    do_deallocate(
        void* p,
        std::size_t n,
        std::size_t align) override;

    bool
    do_is_equal(
        memory_resource const& mr
            ) const noexcept override;
#endif
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

template<>
struct is_deallocate_trivial<
    static_resource>
{
    static constexpr bool value = true;
};

} // namespace json
} // namespace boost

#endif
