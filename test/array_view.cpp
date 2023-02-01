//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/json/value_view.hpp>

#include "test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace json {


struct value_view_test
{
    void testEmpty()
    {
        array_view av{};
        BOOST_TEST(av.empty());
        BOOST_TEST(av.size() == 0u);
        BOOST_TEST(av.begin() == av.end());

    }
    void testEmptyVector()
    {
        std::vector<int> vec;
        array_view av{vec};
        BOOST_TEST(av.empty());
        BOOST_TEST(av.size() == 0u);
        BOOST_TEST(av.begin() == av.end());
    }

    void testVector()
    {
        std::vector<int> vec = { 1,2,3};
        array_view av{vec};
        BOOST_TEST(!av.empty());
        BOOST_TEST(av.size() == 3u);
        BOOST_TEST(std::distance(av.begin(), av.end()) ==3u);
        std::equal(av.begin(), av.end(), vec.begin(), vec.end());
    }
    void testArray()
    {
        json::array ja = { 1,2.6, "test"};
        array_view av{ja};
        BOOST_TEST(!av.empty());
        BOOST_TEST(av.size() == 3u);
        BOOST_TEST(std::distance(av.begin(), av.end()) == 3u);
        std::equal(av.begin(), av.end(), ja.begin(), ja.end());
    }
    void testTuple()
    {
        std::tuple<int, double, json::string> tup = { 1,2.6, "test"};
        using tup_t = decltype(tup);

        array_view av{tup};
        BOOST_TEST(!av.empty());
        BOOST_TEST(av.size() == 3u);
        BOOST_TEST(std::distance(av.begin(), av.end()) == 3u);

        json::array ja{1, 2.6, "test"};
        std::equal(av.begin(), av.end(), ja.begin(), ja.end());
    }

    void run()
    {
        testEmpty();
        testEmptyVector();
        testVector();
        testArray();
        testTuple();
    }
};


TEST_SUITE(value_view_test, "boost.json.array_view");

}
}
