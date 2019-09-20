//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

// Test that header file is self-contained.
#include <boost/json/assign_vector.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <type_traits>

namespace boost {
namespace json {

class assign_vector_test : public beast::unit_test::suite
{
public:
    void
    testAssign()
    {
        value jv(kind::array);
        {
            auto& a = jv.as_array();
            a.push_back(1);
            a.push_back(2);
            a.push_back(3);
        }
        std::vector<int> v;
        try
        {
            jv.store(v);
            BEAST_EXPECT(v.size() == 3);
            BEAST_EXPECT(v[0] == 1);
            BEAST_EXPECT(v[1] == 2);
            BEAST_EXPECT(v[2] == 3);
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

BEAST_DEFINE_TESTSUITE(beast,json,assign_vector);

} // json
} // boost
