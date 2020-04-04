//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#include <boost/json.hpp>

#include <complex>
#include <iostream>

#include "test_suite.hpp"

namespace boost {
namespace json {

namespace {

void
usingStrings()
{
    {
        //[snippet_strings_1

        string str1; // empty string, default storage

        string str2( make_storage<pool>() ); // empty string, pool storage

        //]
    }
    {
        //[snippet_strings_2

        std::string sstr1 = "helloworld";
        std::string sstr2 = "world";

        json::string jstr1 = "helloworld";
        json::string jstr2 = "world";


        assert(jstr2.insert(0, jstr1.subview(0, 6)) == "helloworld");

        // this is equivalent to

        assert(sstr1.insert(0, sstr2, 0, 6) == "helloworld");

        //]
    }
    {
        //[snippet_strings_3

        std::string sstr = "hello";

        json::string jstr = "hello";

        assert(sstr.append({'w', 'o', 'r', 'l', 'd'}) == "helloworld");

        // such syntax is inefficient, and the same can
        // be achieved with a character array.

        assert(jstr.append("world") == "helloworld");

        //]
    }

    {
        //[snippet_strings_4

        json::string str = "Boost.JSON";
        json::string_view sv = str;

        // all of these call compare(string_view)
        str.compare(sv);

        str.compare(sv.substr(0, 5));

        str.compare(str);

        str.compare("Boost");

        //]
    }
}

//----------------------------------------------------------

void
usingArrays()
{
    {
        //[snippet_arrays_1

        array arr1; // empty array, default storage

        array arr2( make_storage<pool>() ); // empty array, pool storage

        //]
    }
    {
        //[snippet_arrays_2

        array arr( { "Hello", 42, true } );

        //]
    }
    {
        //[snippet_arrays_3

        array arr;

        arr.emplace_back( "Hello" );
        arr.emplace_back( 42 );
        arr.emplace_back( true );

        //]

        //[snippet_arrays_4

        assert( arr[0].as_string() == "Hello" );

        // The following line throws std::out_of_range, since the index is out of range
        arr.at( 3 ) = nullptr;

        //]
    }
}

//----------------------------------------------------------

void
usingObjects()
{
    {
        //[snippet_objects_1

        object obj1; // empty object, default storage

        object obj2( make_storage<pool>() ); // empty object, pool storage

        //]
    }
    {
        //[snippet_objects_2

        object obj( {{"key1", "value1" }, { "key2", 42 }, { "key3", false }} );

        //]
    }
    {
        //[snippet_objects_3

        object obj;

        obj.emplace( "key1", "value1" );
        obj.emplace( "key2", 42 );
        obj.emplace( "key3", false );

        //]
    }
    {
        //[snippet_objects_4

        object obj;

        obj["key1"] = "value1";
        obj["key2"] = 42;
        obj["key3"] = false;

        // The following line throws std::out_of_range, since the key does not exist
        obj.at( "key4" );

        //]
    }
}

//----------------------------------------------------------

void
usingStorage()
{
    {
        //[snippet_storage_1

        value jv;                                   // uses the default storage
        storage_ptr sp;                             // uses the default storage
        object obj( sp );                           // uses the default storage

        assert( jv.storage().get() == sp.get() );   // same pointer
        assert( *jv.storage() == *sp );             // deep equality

        assert( jv.storage().get() ==
                    obj.storage().get() );          // same pointer

        //]
    }
}

//[snippet_storage_2

value parse_fast( string_view s )
{
    return parse( s, make_storage<pool>() );
}

//]

void do_json(value const&) {}

//[snippet_storage_3

void do_rpc( string_view cmd )
{
    scoped_storage<pool> sp;

    value const jv = parse( cmd, sp );

    do_json( jv );
}

//]

//[snippet_storage_4

struct Storage
{
    static constexpr std::uint64_t id = 0;
    static constexpr bool need_free = true;

    void* allocate( std::size_t bytes, std::size_t align );
    void deallocate( void* p, std::size_t bytes, std::size_t align );
};

//]

BOOST_STATIC_ASSERT(
    is_storage<Storage>::value);

//----------------------------------------------------------

void
usingParsing()
{
    {
        //[snippet_parsing_1

        value jv = parse( "[1,2,3,4,5]" );

        //]
    }
    {
        //[snippet_parsing_2

        error_code ec;
        value jv = parse( "[1,2,3,4,5]", ec );
        if( ec )
            std::cout << "Parsing failed: " << ec.message() << "\n";

        //]
    }
    {
        //[snippet_parsing_3

        value jv = parse( "[1,2,3,4,5]", make_storage<pool>() );

        //]
    }
    {
        //[snippet_parsing_4

        parser p;

        // This must be called once before parsing every new JSON.
        p.start();

        // Write the entire character buffer, indicating
        // to the parser that there is no more data.
        p.write( "[1,2,3,4,5]", 11 );
        p.finish();

        // Take ownership of the resulting value.
        value jv = p.release();

        // At this point the parser may be re-used by calling p.start() again.

        //]
    }
    {
        //[snippet_parsing_5

        parser p;
        error_code ec;

        // This must be called once before parsing every new JSON
        p.start();

        // Write the first part of the buffer
        p.write( "[1,2,", 5, ec);

        // Write the remaining part of the character buffer,
        // indicating to the parser that there is no more data.
        if(! ec )
            p.write( "3,4,5]", 6, ec );
        if(! ec)
            p.finish( ec );

        // Take ownership of the resulting value.
        if(! ec)
            value jv = p.release();

        // At this point the parser may be re-used by calling p.start() again.

        //]
    }
    {
        //[snippet_parsing_6

        {
            parser p;
            error_code ec;

            // Declare a new, scoped instance of the block storage
            scoped_storage< pool > sp;

            // Use the scoped instance for the parsed value
            p.start( sp );

            // Write the entire JSON
            p.write( "[1,2,3,4,5]", 11, ec );
            if( ! ec )
                p.finish( ec );

            // The value will use the instance of block storage created above
            value jv = p.release();
        }

        //]
    }
}

//----------------------------------------------------------

void
usingSerializing()
{
    {
        //[snippet_serializing_1

        value jv = { 1, 2, 3 ,4 ,5 };

        std::cout << jv << "\n";

        //]
    }
    {
        //[snippet_serializing_2

        value jv = { 1, 2, 3 ,4 ,5 };

        string s = to_string( jv );

        //]
    }
}

//----------------------------------------------------------

//[snippet_exchange_1

struct customer
{
    std::uint64_t id;
    std::string name;
    bool delinquent;

