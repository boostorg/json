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
    struct check_mutable
    {
        kind k;
        bool operator()(std::nullptr_t) { return k == kind::null; }
        bool operator()(bool&) { return k == kind::bool_; }
        bool operator()(std::int64_t&) { return k == kind::int64; }
        bool operator()(std::uint64_t&) { return k == kind::uint64; }
        bool operator()(double&) { return k == kind::double_; }
        bool operator()(string &) { return k == kind::string; }
        bool operator()(array &) { return k == kind::array; }
        bool operator()(object &) { return k == kind::object; }
        bool operator()(...) { return false; }
    };

    struct check_const
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
    testVisitMutable()
    {
        {
            json::value v = value(nullptr);
            BOOST_TEST(visit(check_mutable{kind::null}, v));
        }

        {
            json::value v = value(true);
            BOOST_TEST(visit(check_mutable{kind::bool_}, v));
        }

        {
            json::value v = value(1);
            BOOST_TEST(visit(check_mutable{kind::int64}, v));
        }

        {
            json::value v = value(std::uint64_t(1));
            BOOST_TEST(visit(check_mutable{kind::uint64}, v));
        }

        {
            json::value v = value(1.5);
            BOOST_TEST(visit(check_mutable{kind::double_}, v));
        }

        {
            json::value v = value(string_kind);
            BOOST_TEST(visit(check_mutable{kind::string}, v));
        }

        {
            json::value v = value(array_kind);
            BOOST_TEST(visit(check_mutable{kind::array}, v));
        }

        {
            json::value v = value(object_kind);
            BOOST_TEST(visit(check_mutable{kind::object}, v));
        }
    }

    void
    testVisitConst()
    {
        {
            json::value const v = value(nullptr);
            BOOST_TEST(visit(check_const{kind::null}, v));
        }

        {
            json::value const v = value(true);
            BOOST_TEST(visit(check_const{kind::bool_}, v));
        }

        {
            json::value const v = value(1);
            BOOST_TEST(visit(check_const{kind::int64}, v));
        }

        {
            json::value const v = value(std::uint64_t(1));
            BOOST_TEST(visit(check_const{kind::uint64}, v));
        }

        {
            json::value const v = value(1.5);
            BOOST_TEST(visit(check_const{kind::double_}, v));
        }

        {
            json::value const v = value(string_kind);
            BOOST_TEST(visit(check_const{kind::string}, v));
        }

        {
            json::value const v = value(array_kind);
            BOOST_TEST(visit(check_const{kind::array}, v));
        }

        {
            json::value const v = value(object_kind);
            BOOST_TEST(visit(check_const{kind::object}, v));
        }
    }

    void run()
    {
        testVisitMutable();
        testVisitConst();
    }
};

TEST_SUITE(visit_test, "boost.json.visit");

BOOST_JSON_NS_END
