// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <boost/json.hpp>
#include <boost/endian.hpp>
#include <stdexcept>
#include <vector>

static void serialize_cbor_number( unsigned char mt, std::uint64_t n, std::vector<unsigned char> & out )
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

static void serialize_cbor_string( boost::json::string_view sv, std::vector<unsigned char> & out )
{
    auto const n = sv.size();

    serialize_cbor_number( 3, n, out );

    out.insert( out.end(), sv.data(), sv.data() + n );
}

void serialize_cbor_value( boost::json::value const & v, std::vector<unsigned char> & out )
{
    switch( v.kind() )
    {
    case boost::json::kind::null:

        out.push_back( 224 + 22 );
        break;

    case boost::json::kind::bool_:

        out.push_back( 224 + 20 + v.get_bool() );
        break;

    case boost::json::kind::int64:

        {
            boost::int64_t w = v.get_int64();

            if( w >= 0 )
            {
                serialize_cbor_number( 0, w, out );
            }
            else
            {
                serialize_cbor_number( 1, ~w, out );
            }
        }
        
        break;

    case boost::json::kind::uint64:

        serialize_cbor_number( 0, v.get_uint64(), out );
        break;

    case boost::json::kind::double_:

        {
            unsigned char data[ 9 ];

            data[ 0 ] = 224 + 27;
            boost::endian::endian_store<double, 8, boost::endian::order::big>( data + 1, v.get_double() );

            out.insert( out.end(), std::begin( data ), std::end( data ) );
        }

        break;

    case boost::json::kind::string:

        serialize_cbor_string( v.get_string(), out );
        break;

    case boost::json::kind::array:

        {
            auto const & w = v.get_array();
            auto const n = w.size();

            out.reserve( out.size() + n + 1 );

            serialize_cbor_number( 4, n, out );

            for( std::size_t i = 0; i < n; ++i )
            {
                serialize_cbor_value( w[ i ], out );
            }
        }

        break;

    case boost::json::kind::object:

        {
            auto const & w = v.get_object();
            auto const n = w.size();

            out.reserve( out.size() + 3 * n + 1 );

            serialize_cbor_number( 5, n, out );

            for( auto const & kv: w )
            {
                serialize_cbor_string( kv.key(), out );
                serialize_cbor_value( kv.value(), out );
            }
        }

        break;
    }
}

[[noreturn]] static void throw_eof_error()
{
    throw std::runtime_error( "Unexpected end of input" );
}

[[noreturn]] static void throw_format_error( char const * err )
{
    throw std::runtime_error( err );
}

static void ensure( std::size_t n, unsigned char const * first, unsigned char const * last )
{
    if( static_cast<std::size_t>(last - first) < n ) throw_eof_error();
}

unsigned char const * parse_cbor_value( unsigned char const * first, unsigned char const * last, boost::json::value & v );

static unsigned char const * parse_cbor_number( unsigned char const * first, unsigned char const * last, unsigned char ch, std::uint64_t & n )
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

static unsigned char const * parse_cbor_string( unsigned char const * first, unsigned char const * last, unsigned char ch, boost::json::value & v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    ensure( n, first, last );

    boost::json::string_view sv( reinterpret_cast<char const*>( first ), n );
    first += n;

    v = sv;
    return first;
}

static unsigned char const * parse_cbor_array( unsigned char const * first, unsigned char const * last, unsigned char ch, boost::json::value & v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    boost::json::array & a = v.emplace_array();

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
    {
        first = parse_cbor_value( first, last, a[ i ] );
    }

    return first;
}

static unsigned char const * parse_cbor_object( unsigned char const * first, unsigned char const * last, unsigned char ch, boost::json::value & v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    boost::json::object & o = v.emplace_object();

    o.reserve( n );

    for( std::size_t i = 0; i < n; ++i )
    {
        // key string

        ensure( 1, first, last );
        unsigned char ch2 = *first++;

        if( ( ch2 >> 5 ) != 3 )
        {
            throw_format_error( "Object keys must be strings" );
        }

        std::uint64_t m;
        first = parse_cbor_number( first, last, ch2, m );

        ensure( m, first, last );

        boost::json::string_view sv( reinterpret_cast<char const*>( first ), m );
        first += m;

        // value

        // boost::json::value w( v.storage() );
        // first = parse_cbor_value( first, last, w );
        //
        // o.insert( boost::json::key_value_pair( sv, std::move( w ) ) );

        first = parse_cbor_value( first, last, o[ sv ] );
    }

    return first;
}

static unsigned char const * parse_cbor_unsigned( unsigned char const * first, unsigned char const * last, unsigned char ch, boost::json::value & v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    v = n;
    return first;
}

static unsigned char const * parse_cbor_signed( unsigned char const * first, unsigned char const * last, unsigned char ch, boost::json::value & v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    v = static_cast<std::int64_t>( ~n );
    return first;
}

static unsigned char const * parse_cbor_semantic_tag( unsigned char const * first, unsigned char const * last, unsigned char ch, boost::json::value & v )
{
    std::uint64_t n;
    first = parse_cbor_number( first, last, ch, n );

    // ignore semantic tags

    return parse_cbor_value( first, last, v );
}

static unsigned char const * parse_cbor_type7( unsigned char const * first, unsigned char const * last, unsigned char ch, boost::json::value & v )
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

unsigned char const * parse_cbor_value( unsigned char const * first, unsigned char const * last, boost::json::value & v )
{
    ensure( 1, first, last );
    unsigned char ch = *first++;

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
#if defined(_MSC_VER)
        __assume( 0 );
#else
        __builtin_unreachable();
#endif
    }
}

#include <fstream>
#include <chrono>
#include <cstdio>

void test( char const * fn )
{
    std::ifstream is( fn );
    std::string data( std::istreambuf_iterator<char>( is ), std::istreambuf_iterator<char>{} );

    auto t1 = std::chrono::high_resolution_clock::now();
    auto const v = boost::json::parse( data );
    auto t2 = std::chrono::high_resolution_clock::now();

    std::printf( "Parsing %s from JSON: %lld us\n", fn, std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count() );

    auto t3 = std::chrono::high_resolution_clock::now();
    std::vector<unsigned char> out;
    serialize_cbor_value( v, out );
    auto t4 = std::chrono::high_resolution_clock::now();

    std::printf( "Serializing %s to CBOR: %llu bytes, %lld us\n", fn, (unsigned long long)out.size(), std::chrono::duration_cast<std::chrono::microseconds>( t4 - t3 ).count() );

    auto t5 = std::chrono::high_resolution_clock::now();
    boost::json::value v2;
    parse_cbor_value( out.data(), out.data() + out.size(), v2 );
    auto t6 = std::chrono::high_resolution_clock::now();

    std::printf( "Parsing %s from CBOR: %lld us, %s roundtrip\n", fn, std::chrono::duration_cast<std::chrono::microseconds>( t6 - t5 ).count(), (v == v2? "successful": "UNSUCCESSFUL") );

    auto t7 = std::chrono::high_resolution_clock::now();
    auto js = boost::json::serialize( v );
    auto t8 = std::chrono::high_resolution_clock::now();

    std::printf( "Serializing %s to JSON: %llu bytes, %lld us\n\n", fn, (unsigned long long)js.size(), std::chrono::duration_cast<std::chrono::microseconds>( t8 - t7 ).count() );
}

int main()
{
    test( "canada.json" );
    test( "citm_catalog.json" );
    test( "gsoc-2018.json" );
    test( "marine_ik.json" );
    test( "mesh.pretty.json" );
    test( "twitter.json" );
}
