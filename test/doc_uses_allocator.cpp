//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/container/pmr/memory_resource.hpp>
#include <boost/json/monotonic_resource.hpp>
#include <boost/json/value.hpp>
#include <vector>

#include "test_suite.hpp"

namespace boost {
namespace json {

//----------------------------------------------------------

static void set1() {

//----------------------------------------------------------
{
// tag::doc_uses_allocator_1[]
// We want to use this resource for all the containers
monotonic_resource mr;

// Declare a vector of JSON values
std::vector< value, boost::container::pmr::polymorphic_allocator< value > > v( &mr );

// The polymorphic allocator will use our resource
assert( v.get_allocator().resource() == &mr );

// Add a string to the vector
v.emplace_back( "boost" );

// The vector propagates the memory resource to the string
assert( v[0].storage().get() == &mr );
// end::doc_uses_allocator_1[]
}
//----------------------------------------------------------
{
// tag::doc_uses_allocator_2[]
// This vector will use the default memory resource
std::vector< value, boost::container::pmr::polymorphic_allocator < value > > v;

// This value will same memory resource as the vector
value jv( v.get_allocator() );

// However, ownership is not transferred,
assert( ! jv.storage().is_shared() );

// and deallocate is never null
assert( ! jv.storage().is_deallocate_trivial() );
// end::doc_uses_allocator_2[]
}
//----------------------------------------------------------

} // set1

//----------------------------------------------------------

class doc_uses_allocator_test
{
public:
    void
    run()
    {
        (void)&set1;
        BOOST_TEST_PASS();
    }
};

TEST_SUITE(doc_uses_allocator_test, "boost.json.doc_uses_allocator");

} // namespace json
} // namespace boost
