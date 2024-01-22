//
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/value.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

class pointer_test
{
    bool
    hasLocation(error_code const& ec)
    {
        return ec.has_location();
    }

    bool
    hasLocation(std::error_code const&)
    {
        return true;
    }

    value
    testValue() const
    {
        return value{
            {"foo", {"bar", "baz", "baf"}},
            {"", 0},
            {"a/b", 1},
            {"c%d", 2},
            {"e^f", 3},
            {"g|h", 4},
            {"i\\j", 5},
            {"k\"l", 6},
            {" ", 7},
            {"m~n", 8},
            {"x", object{{"y", "z"}}},
        };
    }

    value
    bigObject() const
    {
        object result;
        for (int i = 0; i < 50; ++i)
        {
            result.emplace(std::to_string(i), i);
        }
        return result;
    }

public:
    void
    testRootPointer()
    {
        value const jv = testValue();
        BOOST_TEST(&jv.at_pointer("") == &jv);
    }

    void
    testChildPointer()
    {
        value const jv = testValue();
        BOOST_TEST(&jv.at_pointer("/foo")== &jv.at("foo"));
        BOOST_TEST(&jv.at_pointer("/c%d")== &jv.at("c%d"));
        BOOST_TEST(&jv.at_pointer("/e^f")== &jv.at("e^f"));
        BOOST_TEST(&jv.at_pointer("/g|h")== &jv.at("g|h"));
        BOOST_TEST(&jv.at_pointer("/")== &jv.at(""));
        BOOST_TEST(&jv.at_pointer("/i\\j")== &jv.at("i\\j"));
        BOOST_TEST(&jv.at_pointer("/k\"l")== &jv.at("k\"l"));
        BOOST_TEST(&jv.at_pointer("/ ")== &jv.at(" "));
    }

    void
    testEscaped()
    {
        value const jv = testValue();
        BOOST_TEST(&jv.at_pointer("/a~1b")== &jv.at("a/b"));
        BOOST_TEST(&jv.at_pointer("/m~0n")== &jv.at("m~n"));
    }

    void
    testNested()
    {
        value const jv = testValue();
        BOOST_TEST(&jv.at_pointer("/foo/0") == &jv.at("foo").at(0));
        BOOST_TEST(&jv.at_pointer("/foo/1") == &jv.at("foo").at(1));
        BOOST_TEST(&jv.at_pointer("/x/y") == &jv.at("x").at("y"));

        {
            value v1;
            object& o1 = v1.emplace_object();
            object& o2 = (o1["very"] = value()).emplace_object();
            object& o3 = (o2["deep"] = value()).emplace_object();

            array& a1 = (o3["path"] = value()).emplace_array();
            a1.emplace_back(value());

            array& a2 = a1.emplace_back(value()).emplace_array();
            a2.emplace_back(value());
            a2.emplace_back(value());

            array& a3 = a2.emplace_back(value()).emplace_array();
            object& o4 = a3.emplace_back(value()).emplace_object();
            object& o5 = (o4["0"] = value()).emplace_object();
            value& v2 = o5["fin"] = value();

            BOOST_TEST(&v1.at_pointer("/very/deep/path/1/2/0/0/fin") == &v2);
        }
    }

