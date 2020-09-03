//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_MONOTONIC_RESOURCE_HPP
#define BOOST_JSON_MONOTONIC_RESOURCE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/detail/monotonic_resource.hpp>

BOOST_JSON_NS_BEGIN

/** A fast memory resource that never deallocates.
    
    A memory resource that allocates large fixed-size
    blocks of memory, with each allocated block being
    twice as large as the previous block.
    Allocated blocks are not freed until the memory
    resource is destroyed; calls to @ref do_deallocate
    have no effect. Memory will not be allocated unless
    @ref do_allocate is called and the current block
    does not have sufficent room to meet the request.
\n
    An *initial buffer* can be optionally specified
    when constructing a `monotonic_resource`.
    If an initial buffer is provided, calls to
    @ref do_allocate will return storage within
    that buffer until it is filled to capacity,
    at which point subsequent blocks will be allocated
    using the default resource.
\n
    In addition to this, an *initial size* can be
    passed to the constructor of `monotonic_resource`.
    The initial size determines the size of the first
    dynamically allocated block.
    @n
\n
    This memory resource is useful for parsing without
    subsequent modification to the resulting @ref value.
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
    
    static std::size_t const min_block_size_ = 1024;
    static std::size_t const max_block_size_ = std::size_t(-1);

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
    /// Copy constructor (deleted)
    monotonic_resource(
        const monotonic_resource&) = delete;

    /// Copy assignment (deleted)
    monotonic_resource& operator=(
        const monotonic_resource&) = delete;

    /** Destructor.
        
        Releases all blocks allocated by the resource.

        @par Complexity

        Linear in the number of allocated blocks.
    */
    BOOST_JSON_DECL
    ~monotonic_resource() noexcept;

    /** Constructor.

        Constructs a `monotonic_resource` whose initial size
        is `initial_size` rounded up to the nearest power of two;
        if `initial_size` is less than 1024, the initial size will
        be 1024.

        @note This function does not allocate memory.

        @param initial_size An optional, initial size of the
        resource. If omitted, a reasonable default will be used.
    */
    BOOST_JSON_DECL
    monotonic_resource(
        std::size_t initial_size = 1024) noexcept;

    /** Constructor.

        Constructs a `monotonic_resource` whose initial buffer is
        the `buffer_size` bytes of memory pointed to by `buffer`.
        The initial size is `buffer_size` rounded up to the nearest
        power of two; if `buffer_size` is less than 1024, the initial
        size will be 1024.

        @par Precondition

        `{buffer, buffer + buffer_size)` is a valid range.

        @note This constructor does allocate memory.

        @param buffer A pointer to the initial buffer for the
        resource to use. Ownership is not transferred.

        @param buffer_size The size of the initial buffer.
    */
    BOOST_JSON_DECL
    monotonic_resource(
        void* buffer,
        std::size_t buffer_size) noexcept;

protected:
    /** Allocate memory.

        Allocates a region of memory of size `n`
        that is suitably aligned for `align`.
        @n
        
        If the `monotonic_resource` has an initial buffer
        and its initial buffer has sufficent space to meet
        the request, the memory will be allocated within
        that buffer.

        Otherwise, if the current dynamically allocated block
        has sufficent room to meet the request, the memory will
        be allocated within that block. If there is no such block,
        or if the current block cannot meet the request, a new block
        will be allocated. The size of the new block will be twice
        as large as that of the current block, or `n` rounded up to
        the nearest power of two if `n` exceeds the size of the current
        block. The memory is then allocated within the new block.

        @par Example
        @code
        monotonic_resource mr;

        error_code ec;
        parser p;
        p.reset( &mr );
        p.write( "[1,2,3]", 7, ec );
        value jv = p.release();

        assert( jv.storage().get() == &mr );
        @endcode

        @par Exception Safety

        Strong guarantee.

        @note Once a new block is dynamically allocated,
        memory will never be allocated in any previous blocks
        even if they have sufficent room to fulfil the request.
        
        @note This function is never called directly;
        calls to `memory_resource::allocate`
        will invoke this function to fufill requests for allocation.

        @return A pointer to memory that is of size `n` and
        suitably aligned for `align`.

        @param n The number of bytes to allocate.

        @param align The desired alignment of the memory.

        @throw std::bad_alloc Failure to allocate a new block.
    */
    BOOST_JSON_DECL
    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override;

    /** Deallocate memory.

        This function has no effect.

        @note This function is never called directly;
        calls to `memory_resource::deallocate`
        will invoke this function to fufill requests for deallocation.
    */
    BOOST_JSON_DECL
    void
    do_deallocate(
        void* p,
        std::size_t n,
        std::size_t align) override;

    /** Compare for equality.
        
        Returns whether `mr` is the base class subobject
        of this resource.

        @note This function is never called directly;
        calls to `memory_resource::is_equal`
        will invoke this function when comparing
        memory resources.

        @return `&mr == this`

        @param mr The `memory_resource` to compare with.
    */
    BOOST_JSON_DECL
    bool
    do_is_equal(
        memory_resource const& mr) const noexcept override;
};

template<>
struct is_deallocate_trivial<
    monotonic_resource>
{
    static constexpr bool value = true;
};

BOOST_JSON_NS_END

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/impl/monotonic_resource.ipp>
#endif

#endif
