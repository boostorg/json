//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/parser.hpp>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class parser_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(parser_test, "boost.json.parser");

BOOST_JSON_NS_END
