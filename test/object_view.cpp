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

struct value_view_test_test_struct
{
    int foo{42};
    double bar{4.2};
    string foobar = "test-string";
    std::vector<int> test_array = {1,2,3};
};

BOOST_DESCRIBE_STRUCT(value_view_test_test_struct, (), (foo, bar, foobar, test_array));

struct object_view_test
{
    void
    testEmpty()
    {
        object_view ov;
        BOOST_TEST(ov.empty());
        BOOST_TEST(ov.size() == 0u);
        BOOST_TEST(ov.begin() == ov.end());
        BOOST_TEST(ov.contains("foobar") == 0);
    }

    void
    testEmptyObject()
    {
        object o;
        object_view ov{o};
        BOOST_TEST(ov.empty());
        BOOST_TEST(ov.size() == 0u);
        BOOST_TEST(ov.begin() == ov.end());
        BOOST_TEST(ov.contains("foobar") == 0);
    }

    void
    testObject()
    {
        object o = {{"foobar", 42}};
        object_view ov{o};
        BOOST_TEST(!ov.empty());
        BOOST_TEST(ov.size() == 1u);
        BOOST_TEST(std::next(ov.begin()) == ov.end());
        BOOST_TEST(ov.contains("foobar") == 1);
        BOOST_TEST((*ov.begin()).key() == "foobar");
        BOOST_TEST((*ov.begin()).value() == 42);
    }


    void
    testEmptyMap()
    {
        std::unordered_map<std::string, int> o;
        object_view ov{o};
        BOOST_TEST(ov.empty());
        BOOST_TEST(ov.size() == 0u);
        BOOST_TEST(ov.begin() == ov.end());
        BOOST_TEST(ov.contains("foobar") == 0);
    }

    void
    testMap()
    {
        std::unordered_map<std::string, int> o = {{"foobar", 42}};
        object_view ov{o};
        BOOST_TEST(!ov.empty());
        BOOST_TEST(ov.size() == 1u);
        BOOST_TEST(std::next(ov.begin()) == ov.end());
        BOOST_TEST(ov.contains("foobar") == 1);
        BOOST_TEST((*ov.begin()).key() == "foobar");
        BOOST_TEST((*ov.begin()).value() == 42);
    }

    void
    testDescribedStruct()
    {
        value_view_test_test_struct ss{};
        object_view ov{ss};
        BOOST_TEST(!ov.empty());
        BOOST_TEST(ov.size() == 4u);
        auto itr = ov.begin();

        BOOST_TEST((*itr).key() == "foo");
        BOOST_TEST((*itr).value() == 42);
        itr++;
        BOOST_TEST((*itr).key() == "bar");
        BOOST_TEST((*itr).value() == 4.2);
        itr++;
        BOOST_TEST((*itr).key() == "foobar");
        BOOST_TEST((*itr).value() == "test-string");
        itr++;
        BOOST_TEST((*itr).key() == "test_array");
        BOOST_TEST((*itr).value() == (array{1,2,3}));
        itr++;
        BOOST_TEST(itr == ov.end());
        BOOST_TEST(ov.contains("foobar") == 1);

        BOOST_TEST(ov.at("foo") == 42);
        BOOST_TEST(ov.at("bar") == 4.2);
        BOOST_TEST(ov.at("foobar") == "test-string");
    }

    void run()
    {
        testEmpty();
        testEmptyObject();
        testObject();
        testEmptyMap();
        testMap();
        testDescribedStruct();
    }
};


TEST_SUITE(object_view_test, "boost.json.object_view");

}
}
