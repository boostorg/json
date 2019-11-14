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

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include "test.hpp"
#include <vector>

namespace boost {
namespace json {

/*
    This translation unit exercises code paths
    related to library limits such as max string
    length.
*/

class limits_test : public beast::unit_test::suite
{
public:
    void
    testObject()
    {
        // max size
        {
            BEAST_THROWS(
                object(object::max_size()+1),
                std::length_error);
        }

        // max size
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
            { "1", 1},{ "2", 2},{ "3", 3},{ "4", 4},{ "5", 5},
            { "6", 6},{ "7", 7},{ "8", 8},{ "9", 9},{"10",10},
            {"11",11},{"12",12},{"13",13},{"14",14},{"15",15},
            {"16",16},{"17",17},{"18",18},{"19",19},{"10",10},
            {"21",21},{"22",22},{"23",23},{"24",24},{"25",25},
            {"26",26},{"27",27},{"28",28},{"29",29},{"30",30},
            {"31",31}};
            BEAST_EXPECT(init.size() > object::max_size());
            BEAST_THROWS(
                object(init.begin(), init.end()),
                std::length_error);
            BEAST_THROWS(
                object(
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end())),
                std::length_error);
        }

        // max key size
        {
            std::string const big(
                string::max_size() + 1, '*');
            BEAST_THROWS(
                object({ {big, nullptr} }),
                std::length_error);
        }
    }

    void
    testArray()
    {
        {
            BEAST_THROWS(
                array(
                    array::max_size()+1,
                    value(nullptr)),
                std::length_error);
        }

        {
            std::vector<int> v(
                array::max_size()+1, 42);
            BEAST_THROWS(
                array(v.begin(), v.end()),
                std::length_error);
        }

        {
            std::vector<int> v(
                array::max_size()+1, 42);
            BEAST_THROWS(array(
                make_input_iterator(v.begin()),
                make_input_iterator(v.end())),
                std::length_error);
        }

        {
            array a;
            BEAST_THROWS(
                a.insert(a.begin(),
                    array::max_size() + 1,
                    nullptr),
                std::length_error);
        }
    }

    void
    testString()
    {
        // strings
        {
            {
                string s;
                BEAST_THROWS(
                    (s.resize(s.max_size() + 1)),
                    std::length_error);
            }

            {
                string s;
                s.resize(100);
                BEAST_THROWS(
                    (s.append(s.max_size() - 1, '*')),
                    std::length_error);
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

        // key in parser
        {
            parser p;
            std::string const big(
                string::max_size() + 1, '*');
            auto const js =
                "{\"" + big + "\":null}";
            BEAST_THROWS(
                parse(js),
                std::length_error);
        }
    }

    void
    testStack()
    {
        // max raw_stack
        {
            std::string big;
            BEAST_THROWS(
                parse(
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
                std::length_error);
        }
    }

    void
    testParser()
    {
        // overflow in on_string_part
        {
            std::string big;
            big = "\\b";
            big += std::string(
                string::max_size()*2, '*');
            auto const js =
                "\"" + big + "\"";
            BEAST_THROWS(
                parse(js),
                std::length_error);
        }

        // overflow in on_string
        {
            std::string big;
            big = "\\b";
            big += std::string(
                (string::max_size()*3)/2, '*');
            auto const js =
                "\"" + big + "\"";
            BEAST_THROWS(
                parse(js),
                std::length_error);
        }
    }

    void
    run() override
    {
    #ifndef BOOST_JSON_NO_MAX_OBJECT_SIZE
        testObject();
    #endif

    #ifndef BOOST_JSON_NO_MAX_ARRAY_SIZE
        testArray();
    #endif

    #ifndef BOOST_JSON_NO_MAX_STRING_SIZE
        testString();
    #endif

    #ifndef BOOST_JSON_NO_MAX_STACK_SIZE
        testStack();
    #endif

    #ifndef BOOST_JSON_NO_PARSER_BUFFER_SIZE
        testParser();
    #endif

        pass();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,limits);

} // json
} // boost
