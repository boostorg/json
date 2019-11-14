//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
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
    #if ! defined(_MSC_VER) || _MSC_VER >= 1910
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
    #else
        pass();
    #endif
    }

    void
    run() override
    {
        testAssign();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,assign_string);

} // json
} // boost
