//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2021 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

// Test that header file is self-contained.
#include <boost/json/parse_into.hpp>

#include "test_suite.hpp"
#include "parse_into/common.hpp"

#include <boost/describe/enum.hpp>

#ifndef BOOST_NO_CXX17_HDR_FILESYSTEM
# include <filesystem>
#endif // BOOST_NO_CXX17_HDR_FILESYSTEM

namespace {

BOOST_DEFINE_ENUM_CLASS(E, x, y, z)

} // namespace

namespace boost {
namespace json {

class parse_into_scalars_test
{
public:
    void testNull()
    {
        testParseInto( nullptr );
        testParseIntoErrors< std::nullptr_t >( error::not_null, 100 );
    }

    void testBoolean()
    {
        testParseInto( false );
        testParseInto( true );
        testParseIntoErrors< bool >( error::not_bool, nullptr );
    }

    void testIntegral()
    {
        testParseInto<char>( 'A' ); // ?
        testParseInto<signed char>( -127 );
        testParseInto<unsigned char>( 255 );
        testParseInto<short>( -32767 );
        testParseInto<unsigned short>( 65535 );
        testParseInto<int>( -32767 );
        testParseInto<unsigned int>( 65535 );
        testParseInto<long>( LONG_MIN );
        testParseInto<unsigned long>( ULONG_MAX );
        testParseInto<long long>( LLONG_MIN );
        testParseInto<unsigned long long>( ULLONG_MAX );

        testParseIntoErrors< int >( error::not_integer, "123" );
        testParseIntoErrors< int >( error::not_integer, true );
        testParseIntoErrors< int >( error::not_exact, LLONG_MIN );
        testParseIntoErrors< int >( error::not_exact, ULONG_MAX );
    }

    void testFloatingPoint()
    {
        testParseInto( 0.25f );
        testParseInto( 1.125 );
        // value_from doesn't support long double
        // testParseInto( 2.25L );
        {
            double d1 = 12;
            std::string json = serialize( value_from( 12 ) );

            system::error_code ec;
            double d2;
            parse_into(d2, json, ec);
            BOOST_TEST( !ec.failed() );
            BOOST_TEST( d1 == d2 );

            d1 = double(UINT64_MAX);
            json = serialize( value_from( UINT64_MAX ) );
            parse_into(d2, json, ec);
            BOOST_TEST( !ec.failed() );
            BOOST_TEST( d1 == d2 );
        }

        testParseIntoErrors< double >( error::not_double, { {"value", 12.1 } } );
    }

    void testString()
    {
        testParseInto<std::string>( "" );
        testParseInto<std::string>( "12345" );

        testParseIntoErrors< string >( error::not_string, UINT64_MAX );

        std::string s;
        parse_into(s, R"( "foobar" )");
        BOOST_TEST( s.size() == 6 );

        parse_into(s, R"( "qwertyuiop" )");
        BOOST_TEST( s.size() == 10 );
    }

    void testPath()
    {
#ifndef BOOST_NO_CXX17_HDR_FILESYSTEM
        testParseInto< std::filesystem::path >( "c:/" );
        testParseInto< std::filesystem::path >( ".." );
        testParseInto< std::filesystem::path >( "../" );
#endif // BOOST_NO_CXX17_HDR_FILESYSTEM
    }

    void testEnum()
    {
#ifdef BOOST_DESCRIBE_CXX14
        testParseInto<E>( E::x );
        testParseInto<E>( E::y );
        testParseInto<E>( E::z );

        testParseInto< std::vector<E> >( {E::x, E::y, E::z} );

        testParseIntoErrors< E >( error::not_string, (int)(E::y) );
        testParseIntoErrors< E >( error::unknown_name, "zoom" );
#endif // BOOST_DESCRIBE_CXX14
    }

    void run()
    {
        testNull();
        testBoolean();
        testIntegral();
        testFloatingPoint();
        testString();
        testPath();
        testEnum();

        {
            int n;
            BOOST_TEST_THROWS_WITH_LOCATION( parse_into( n, "null" ) );

            std::stringstream is("null");
            BOOST_TEST_THROWS_WITH_LOCATION( parse_into( n, is ) );
        }

        {
            int n;
            system::error_code ec;
            parse_into( n, "12 1", ec);
            BOOST_TEST( ec == error::extra_data );
            BOOST_TEST( ec.has_location() );
        }

        {
            std::stringstream is("12 1");
            int n;
            system::error_code ec;
            parse_into( n, is, ec);
            BOOST_TEST( ec == error::extra_data );
            BOOST_TEST( ec.has_location() );
        }

        {
            int n;
            std::stringstream is("1");
            is.setstate( std::ios::failbit );
            system::error_code ec;
            parse_into(n, is, ec);
            BOOST_TEST( ec == error::input_error );
            BOOST_TEST( ec.has_location() );
        }
    }
};

TEST_SUITE(parse_into_scalars_test, "boost.json.parse_into.scalars");

} // namespace json
} // namespace boost
