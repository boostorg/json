//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

// Test that header file is self-contained.
#include <boost/json/visit.hpp>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class visit_test
{
public:
    struct check
    {
        kind k;
        bool operator()(std::nullptr_t) { return k == kind::null; }
        bool operator()(bool) { return k == kind::bool_; }
        bool operator()(std::int64_t) { return k == kind::int64; }
        bool operator()(std::uint64_t) { return k == kind::uint64; }
        bool operator()(double) { return k == kind::double_; }
        bool operator()(string const&) { return k == kind::string; }
        bool operator()(array const&) { return k == kind::array; }
        bool operator()(object const&) { return k == kind::object; }
        bool operator()(...) { return false; }
    };

    void
    testVisit()
    {
        BOOST_TEST(visit(check{kind::null},    value(nullptr)));
        BOOST_TEST(visit(check{kind::bool_},   value(true)));
        BOOST_TEST(visit(check{kind::int64},   value(1)));
        BOOST_TEST(visit(check{kind::uint64},  value(1UL)));
        BOOST_TEST(visit(check{kind::double_}, value(1.5)));
        BOOST_TEST(visit(check{kind::string},  value(string_kind)));
        BOOST_TEST(visit(check{kind::array},   value(array_kind)));
        BOOST_TEST(visit(check{kind::object},  value(object_kind)));
    }

    void run()
    {
        testVisit();
    }
};

TEST_SUITE(visit_test, "boost.json.visit");

BOOST_JSON_NS_END
