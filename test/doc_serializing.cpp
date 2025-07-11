//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/serialize.hpp>
#include <boost/json/serializer.hpp>

#include <iostream>

#include "test_suite.hpp"

namespace boost {
namespace json {

//----------------------------------------------------------

static void set1() {

//----------------------------------------------------------
{
// tag::doc_serializing_1[]
value jv = { 1, 2, 3, 4, 5 };

std::cout << jv << "\n";
// end::doc_serializing_1[]
}
//----------------------------------------------------------
{
// tag::doc_serializing_2[]
value jv = { 1, 2, 3, 4, 5 };

std::string s = serialize( jv );
// end::doc_serializing_2[]
}
//----------------------------------------------------------
{
//[doc_serializing_5
//]
}

} // set1

class doc_serializing_test
{
public:
    void
    run()
    {
        (void)&set1;
    }
};

TEST_SUITE(doc_serializing_test, "boost.json.doc_serializing");

} // namespace json
} // namespace boost
