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

#include <boost/json/serialize.hpp>
#include <boost/json/value_from.hpp>
#include <boost/describe.hpp>

#include <climits>
#include <map>

#include "test.hpp"
#include "test_suite.hpp"

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

BOOST_DESCRIBE_STRUCT(Y, (), (v, m))

bool operator==( Y const& y1, Y const& y2 )
{
    return y1.v == y2.v && y1.m == y2.m;
}

BOOST_DEFINE_ENUM_CLASS(E, x, y, z)

namespace boost {
namespace json {

class parse_into_test
{
public:

    template<class T> void testParseInto( T const& t )
    {
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
        T t1( t );
        std::string json = serialize( value_from( t1 ) );

        T t2{};
        error_code jec;
        parse_into(t2, json, jec);
        BOOST_TEST( !jec.failed() ) && BOOST_TEST( t1 == t2 );

        T t3{};
        std::error_code ec;
        parse_into(t3, json, ec);
        BOOST_TEST( !ec ) && BOOST_TEST( t1 == t3 );

        T t4{};
        parse_into(t4, json);
        BOOST_TEST( t1 == t4 );

        std::istringstream is(json);
        T t5{};
        jec = {};
        parse_into(t5, is, jec);
        BOOST_TEST( !jec.failed() ) && BOOST_TEST( t1 == t5 );

        is.clear();
        is.seekg(0);
        T t6{};
        ec = {};
        parse_into(t6, is, ec);
        BOOST_TEST( !ec ) && BOOST_TEST( t1 == t6 );

        is.clear();
        is.seekg(0);
        T t7{};
        parse_into(t7, is);
        BOOST_TEST( t1 == t7 );
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic pop
#endif
    }

    template<class T>
    void
    testParseIntoErrors( error e, value const& sample )
    {
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
        error_code ec;
        T t{};
        std::string json = serialize(sample);
        parse_into(t, json, ec);

        BOOST_TEST( ec.failed() );
        BOOST_TEST( ec.has_location() );
        BOOST_TEST( ec == e );
#if defined(__GNUC__) && __GNUC__ < 5
# pragma GCC diagnostic pop
#endif
    }

    void testNull()
    {
        testParseInto( nullptr );
        testParseIntoErrors< std::nullptr_t >( error::not_null, 1 );
    }

    void testBoolean()
    {
        testParseInto( false );
        testParseInto( true );
        testParseIntoErrors< bool >( error::not_bool, 1 );
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

        testParseIntoErrors< int >( error::not_integer, "1" );
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

        testParseIntoErrors< double >( error::not_double, "1" );
    }

    void testString()
    {
        testParseInto<std::string>( "" );
        testParseInto<std::string>( "12345" );

        testParseIntoErrors< string >( error::not_string, 1 );
    }

    void testSequence()
    {
        testParseInto<std::vector<std::nullptr_t>>( { nullptr, nullptr } );

        testParseInto< std::vector<bool> >( {} );
        testParseInto< std::vector<bool> >( { true, false } );

        testParseInto< std::vector<int> >( {} );
        testParseInto< std::vector<int> >( { 1, 2, 3 } );

        testParseInto< std::vector<float> >( {} );
        testParseInto< std::vector<float> >( { 1.02f, 2.11f, 3.14f } );

        testParseInto< std::vector<std::string> >( {} );
        testParseInto< std::vector<std::string> >( { "one", "two", "three" } );

        testParseInto< std::vector<std::vector<int>> >( {} );
        testParseInto< std::vector<std::vector<int>> >( { {}, { 1 }, { 2, 3 }, { 4, 5, 6 } } );

        // clang <= 5 doesn't like when std::array is created from init-list
        std::array<int, 4> arr;
        arr.fill(17);
        testParseInto< std::array<int, 4> >( arr );

        testParseIntoErrors< std::vector<int> >( error::not_array, 1 );
        testParseIntoErrors< std::vector<char> >( error::not_array, "abcd" );
        testParseIntoErrors< std::vector<std::vector<int>> >(
            error::not_array, { 1, 2, 3} );
    }

    void testMap()
    {
        testParseInto< std::map<std::string, int> >( {} );
        testParseInto< std::map<std::string, int> >( { { "one", 1 }, { "two", 2 } } );

        testParseInto< std::map<std::string, int> >( {} );
        testParseInto< std::map<std::string, int> >( { { "one", 1 }, { "two", 2 } } );

        testParseInto< std::map<std::string, std::vector<int>> >( {} );
        testParseInto< std::map<std::string, std::vector<int>> >( { { "one", { 1 } }, { "two", { 2, 3 } } } );

        testParseInto< std::map<std::string, std::map<std::string, int>> >( {} );
        testParseInto< std::map<std::string, std::map<std::string, int>> >( { { "one", {} }, { "two", { { "1", 1 }, { "2", 2 } } } } );

        testParseIntoErrors< std::map<std::string, int> >(
            error::not_object, { "1", 1, "2", 2} );
        testParseIntoErrors< std::map<std::string, std::map<std::string, int>> >(
            error::not_object, { {"1", {}}, {"2", {"3", 4}} } );
    }

