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

#include <boost/json/serialize.hpp>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class parse_test
{
public:
    void
    good(string_view s)
    {
        error_code ec;
        auto jv = parse(s, ec);
        if(! BOOST_TEST(! ec))
            return;
        BOOST_TEST(
            serialize(jv) == s);
    }

    void
    bad(string_view s)
    {
        error_code ec;
        auto jv = parse(s, ec);
        BOOST_TEST(ec);
    }

    void
    testParse()
    {
        good("null");
        good("[1,2,3]");
        bad ("[1,2,3] #");
    }

    void
    run()
    {
        testParse();
    }
};

TEST_SUITE(parse_test, "boost.json.parse");

BOOST_JSON_NS_END
