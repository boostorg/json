//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/parse.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

class parse_test
{
public:
    void
    run()
    {
        BOOST_TEST_PASS();
    }
};

TEST_SUITE(parse_test, "boost.json.parse");

} // json
} // boost
