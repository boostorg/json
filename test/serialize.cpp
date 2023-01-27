//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

// Test that header file is self-contained.
#include <boost/json/serialize.hpp>

#include <boost/json/parse.hpp>
#include <boost/json/static_resource.hpp>
#include <sstream>

#include "test_suite.hpp"

namespace boost {
namespace json {

class serialize_test
{
public:
    template<class T>
    static
    std::string
    print(T const& t)
    {
        std::stringstream ss;
        ss << t;
        return ss.str();
    }

    void
    testSerialize()
    {
        {
            value const jv = { 1, 2, 3 };
            BOOST_TEST(serialize(jv) == "[1,2,3]");
            BOOST_TEST(print(jv) == "[1,2,3]");
        }
        {
            array const arr = { 1, 2 ,3 };
            BOOST_TEST(serialize(arr) == "[1,2,3]");
            BOOST_TEST(print(arr) == "[1,2,3]");
        }
        {
            object const obj = { {"k1",1}, {"k2",2} };
            BOOST_TEST(serialize(obj) == "{\"k1\":1,\"k2\":2}");
            BOOST_TEST(print(obj) == "{\"k1\":1,\"k2\":2}");
        }
        {
            string const str = "123";
            BOOST_TEST(serialize(str) == "\"123\"");
            BOOST_TEST(print(str) == "\"123\"");
        }
    }

    void
    testUDT()
    {
        {
            std::vector<std::uint64_t> v = { 1, 2, 3 };
            BOOST_TEST(serialize(v) == "[1,2,3]");
        }
    }

    void
    run()
    {
        testSerialize();
        testUDT();
    }
};

TEST_SUITE(serialize_test, "boost.json.serialize");

} // namespace json
} // namespace boost
