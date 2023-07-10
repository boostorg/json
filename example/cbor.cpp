//
// Copyright 2020 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

//[example_cbor

/*
    This example implements simple parsing and serialization of a
    subset of CBOR types that are directly supported by JSON.
*/

#include <boost/json.hpp>
#include <boost/endian.hpp>
#include <stdexcept>
#include <fstream>
#include <iostream>

using namespace boost::json;

void serialize_cbor_number(
    unsigned char mt, std::uint64_t n, std::vector<unsigned char> & out )
{
    mt <<= 5;

    if( n < 24 )
    {
        out.push_back( static_cast<unsigned char>( mt + n ) );
    }
    else if( n < 256 )
    {
        unsigned char data[] = { static_cast<unsigned char>( mt + 24 ), static_cast<unsigned char>( n ) };
        out.insert( out.end(), std::begin( data ), std::end( data ) );
    }
    else if( n < 65536 )
    {
        unsigned char data[] = { static_cast<unsigned char>( mt + 25 ), static_cast<unsigned char>( n >> 8 ), static_cast<unsigned char>( n ) };
        out.insert( out.end(), std::begin( data ), std::end( data ) );
    }
    else if( n < 0x1000000ull )
    {
        unsigned char data[ 5 ];

        data[ 0 ] = static_cast<unsigned char>( mt + 26 );
        boost::endian::endian_store<std::uint32_t, 4, boost::endian::order::big>( data + 1, static_cast<std::uint32_t>( n ) );

        out.insert( out.end(), std::begin( data ), std::end( data ) );
    }
    else
    {
        unsigned char data[ 9 ];

        data[ 0 ] = static_cast<unsigned char>( mt + 27 );
        boost::endian::endian_store<std::uint64_t, 8, boost::endian::order::big>( data + 1, n );

        out.insert( out.end(), std::begin( data ), std::end( data ) );
    }
}

void
serialize_cbor_string( string_view sv, std::vector<unsigned char>& out )
{
    std::size_t n = sv.size();
    serialize_cbor_number( 3, n, out );

    out.insert( out.end(), sv.data(), sv.data() + n );
}

void
serialize_cbor_value( const value& jv, std::vector<unsigned char>& out )
{
    switch( jv.kind() )
    {
    case kind::null:
        out.push_back( 224 + 22 );
        break;

    case kind::bool_:
        out.push_back( 224 + 20 + jv.get_bool() );
        break;

    case kind::int64:
        {
            std::int64_t n = jv.get_int64();
            if( n >= 0 )
                serialize_cbor_number( 0, n, out );
            else
                serialize_cbor_number( 1, ~n, out );
        }
        break;

    case kind::uint64:
        serialize_cbor_number( 0, jv.get_uint64(), out );
        break;

    case kind::double_:
        {
            unsigned char data[ 9 ];
            data[ 0 ] = 224 + 27;
            boost::endian::endian_store<double, 8, boost::endian::order::big>( data + 1, jv.get_double() );

            out.insert( out.end(), std::begin(data), std::end(data) );
        }
        break;

    case kind::string:
        serialize_cbor_string( jv.get_string(), out );
        break;

    case kind::array:
        {
            const array& ja = jv.get_array();
            std::size_t n = ja.size();

            out.reserve( out.size() + n + 1 );

            serialize_cbor_number( 4, n, out );

            for( std::size_t i = 0; i < n; ++i )
                serialize_cbor_value( ja[i], out );
        }
        break;

    case kind::object:
        {
            const object& jo = jv.get_object();
            std::size_t n = jo.size();

            out.reserve( out.size() + 3 * n + 1 );

            serialize_cbor_number( 5, n, out );

            for( const key_value_pair& kv: jo )
            {
                serialize_cbor_string( kv.key(), out );
                serialize_cbor_value( kv.value(), out );
            }
        }
        break;
    }
}

BOOST_NORETURN
void
throw_eof_error()
{
    throw std::runtime_error( "Unexpected end of input" );
}

BOOST_NORETURN
void
throw_format_error( char const * err )
{
    throw std::runtime_error( err );
}

void
ensure( std::size_t n, const unsigned char* first, const unsigned char* last )
{
    if( static_cast<std::size_t>(last - first) < n )
        throw_eof_error();
}

const unsigned char*
parse_cbor_value(
    const unsigned char* first, const unsigned char* last, value& v );

const unsigned char*
parse_cbor_number(
    const unsigned char* first, const unsigned char* last, unsigned char ch, std::uint64_t& n )
{
    unsigned char cv = ch & 31;

    if( cv < 24 )
    {
        n = cv;
    }
    else if( cv == 24 )
    {
        ensure( 1, first, last );
        n = *first++;
    }
    else if( cv == 25 )
    {
        ensure( 2, first, last );
        n = boost::endian::load_big_u16( first );
        first += 2;
    }
    else if( cv == 26 )
    {
        ensure( 4, first, last );
        n = boost::endian::load_big_u32( first );
        first += 4;
    }
    else if( cv == 27 )
    {
        ensure( 8, first, last );
        n = boost::endian::load_big_u64( first );
        first += 8;
    }
    else if( cv == 31 )
    {
        // infinite array/object
        throw_format_error( "Infinite sequences aren't supported" );
    }
    else
    {
        throw_format_error( "Invalid minor type" );
    }

    return first;
}

