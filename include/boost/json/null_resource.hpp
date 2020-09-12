//
// Copyright (c) 2020 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_NULL_RESOURCE_HPP
#define BOOST_JSON_NULL_RESOURCE_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/memory_resource.hpp>

BOOST_JSON_NS_BEGIN

//----------------------------------------------------------

/** A resource which always fails.

    This memory resource always throws the exception
    `std::bad_alloc` in calls to `allocate`.
*/
class BOOST_JSON_CLASS_DECL
    null_resource final
    : public memory_resource
{   
public:
    /// Copy constructor (deleted)
    null_resource(
        null_resource const&) = delete;

    /// Copy assignment (deleted)
    null_resource& operator=(
        null_resource const&) = delete;

    /// Destructor
    ~null_resource() noexcept = default;

    /** Constructor

        This constructors the resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    /** @{ */
    null_resource() noexcept = default;

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

template<>
struct is_deallocate_trivial<
    null_resource>
{
    static constexpr bool value = true;
};

BOOST_JSON_NS_END

#endif