    void
    testErrors()
    {
        value const jv = testValue();
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("foo") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/fo") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/m~") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/m~n") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/foo/bar") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/foo/") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/foo/01") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/foo/2b") );
        BOOST_TEST_THROWS_WITH_LOCATION( jv.at_pointer("/x/y/z") );
    }

    void
    testTry()
    {
        error_code ec;

        ec = value().try_at_pointer("/x").error();
        BOOST_TEST( ec == error::value_is_scalar );
        BOOST_TEST( ec.has_location() );

        value jv = testValue();
        auto const& cjv = jv;

        BOOST_TEST(
            &jv.try_at_pointer("/foo/0").value() == &jv.at("foo").at(0));

        BOOST_TEST(
            &cjv.try_at_pointer("/foo/1").value() == &jv.at("foo").at(1));

        ec = jv.try_at_pointer("foo").error();
        BOOST_TEST( ec == error::missing_slash );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/fo").error();
        BOOST_TEST( ec == error::not_found );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/25").error();
        BOOST_TEST( ec == error::not_found );
        BOOST_TEST( ec.has_location() );

        ec = value( object() ).try_at_pointer("/foo").error();
        BOOST_TEST( ec == error::not_found );
        BOOST_TEST( ec.has_location() );

        ec = bigObject().try_at_pointer("/foo").error();
        BOOST_TEST( ec == error::not_found );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/m~").error();
        BOOST_TEST( ec == error::invalid_escape );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/m~n").error();
        BOOST_TEST( ec == error::invalid_escape );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/bar").error();
        BOOST_TEST( ec == error::token_not_number );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/").error();
        BOOST_TEST( ec == error::token_not_number );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/01").error();
        BOOST_TEST( ec == error::token_not_number );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/2b").error();
        BOOST_TEST( ec == error::token_not_number );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/2.").error();
        BOOST_TEST( ec == error::token_not_number );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/-").error();
        BOOST_TEST( ec == error::past_the_end );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/-/x").error();
        BOOST_TEST( ec == error::past_the_end );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/foo/-1").error();
        BOOST_TEST( ec == error::token_not_number );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_at_pointer("/x/y/z").error();
        BOOST_TEST( ec == error::value_is_scalar );
        BOOST_TEST( ec.has_location() );

        string s = "/foo/";
        s += std::to_string((std::numeric_limits<std::size_t>::max)());
        if ( '9' == s[s.size() - 1] )
        {
            for (std::size_t i = 6; i < s.size(); ++i)
            {
                s[i] = '0';
            }
            s[5] = '1';
            s += "0";
        }
        else
        {
            ++s[s.size() - 1];
        }
        ec = jv.try_at_pointer(s).error();
        BOOST_TEST( ec == error::token_overflow );
        BOOST_TEST( ec.has_location() );
    }

    void
    testNonThrowing()
    {
        error_code jec;
        BOOST_TEST( !value().find_pointer("/x", jec) );
        BOOST_TEST( jec == error::value_is_scalar );
        BOOST_TEST( jec.has_location() );

        value jv = testValue();
        BOOST_TEST( jv.find_pointer("/foo/0", jec) == &jv.at("foo").at(0) );
        BOOST_TEST( !jec.failed() );

        auto const& cjv = jv;
        jec = error::syntax;
        BOOST_TEST( cjv.find_pointer("/foo/1", jec) == &jv.at("foo").at(1) );
        BOOST_TEST( !jec.failed() );

        std::error_code ec;
        BOOST_TEST( !value().find_pointer("/x", ec) );
        BOOST_TEST( ec == error::value_is_scalar );

        jv = testValue();
        BOOST_TEST(jv.find_pointer("/foo/0", ec) == &jv.at("foo").at(0));
        BOOST_TEST( !ec );

        ec = error::syntax;
        BOOST_TEST( cjv.find_pointer("/foo/1", ec) == &jv.at("foo").at(1) );
        BOOST_TEST( !ec );
    }

    void
    testSet()
    {
        value* result;
        value jv;
        result = &jv.set_at_pointer("", array());
        BOOST_TEST(( jv == array() ));
        BOOST_TEST( result == &jv );

        result = &jv.set_at_pointer("/0", 10);
        BOOST_TEST(( jv == array{10} ));
        BOOST_TEST( *result == 10 );

        result = &jv.set_at_pointer("/-", 20);
        BOOST_TEST(( jv == array{10, 20} ));
        BOOST_TEST( *result == 20 );

        result = &jv.set_at_pointer("/-", 30);
        BOOST_TEST(( jv == array{10, 20, 30} ));
        BOOST_TEST( *result == 30 );

        result = &jv.set_at_pointer("/1", 25);
        BOOST_TEST(( jv == array{10, 25, 30} ));
        BOOST_TEST( *result == 25 );

        result = &jv.set_at_pointer("/-/x", 1);
        BOOST_TEST(( jv == array{10, 25, 30, object{{"x", 1}}} ));
        BOOST_TEST( *result == 1 );

        result = &jv.set_at_pointer("/3/0", 2);
        BOOST_TEST(( jv == array{10, 25, 30, object{{"x", 1}, {"0", 2}}} ));
        BOOST_TEST( *result == 2 );

        result = &jv.set_at_pointer("/-/0", 3);
        BOOST_TEST((
            jv == array{
                10,
                25,
                30,
                object{{"x", 1}, {"0", 2}},
                array{3} } ));
        BOOST_TEST( *result == 3 );

        BOOST_TEST_THROWS_WITH_LOCATION( jv.set_at_pointer("/0/1", 1) );

        jv = value();
        result = &jv.set_at_pointer("/a/b/c/d/e/f/g/h/i/j/k/l", "m");
        BOOST_TEST( *result == "m" );
        BOOST_TEST( result == &jv.at_pointer("/a/b/c/d/e/f/g/h/i/j/k/l") );

        result = &jv.set_at_pointer("/a/b/c/d/e/f/g/h/i/j/k/l", "n");
        BOOST_TEST( *result == "n" );
        BOOST_TEST( result == &jv.at_pointer("/a/b/c/d/e/f/g/h/i/j/k/l") );

        set_pointer_options opts;
        opts.replace_any_scalar = true;
        jv = 1;
        result = &jv.set_at_pointer( "/x", 1, opts );
        BOOST_TEST(( jv == object{ {"x", 1} } ));
        BOOST_TEST( *result == 1 );
        BOOST_TEST( result == &jv.at_pointer("/x") );

        opts = {};
        opts.max_created_elements = 5;
        jv = array();
        BOOST_TEST_THROWS_WITH_LOCATION( jv.set_at_pointer("/5", 1, opts) );
        result = &jv.set_at_pointer( "/4", 0, opts );
        BOOST_TEST(( jv == array{nullptr, nullptr, nullptr, nullptr, 0} ));
        BOOST_TEST( *result == 0 );
        BOOST_TEST( result == &jv.at_pointer("/4") );

        opts = {};
        opts.create_arrays = false;
        opts.create_objects = false;
        jv = object();
        result = &jv.set_at_pointer( "/x", 1, opts );
        BOOST_TEST(( jv == object{ {"x", 1} } ));
        BOOST_TEST( *result == 1 );
        BOOST_TEST( result == &jv.at_pointer("/x") );
    }


    void
    testTrySet()
    {
        error_code ec;
        value jv;

        BOOST_TEST( jv.try_set_at_pointer( "", array() ).value().is_array() );

        ec = jv.try_set_at_pointer("/1", 0).error();
        BOOST_TEST( ec == error::not_found );
        BOOST_TEST( ec.has_location() );

        ec = jv.try_set_at_pointer("/x", 0).error();
        BOOST_TEST( ec == error::token_not_number );
        BOOST_TEST( ec.has_location() );

        BOOST_TEST( jv.try_set_at_pointer("/-", "").value().is_string() );

        ec = jv.try_set_at_pointer("/0/x", 1).error();
        BOOST_TEST( ec == error::value_is_scalar );
        BOOST_TEST( ec.has_location() );
    }

    void
    testSetNonThrowing()
    {
        error_code jec;
        value jv;

        value* p = jv.set_at_pointer("", array(), jec);
        BOOST_TEST( p && p->is_array() );
        BOOST_TEST( !jec.failed() );

        BOOST_TEST( !jv.set_at_pointer("/1", 0, jec) );
        BOOST_TEST( jec == error::not_found );
        BOOST_TEST( jec.has_location() );

        jec = {};

        BOOST_TEST( !jv.set_at_pointer("/x", 0, jec) );
        BOOST_TEST( jec == error::token_not_number );
        BOOST_TEST( jec.has_location() );

        std::error_code ec;
        jv = {};

        p = jv.set_at_pointer("", array(), ec);
        BOOST_TEST( p && p->is_array() );
        BOOST_TEST( !ec );

        BOOST_TEST( !jv.set_at_pointer("/1", 0, ec) );
        BOOST_TEST( ec == error::not_found );

        ec = {};

        BOOST_TEST( !jv.set_at_pointer("/x", 0, ec) );
        BOOST_TEST( ec == error::token_not_number );
    }

    void
    run()
    {
        testRootPointer();
        testChildPointer();
        testEscaped();
        testNested();
        testErrors();
        testTry();
        testNonThrowing();
        testSet();
        testTrySet();
        testSetNonThrowing();
    }
};

TEST_SUITE(pointer_test, "boost.json.pointer");

} // namespace json
} // namespace boost
