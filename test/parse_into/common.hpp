//
// Copyright (c) 2026 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_TEST_PARSE_INTO_COMMON_HPP
#define BOOST_JSON_TEST_PARSE_INTO_COMMON_HPP

#include <boost/json/parse.hpp>
#include <boost/json/parse_into.hpp>
#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {
namespace {

template<class T>
void testParseIntoValue(
    value const& jv, source_location const& loc = BOOST_CURRENT_LOCATION)
{
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
    T t1 = value_to<T>(jv);
    (void)t1; // older GCC thinks t1 can be unused
    std::string json = serialize(jv);

    T t2{};
    system::error_code jec;
    parse_into(t2, json, jec);
    BOOST_TEST( !jec.failed(), loc );
    BOOST_TEST( t1 == t2, loc );

    T t3{};
    std::error_code ec;
    parse_into(t3, json, ec);
    BOOST_TEST( !ec, loc );
    BOOST_TEST( t1 == t3, loc );

    T t4{};
    parse_into(t4, json);
    BOOST_TEST( t1 == t4, loc );

    std::istringstream is(json);
    T t5{};
    jec = {};
    parse_into(t5, is, jec);
    BOOST_TEST( !jec.failed(), loc );
    BOOST_TEST( t1 == t5, loc );

    is.clear();
    is.seekg(0);
    T t6{};
    ec = {};
    parse_into(t6, is, ec);
    BOOST_TEST( !ec, loc );
    BOOST_TEST( t1 == t6, loc );

    is.str(json);
    is.clear();
    T t7{};
    parse_into(t7, is);
    BOOST_TEST( t1 == t7, loc );

    parse_options opt;
    opt.allow_comments = true;
    json = "// this is a comment\n" + json;

    T t8{};
    parser_for<T> p( opt, &t8 );
    for( auto& c: json )
    {
        std::size_t const n = p.write_some( true, &c, 1, jec );
        BOOST_TEST( !jec.failed(), loc );
        BOOST_TEST( n == 1, loc );
    }
    p.write_some(false, nullptr, 0, jec);
    BOOST_TEST( !jec.failed(), loc );
    BOOST_TEST( t1 == t8, loc );
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic pop
#endif
}

template<class T>
void testParseInto(
    T const& t, source_location const& loc = BOOST_CURRENT_LOCATION)
{
    testParseIntoValue<T>(value_from(t), loc);
}

template<class T>
void
testParseIntoErrors(
    error e,
    value const& sample,
    source_location const& loc = BOOST_CURRENT_LOCATION )
{
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
    system::error_code ec;
    T t1{};
    std::string json = serialize(sample);

    parse_into<T>(t1, json, ec);
    BOOST_TEST( ec.failed(), loc );
    BOOST_TEST( ec.has_location(), loc );
    BOOST_TEST( ec == e, loc );

    T t2{};
    ec = {};
    parser_for<T> p( parse_options{}, &t2 );
    for( auto& c: json )
    {
        std::size_t const n = p.write_some( true, &c, 1, ec );
        if( ec.failed() )
            break;
        BOOST_TEST( n == 1, loc );
    }
    if( !ec.failed() )
        p.write_some(false, nullptr, 0, ec);

    BOOST_TEST( ec.failed(), loc );
    BOOST_TEST( ec.has_location(), loc );
    BOOST_TEST( ec == e, loc );
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic pop
#endif
}

} // namespace
} // namespace json
} // namespace boost

#endif // BOOST_JSON_TEST_PARSE_INTO_COMMON_HPP
