//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#include <boost/json/array.hpp>
#include <boost/json/object.hpp>
#include <boost/json/string.hpp>
#include <boost/json/value.hpp>
#include <boost/json/parser.hpp>

#include <vector>

#include "test_suite.hpp"
#include "test.hpp"

namespace boost {
namespace json {

/*
    This translation unit exercises code paths
    related to library limits such as max string
    length.
*/

class limits_test
{
public:
    void
    testValue()
    {
        // object too large
        {
            std::initializer_list<std::pair<
                string_view, value_ref>> init = {
            { "1", 1},{ "2", 2},{ "3", 3},{ "4", 4},{ "5", 5},
            { "6", 6},{ "7", 7},{ "8", 8},{ "9", 9},{"10",10},
            {"11",11},{"12",12},{"13",13},{"14",14},{"15",15},
            {"16",16},{"17",17},{"18",18},{"19",19},{"10",10},
            {"21",21},{"22",22},{"23",23},{"24",24},{"25",25},
            {"26",26},{"27",27},{"28",28},{"29",29},{"30",30},
            {"31",31}};
            BOOST_TEST(init.size() > object::max_size());
            BOOST_TEST_THROWS(value{init}, object_too_large);
        }
    }

    void
    testObject()
    {
        // max size
        {
            BOOST_TEST_THROWS(
                object(object::max_size()+1),
                std::length_error);
        }

        // max size
        {
            std::initializer_list<std::pair<
                string_view, value_ref>> init = {
            { "1", 1},{ "2", 2},{ "3", 3},{ "4", 4},{ "5", 5},
            { "6", 6},{ "7", 7},{ "8", 8},{ "9", 9},{"10",10},
            {"11",11},{"12",12},{"13",13},{"14",14},{"15",15},
            {"16",16},{"17",17},{"18",18},{"19",19},{"10",10},
            {"21",21},{"22",22},{"23",23},{"24",24},{"25",25},
            {"26",26},{"27",27},{"28",28},{"29",29},{"30",30},
            {"31",31}};
            BOOST_TEST(init.size() > object::max_size());
            BOOST_TEST_THROWS(
                object(init.begin(), init.end()),
                object_too_large);
            BOOST_TEST_THROWS(
                object(
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end())),
                object_too_large);
        }

        // max key size
        {
            std::string const big(
                string::max_size() + 1, '*');
            BOOST_TEST_THROWS(
                object({ {big, nullptr} }),
                key_too_large);
        }
    }

    void
    testArray()
    {
        {
            BOOST_TEST_THROWS(
                array(
                    array::max_size()+1,
                    value(nullptr)),
                array_too_large);
        }

        {
            std::vector<int> v(
                array::max_size()+1, 42);
            BOOST_TEST_THROWS(
                array(v.begin(), v.end()),
                array_too_large);
        }

        {
            std::vector<int> v(
                array::max_size()+1, 42);
            BOOST_TEST_THROWS(array(
                make_input_iterator(v.begin()),
                make_input_iterator(v.end())),
                array_too_large);
        }

        {
            array a;
            BOOST_TEST_THROWS(
                a.insert(a.begin(),
                    array::max_size() + 1,
                    nullptr),
                array_too_large);
        }
    }

    void
    testString()
    {
        // strings
        {
            {
                string s;
                BOOST_TEST_THROWS(
                    (s.resize(s.max_size() + 1)),
                    string_too_large);
            }

            {
                string s;
                s.resize(100);
                BOOST_TEST_THROWS(
                    (s.append(s.max_size() - 1, '*')),
                    string_too_large);
            }

            {
                string s;
                s.resize(s.max_size() - 5);
                BOOST_TEST_THROWS(
                    (s.replace(0, 1, s.subview(0, 10))),
                    string_too_large);
            }

            {
                string s;
                s.resize(s.max_size() - 5);
                BOOST_TEST_THROWS(
                    (s.replace(0, 1, 10, 'a')),
                    string_too_large);
            }

            {
                string s;
                s.resize(s.max_size() - 5);
                BOOST_TEST_THROWS(
                    (s.insert(0, s.subview(0, 10))),
                    string_too_large);
            }

    #if 0
            {
                // VFALCO tsan doesn't like this
                string s;
                try
                {
                    s.resize(s.max_size() - 1);
                }
                catch(std::exception const&)
                {
                }
            }
    #endif
        }

        // string in parser
        {
            parser p;
            std::string const big(
                string::max_size() + 1, '*');
            auto const js =
                "\"" + big + "\":null";
            error_code ec;
            auto jv = parse(js, ec);
            BOOST_TEST(ec == error::string_too_large);
        }

        // key in parser
        {
            parser p;
            std::string const big(
                string::max_size() + 1, '*');
            auto const js =
                "{\"" + big + "\":null}";
            error_code ec;
            auto jv = parse(js, ec);
            BOOST_TEST(ec == error::key_too_large);
        }
    }

