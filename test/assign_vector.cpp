//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
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
    #if ! defined(_MSC_VER) || _MSC_VER >= 1910
        BOOST_JSON_STATIC_ASSERT(
            has_from_json<int>::value);
        BOOST_JSON_STATIC_ASSERT(
            detail::has_adl_from_json<std::vector<int>>::value);

        value jv(array{});
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

BEAST_DEFINE_TESTSUITE(boost,json,assign_vector);

} // json
} // boost