    void testTuple()
    {
        testParseInto<std::pair<int, float>>( {} );
        testParseInto<std::pair<int, float>>( { 1, 3.14f } );

        testParseInto<std::tuple<int, float, std::string>>( {} );
        testParseInto<std::tuple<int, float, std::string>>( std::make_tuple(1, 3.14f, "hello") );

        testParseInto<std::vector<std::pair<int, int>>>( {} );
        testParseInto<std::vector<std::pair<int, int>>>( { { 1, 2 }, { 3, 4 } } );

        testParseInto<std::vector<std::vector<std::pair<int, int>>>>( {} );
        testParseInto<std::vector<std::vector<std::pair<int, int>>>>( { { { 1, 2 }, { 3, 4 } } } );
        testParseInto<std::vector<std::vector<std::pair<int, int>>>>( { { { 1, 2 }, { 3, 4 } }, { { 5, 6 }, { 7, 8 } } } );

        testParseInto<std::map<std::string, std::vector<std::pair<int, int>>>>( {} );
        testParseInto<std::map<std::string, std::vector<std::pair<int, int>>>>( { { "one", {} } } );
        testParseInto<std::map<std::string, std::vector<std::pair<int, int>>>>( { { "one", { { 1, 2 }, { 3, 4 } } } } );

        testParseInto<std::pair<std::vector<int>, std::map<std::string, std::pair<int, bool>>>>( {} );
        testParseInto<std::pair<std::vector<int>, std::map<std::string, std::pair<int, bool>>>>( { { 1, 2, 3 }, { { "one", { 7, true } } } } );

        testParseIntoErrors< std::pair<int, int> >( error::not_array, 1 );
        // these two should be errors too
        // testParseIntoErrors< std::pair<int, int> >(
        //     error::size_mismatch, {1, 2, 3} );
        // testParseIntoErrors< std::tuple<int, int, int> >(
        //     error::size_mismatch, {1, 2} );
    }

    void testStruct()
    {
#if defined(BOOST_DESCRIBE_CXX14)
        testParseInto<X>( {} );
        testParseInto<X>( { 1, 3.14f, "hello" } );

        testParseInto<Y>( {} );
        testParseInto<Y>( { { { 1, 1.0f, "one" }, { 2, 2.0f, "two" } }, { { "one", { 1, 1.1f, "1" } }, { "two", { 2, 2.2f, "2" } } } } );

        testParseIntoErrors<X>( error::not_object, 1 );
        testParseIntoErrors<X>(
            error::unknown_name,
            { {"a", 1}, {"b", 3.14f}, {"c", "hello"}, {"d", 0} } );
        // this two should be an error too
        // testParseIntoErrors<X>( error::size_mismatch, { {"a", 1} } );
#endif
    }

    void testEnum()
    {
#ifdef BOOST_DESCRIBE_CXX14
        testParseInto<E>( E::x );
        testParseInto<E>( E::y );
        testParseInto<E>( E::z );

        testParseIntoErrors< E >( error::not_string, (int)(E::y) );
#endif // BOOST_DESCRIBE_CXX14
    }

    template< template<class...> class Variant, class Monostate >
    void testVariant()
    {
        testParseInto< Variant<int> >( 1 );
        testParseInto< Variant<int, std::string> >( 1 );
        testParseInto< Variant<int, std::string> >( "qwerty" );
        testParseInto< Variant<Monostate, int, std::string> >( {} );
        testParseInto< Variant<Monostate, int, std::string> >( 1 );
        testParseInto< Variant<Monostate, int, std::string> >( "qwerty" );

        testParseInto< Variant< std::vector<int> > >(
            std::vector<int>{1, 2, 3, 4, 5} );
        testParseInto< Variant< Monostate, std::vector<int> > >(
            std::vector<int>{1, 2, 3, 4, 5} );

        testParseInto< std::vector< Variant<int, std::string> > >(
            {1, 2, 3, "four", 5, "six", "seven", 8});

        using V = Variant<
            std::vector< int >,
            std::tuple< int, std::string, std::map<std::string, int> >,
            std::tuple< int, std::string, std::map<std::string, double> > >;
        testParseInto< V >(
            std::make_tuple(
                5,
                "five",
                std::map<std::string, double>{ {"one", 1}, {"pi", 3.14} }));

        testParseIntoErrors< Variant<Monostate> >(
            error::exhausted_variants, "a" );
        testParseIntoErrors< Variant<int> >(
            error::exhausted_variants, "a" );
        testParseIntoErrors< Variant<Monostate, int, bool> >(
            error::exhausted_variants, "a" );
    }

    void testOptional()
    {
#ifndef BOOST_NO_CXX17_HDR_OPTIONAL
        testParseInto< std::optional<int> >( std::nullopt );
        testParseInto< std::optional<int> >( 1 );

        testParseInto< std::optional<std::vector<std::nullptr_t>> >(
            std::nullopt );
        testParseInto< std::optional<std::vector<std::nullptr_t>> >(
           std::vector<std::nullptr_t>{} );
        testParseInto< std::optional<std::vector<std::nullptr_t>> >(
           std::vector<std::nullptr_t>{nullptr, nullptr} );

        testParseInto< std::optional<std::vector<std::string>> >(
           std::vector<std::string>{"1", "2", "3"} );

        testParseInto< std::vector< std::optional<int> > >(
            {1, 2, 3, std::nullopt, 5, std::nullopt, std::nullopt, 8});
#endif
    }

    void run()
    {
        testNull();
        testBoolean();
        testIntegral();
        testFloatingPoint();
        testString();
        testSequence();
        testMap();
        testTuple();
        testStruct();
        testEnum();
        testOptional();
        testVariant<variant2::variant, variant2::monostate>();
#ifndef BOOST_NO_CXX17_HDR_VARIANT
        testVariant<std::variant, std::monostate>();
#endif
    }
};

TEST_SUITE(parse_into_test, "boost.json.parse_into");

} // namespace boost
} // namespace json
