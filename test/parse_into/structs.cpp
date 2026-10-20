//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2021 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include "test_suite.hpp"


#include "parse_into/common.hpp"

#include <boost/describe/class.hpp>
#include <boost/json/parse_into.hpp>

#include <map>
#include <string>
#include <vector>

#ifdef BOOST_DESCRIBE_CXX14

namespace {

struct X
{
    int a;
    float b;
    std::string c;
};

BOOST_DESCRIBE_STRUCT(X, (), (a, b, c))

bool operator==( X const& x1, X const& x2 )
{
    return x1.a == x2.a && x1.b == x2.b && x1.c == x2.c;
}

struct Y
{
    std::vector<X> v;
    std::map<std::string, X> m;
};

bool operator==( Y const& y1, Y const& y2 )
{
    return y1.v == y2.v && y1.m == y2.m;
}

BOOST_DESCRIBE_STRUCT(Y, (), (v, m))

struct Z : X
{
    Z() = default;
    Z(X x, bool b) : X(x), d(b)
    {}

private:
    bool d = false;

    friend
    bool operator==( Z const& z1, Z const& z2 )
    {
        X const& x1 = z1;
        X const& x2 = z2;
        return (x1 == x2) && z1.d == z2.d;
    }

    BOOST_DESCRIBE_CLASS(Z, (X), (), (), (d))
};

struct W
{
    X x;
};

BOOST_DESCRIBE_STRUCT(W, (), (x))

bool operator==( W const& w1, W const& w2 )
{
    return w1.x == w2.x;
}

} // namespace

#endif // BOOST_DESCRIBE_CXX14

namespace boost {
namespace json {

#ifdef BOOST_DESCRIBE_CXX14

template<>
struct is_described_class<Z> : std::true_type {};

class parse_into_structs_test
{
public:
    void
    run()
    {
        testParseInto<X>( {} );
        testParseInto<X>( { 1, 3.14f, "hello" } );

        testParseInto<Y>( {} );
        testParseInto<Y>( { { { 1, 1.0f, "one" }, { 2, 2.0f, "two" } }, { { "one", { 1, 1.1f, "1" } }, { "two", { 2, 2.2f, "2" } } } } );

        testParseInto<Z>( {} );
        testParseInto<Z>( { {1, 3.14f, "hello"}, true } );

        testParseIntoErrors<X>( error::not_object, 1 );
        testParseIntoErrors<X>( error::size_mismatch, { {"a", 1} } );

        object jo{ {"a", 1}, {"b", 3.14f}, {"c", "hello"} };
        jo["e1"] = array{1, ULONG_MAX, "three", array{}, true, .5, nullptr};
        jo["e2"] = object{ {"one", 1}, {"two", 2}, {"three", object{}} };
        jo["e3"] = "third extra element";
        jo["e4"] = 100;
        jo["e5"] = ULONG_MAX;
        jo["e6"] = 12.4;
        jo["e7"] = false;
        jo["e8"] = nullptr;
        testParseIntoValue<X>(jo);

        object const unexpected_jo{{"x", object{{"unexpectedArray", array{}},
                                                {"unexpectedObject", object{}},
                                                {"unexpectedString", "qwerty"},
                                                {"unexpectedInt", 42},
                                                {"unexpectedUint", ULONG_MAX},
                                                {"unexpectedDouble", 2.71},
                                                {"unexpectedBool", true},
                                                {"unexpectedNull", nullptr},
                                                {"a", 1},
                                                {"b", 3.14f},
                                                {"c", "hello"}}}};
        testParseIntoValue<W>(unexpected_jo);

        system::error_code ec;
        X x{};
        parse_into(x, R"( {"a": 1, "a": 2, "a": 3} )", ec);
        BOOST_TEST( ec == error::size_mismatch );

        std::vector<X> v;
        ec = {};
        parse_into(
            v, R"( [{"a": 1, "b": 1, "c": "one"}, {"a": 2}] )", ec);
        BOOST_TEST( ec == error::size_mismatch );

        ec = {};
        parse_into(
            v,
            R"( [{"a": 1, "b": 1, "c": "one"}, {"a": 2, "b": 2, "c": "two"}] )",
            ec);
        BOOST_TEST( !ec.failed() ) && BOOST_TEST( v.size() == 2 );
    }
};
#else // BOOST_DESCRIBE_CXX14

struct parse_into_structs_test
{
    void
    run()
    {}
};

#endif // BOOST_DESCRIBE_CXX14

TEST_SUITE(parse_into_structs_test, "boost.json.parse_into.structs");

} // namespace json
} // namespace boost