    customer() = default;
    
    explicit customer( value const& );

    void to_json( value& jv ) const;
};

//]

BOOST_STATIC_ASSERT(
    has_to_value<customer>::value);

//BOOST_STATIC_ASSERT(
//    has_from_json<customer>::value);

//[snippet_exchange_2

void customer::to_json( value& jv ) const
{
    // Assign a JSON value
    jv = {
        { "id", id },
        { "name", name },
        { "delinquent", delinquent }
    };
}

//]

//[snippet_exchange_3

customer::customer( value const& jv )

    // at() throws if `jv` is not an object, or if the key is not found.
    //
    // as_uint64() will throw if the value is not an unsigned 64-bit integer.

    : id( jv.at( "id" ).as_uint64() )

    // We already know that jv is an object from
    // the previous call to jv.as_object() suceeding,
    // now we use jv.get_object() which skips the
    // check.
    //
    // value_cast will throw if jv.kind() != kind::string

    , name( value_cast< std::string >( jv.get_object().at( "name" ) ) )
{
    // id and name are constructed from JSON in the member
    // initializer list above, but we can also use regular
    // assignments in the body of the function as shown below.
    //
    // as_bool() will throw if kv.kind() != kind::bool
    
    this->delinquent = jv.get_object().at( "delinquent" ).as_bool();
}

//]

void
usingExchange1()
{
    //[snippet_exchange_4

    customer cust;
    cust.id = 1;
    cust.name = "John Doe";
    cust.delinquent = false;

    // Convert customer to value
    value jv = to_value( cust );

    // Store value in customer
    customer cust2;
    cust2 = value_cast< customer >( jv );

    //]
}

} // (anon)

} // json
} // boost
//[snippet_exchange_5

// Specializations of to_value_traits and value_cast_traits
// must be declared in the boost::json namespace.

namespace boost {
namespace json {

template<class T>
struct to_value_traits< std::complex< T > >
{
    static void assign( value& jv, std::complex< T > const& t );
};

template<class T>
struct value_cast_traits< std::complex< T > >
{
    static std::complex< T > construct( value const& jv );
};

} // namespace json
} // namespace boost

//]
namespace boost {
namespace json {

BOOST_STATIC_ASSERT(
    has_to_value<std::complex<float>>::value);
BOOST_STATIC_ASSERT(
    has_to_value<std::complex<double>>::value);
BOOST_STATIC_ASSERT(
    has_to_value<std::complex<long double>>::value);

BOOST_STATIC_ASSERT(
    has_value_cast<std::complex<float>>::value);
BOOST_STATIC_ASSERT(
    has_value_cast<std::complex<double>>::value);
BOOST_STATIC_ASSERT(
    has_value_cast<std::complex<long double>>::value);

//[snippet_exchange_6

template< class T >
void
to_value_traits< std::complex< T > >::
assign( boost::json::value& jv, std::complex< T > const& t )
{
    // Store a complex number as a 2-element array
    array& a = jv.emplace_array();

    // Real part first
    a.emplace_back( t.real() );

    // Imaginary part last
    a.emplace_back( t.imag() );
}

//]

//[snippet_exchange_7]

template< class T >
std::complex< T >
value_cast_traits< std::complex< T > >::
construct( value const& jv )
{
    // as_array() throws if jv.kind() != kind::array.

    array const& a = jv.as_array();

    // We store the complex number as a two element
    // array with the real part first, and the imaginary
    // part last.
    //
    // value_cast() throws if the JSON value does
    // not contain an applicable kind for the type T.

    if(a.size() != 2)
        throw std::invalid_argument(
            "invalid json for std::complex");
    return {
        value_cast< T >( a[0] ),
        value_cast< T >( a[1] ) };
}

//]

namespace {

void
usingExchange2()
{
    {
    //[snippet_exchange_8

    std::complex< double > c = { 3.14159, 2.71828 };

    // Convert std::complex< double > to value
    value jv = to_value(c);

    // Store value in std::complex< double >
    std::complex< double > c2;

    c2 = value_cast< std::complex< double > >( jv );

    //]
    }

    {
    //[snippet_exchange_9

    // Use float instead of double.

    std::complex< float > c = { -42.f, 1.41421f };

    value jv = to_value(c);

    std::complex< float > c2;

    c2 = value_cast< std::complex< float > >( jv );

    //]
    }
}

} // (anon)

//----------------------------------------------------------

class snippets_test
{
public:
    void
    run()
    {
        (void)&usingStrings;
        (void)&usingArrays;
        (void)&usingObjects;
        (void)&usingStorage;
        (void)&parse_fast;
        (void)&do_json;
        (void)&do_rpc;
        (void)&usingParsing;
        (void)&usingSerializing;
        (void)&usingExchange1;
        (void)&usingExchange2;
        BOOST_TEST_PASS();
    }
};

TEST_SUITE(snippets_test, "boost.json.snippets");

} // json
} // boost
