//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DEFAULT_RESOURCE_HPP
#define BOOST_JSON_DEFAULT_RESOURCE_HPP

#include <boost/json/detail/config.hpp>
#include <new>

BOOST_JSON_NS_BEGIN
namespace detail {

/** A simple memory resource that uses operator new and delete.
*/
class
    BOOST_SYMBOL_VISIBLE
    BOOST_JSON_CLASS_DECL
    default_resource final
    : public memory_resource
{
public:
#ifdef BOOST_JSON_HEADER_ONLY
    inline
#endif
    ~default_resource();

    void*
    do_allocate(
        std::size_t n,
        std::size_t) override;

    void
    do_deallocate(
        void* p,
        std::size_t,
        std::size_t) override;

    bool
    do_is_equal(
        memory_resource const& mr) const noexcept override;
};

} // detail
BOOST_JSON_NS_END

#ifdef BOOST_JSON_HEADER_ONLY
#include <boost/json/detail/impl/default_resource.ipp>
#endif

#endif
