//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/assign_string.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <type_traits>

namespace boost {
namespace json {

class assign_string_test : public beast::unit_test::suite
{
public:
    void
    testAssign()
    {
        value jv = "test";
        std::string s;
        try
        {
            jv.store(s);
            BEAST_EXPECT(s == "test");
        }
        catch(std::exception const&)
        {
            BEAST_FAIL();
        }
    }

    void run() override
    {
        testAssign();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,assign_string);

} // json
} // boost