    void
    testStack()
    {
        // max raw_stack
        // VFALCO The problem here is that we need the maximum stack
        //        larger than the maximum string to test string parts,
        //        but we need the opposite to test stack overflows.
        //        Thus stack overflow cannot be covered by tests.
    #if 0
        {
            std::string big;
            value jv;
            BOOST_TEST_THROWS(
                jv = parse(
                   "[1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,"
                    "1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0]"),
                stack_overflow);
        }
    #endif
    }

    void
    testParser()
    {
        // overflow in on_key_part
        {
            std::string big;
            big = "\\b";
            big += std::string(
                string::max_size()*2, '*');
            auto const js =
                "{\"" + big + "\":null}";
            error_code ec;
            auto jv = parse(js, ec);
            BOOST_TEST(ec == error::key_too_large);
        }

        // overflow in on_key
        {
            std::string big;
            big = "\\b";
            big += std::string(
                (string::max_size()*3)/2, '*');
            auto const js =
                "{\"" + big + "\":null}";
            error_code ec;
            auto jv = parse(js, ec);
            BOOST_TEST(ec == error::key_too_large);
        }

        // overflow in on_string_part
        {
            std::string big;
            big = "\\b";
            big += std::string(
                string::max_size()*2, '*');
            auto const js =
                "\"" + big + "\"";
            error_code ec;
            auto jv = parse(js, ec);
            BOOST_TEST(ec == error::string_too_large);
        }

        // overflow in on_string
        {
            std::string big;
            big = "\\b";
            big += std::string(
                (string::max_size()*3)/2, '*');
            auto const js =
                "\"" + big + "\"";
            error_code ec;
            auto jv = parse(js, ec);
            BOOST_TEST(ec == error::string_too_large);
        }

        // object overflow
        {
            string_view s = R"({
                "00":0,"01":0,"02":0,"03":0,"04":0,"05":0,"06":0,"07":0,"08":0,"09":0,
                "10":0,"11":0,"12":0,"13":0,"14":0,"15":0,"16":0,"17":0,"18":0,"19":0,
                "20":0
                })";
                
            error_code ec;
            auto jv = parse(s, ec);
            BOOST_TEST(ec == error::object_too_large);
        }

        // array overflow
        {
            string_view s = "["
                "0,0,0,0,0,0,0,0,0,0,"
                "0,0,0,0,0,0,0,0,0,0,"
                "0"
                "]";
            error_code ec;
            auto jv = parse(s, ec);
            BOOST_TEST(ec == error::array_too_large);
        }
    }

    void
    run()
    {
    #if ! defined(BOOST_JSON_NO_MAX_OBJECT_SIZE) && \
        ! defined(BOOST_JSON_NO_MAX_ARRAY_SIZE) && \
        ! defined(BOOST_JSON_NO_MAX_STRING_SIZE) && \
        ! defined(BOOST_JSON_NO_MAX_STACK_SIZE) && \
        ! defined(BOOST_JSON_NO_PARSER_BUFFER_SIZE)

        testValue();
        testObject();
        testArray();
        testString();
        testStack();
        testParser();

    #else
        BOOST_TEST_PASS();
    #endif
    }
};

TEST_SUITE(limits_test, "boost.json.limits");

} // json
} // boost
