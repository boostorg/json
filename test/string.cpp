//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/string.hpp>

#include <boost/config.hpp>
#include <boost/beast/_experimental/unit_test/suite.hpp>

#include "test_storage.hpp"

namespace boost {
namespace json {

class string_test : public beast::unit_test::suite
{
public:
    void
    testExceptions()
    {
        string s1 = "Hello, world!";
        auto sp =
            std::make_shared<fail_storage>();
        string s2;
        while(sp->fail < 200)
        {
            try
            {
                auto s = string(
                    string::allocator_type{sp});
                //obj.emplace("a", 2);
                //obj = obj0;
                //obj1 = obj;
                break;
            }
            catch(std::bad_alloc const&)
            {
            }
        }
    }

    void
    run() override
    {
    #if !defined(BOOST_MSVC) || _ITERATOR_DEBUG_LEVEL == 0
        testExceptions();
    #endif
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,string);

} // json
} // boost
