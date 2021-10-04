//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

// Test that header file is self-contained.
#include <boost/json/parse_into.hpp>

#include "test.hpp"
#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class parse_into_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(parse_into_test, "boost.json.parse_into");

BOOST_JSON_NS_END