const unsigned char*
parse_cbor_string(
    const unsigned char* first, const unsigned char* last, unsigned char ch, value& v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    ensure( n, first, last );

    string_view sv( reinterpret_cast<char const*>( first ), n );
    first += n;

    v = sv;
    return first;
}

const unsigned char*
parse_cbor_array(
    const unsigned char* first, const unsigned char* last, unsigned char ch, value& v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    array & a = v.emplace_array();

    a.resize( n );

    std::size_t i = 0;

    for( ; i < n; ++i ) // double[] fast path
    {
        ensure( 1, first, last );
        unsigned char ch2 = *first;

        if( ch2 != 0xFB ) break;

        ++first;
        ensure( 8, first, last );

        double w = boost::endian::endian_load<double, 8, boost::endian::order::big>( first );
        first += 8;

        a[ i ] = w;
    }

    for( ; i < n; ++i ) // int[] fast path
    {
        ensure( 1, first, last );
        unsigned char ch2 = *first;

        if( ch2 >= 0x40 ) break;

        ++first;

        std::uint64_t m;
        first = parse_cbor_number( first, last, ch2, m );

        if( ch2 < 0x20 )
        {
            a[ i ] = m;
        }
        else
        {
            a[ i ] = static_cast<std::int64_t>( ~m );
        }
    }

    for( ; i < n; ++i )
        first = parse_cbor_value( first, last, a[ i ] );

    return first;
}

const unsigned char*
parse_cbor_object(
    const unsigned char* first, const unsigned char* last, unsigned char ch, value& v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    object & o = v.emplace_object();

    o.reserve( n );

    for( std::size_t i = 0; i < n; ++i )
    {
        // key string

        ensure( 1, first, last );
        unsigned char ch2 = *first++;

        if( ( ch2 >> 5 ) != 3 )
            throw_format_error( "Object keys must be strings" );

        std::uint64_t m;
        first = parse_cbor_number( first, last, ch2, m );

        ensure( m, first, last );

        string_view sv( reinterpret_cast<char const*>( first ), m );
        first += m;

        // value

        first = parse_cbor_value( first, last, o[ sv ] );
    }

    return first;
}

const unsigned char*
parse_cbor_unsigned(
    const unsigned char* first, const unsigned char* last, unsigned char ch, value& v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    v = n;
    return first;
}

const unsigned char*
parse_cbor_signed(
    const unsigned char* first, const unsigned char* last, unsigned char ch, value& v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    v = static_cast<std::int64_t>( ~n );
    return first;
}

const unsigned char*
parse_cbor_semantic_tag(
    const unsigned char* first, const unsigned char* last, unsigned char ch, value& v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    // ignore semantic tags

    return parse_cbor_value( first, last, v );
}

const unsigned char*
parse_cbor_type7(
    const unsigned char* first, const unsigned char* last, unsigned char ch, value& v )
{
    switch( ch & 31 )
    {
    case 20:
        v = false;
        return first;

    case 21:
        v = true;
        return first;

    case 22:
        v = nullptr;
        return first;

    case 26: // float
    {
        ensure( 4, first, last );

        float w = boost::endian::endian_load<float, 4, boost::endian::order::big>( first );
        first += 4;

        v = w;
        return first;
    }

    case 27: // double
    {
        ensure( 8, first, last );

        double w = boost::endian::endian_load<double, 8, boost::endian::order::big>( first );
        first += 8;

        v = w;
        return first;
    }

    default:
        throw_format_error( "Invalid minor type for major type 7" );
    }
}

const unsigned char*
parse_cbor_value( const unsigned char* first, const unsigned char* last, value& v )
{
    ensure( 1, first, last );
    const unsigned char ch = *first++;

    switch( ch >> 5 )
    {
    case 0:
        return parse_cbor_unsigned( first, last, ch, v );

    case 1:
        return parse_cbor_signed( first, last, ch, v );

    case 2:
        throw_format_error( "Binary strings aren't supported" );

    case 3:
        return parse_cbor_string( first, last, ch, v );

    case 4:
        return parse_cbor_array( first, last, ch, v );

    case 5:
        return parse_cbor_object( first, last, ch, v );

    case 6:
        return parse_cbor_semantic_tag( first, last, ch, v );

    case 7:
        return parse_cbor_type7( first, last, ch, v );

    default:
        BOOST_JSON_UNREACHABLE();
    }
}

int
main(int argc, const char** argv)
{
    if( argc != 2 )
    {
        std::cerr << "Usage: cbor FILE_NAME\n";
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[1]);
    is.exceptions(std::ios::badbit);

    const value jv = parse(is);

    std::vector<unsigned char> out;
    serialize_cbor_value( jv, out );

    value jv2;
    parse_cbor_value( out.data(), out.data() + out.size(), jv2 );

    if( jv != jv2 )
    {
        std::cerr << "Roundtrip check failed\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

//]
