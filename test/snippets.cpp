//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#include <boost/json.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <unordered_map>
#include <vector>

#include "test_suite.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4101)
#elif defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused"
#elif defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused"
#endif

namespace boost {
namespace json {

//[snippet_conv_2

template< class T >
void
tag_invoke( const value_from_tag&, value& jv, std::complex< T > const& t)
{
    // Store a complex number as a 2-element array
    // with the real part followed by the imaginary part
    jv = { t.real(), t.imag() };
}

template< class T >
std::complex< T >
tag_invoke( const value_to_tag< std::complex< T > >&, value const& jv )
{
    return std::complex< T >(
        number_cast< T >( jv.as_array().at(0) ),
        number_cast< T >( jv.as_array().at(1) ) );
}

//]

namespace {

void
usingStrings()
{
    {
        //[snippet_strings_1

        string str1; // empty string, uses the default memory resource

        string str2( make_counted_resource<monotonic_resource>() ); // empty string, uses a counted monotonic resource

        //]
    }
    {
        //[snippet_strings_2

        std::string sstr1 = "helloworld";
        std::string sstr2 = "world";

        json::string jstr1 = "helloworld";
        json::string jstr2 = "world";

        assert( jstr2.insert(0, jstr1.subview(0, 5)) == "helloworld" );

        // this is equivalent to
        assert( sstr2.insert(0, sstr1, 0, 5) == "helloworld" );

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
usingValues()
{
    {
        //[snippet_value_1

        value jv1;
        value jv2( nullptr );

        assert( jv1.is_null() );
        assert( jv2.is_null() );

        //]
    }
    {
        //[snippet_value_2

        value jv( object_kind );

        assert( jv.kind() == kind::object );
        assert( jv.is_object() );
        assert( ! jv.is_number() );

        //]
    }
    {
        //[snippet_value_3

        value jv;
        jv = value( array_kind );

        assert( jv.is_array() );
        
        jv.emplace_string();
        
        assert( jv.is_string() );

        //]
    }
    {
        //[snippet_value_4

        value jv;
        jv.emplace_string() = "Hello, world!";

        int64_t& num = jv.emplace_int64();
        num = 1;

        assert( jv.is_int64() );

        //]
    }
    {
        try
        {
            //[snippet_value_5

            value jv( true );
            jv.as_bool() = true;

            jv.as_string() = "Hello, world!"; // throws an exception

            //]
        }
        catch(...) 
        {
        }
    }
    {
        //[snippet_value_6

        value jv( string_kind );
        if( string* str = jv.if_string() )
            *str = "Hello, world!";

        //]
    }
    {
        //[snippet_value_7

        value jv( string_kind );

        // The compiler's static analysis can see that
        // a null pointer is never dereferenced.
        *jv.if_string() = "Hello, world!";

        //]
    }
}

//----------------------------------------------------------

void
usingInitLists()
{
    {
        //[snippet_init_list_1

        value jv = {
            { "name", "John Doe" },
            { "active", true },
            { "associated-accounts", nullptr },
            { "total-balance", 330.00 },
            { "account-balances", { 84, 120, 126 } } };

        //]
    }

    {
        //[snippet_init_list_2

        value jv = { true, 2, "hello", nullptr };

        assert( jv.is_array() );

        assert( jv.as_array().size() == 4 );

        assert( to_string(jv) == "[true,2,\"hello\",null]" );

        //]
    }

    {
        //[snippet_init_list_3

        value jv = { true, 2, "hello", { "bye", nullptr, false } };

        assert( jv.is_array() );

        assert( jv.as_array().back().is_array() );

        assert( to_string(jv) == "[true,2,\"hello\",[\"bye\",null,false]]" );

        //]
    }

    {
        //[snippet_init_list_4

        // Should this be an array or an object?
        value jv = { { "hello", 42 }, { "world", 43 } }; 

        //]
    }

    {
        //[snippet_init_list_5

        value jv1 = { { "hello", 42 }, { "world", 43 } };

        assert( jv1.is_object() );

        assert( jv1.as_object().size() == 2 );

        assert( to_string(jv1) == R"({"hello":42,"world":43})" );

        // All of the following are arrays

        value jv2 = { { "make", "Tesla" }, { "model", 3 }, "black" };

        value jv3 = { { "library", "JSON" }, { "Boost", "C++", "Fast", "JSON" } };

        value jv4 = { { "color", "blue" }, { 1, "red" } };

        assert( jv2.is_array() && jv3.is_array() && jv4.is_array() );
        
        //]
    }

    {
        //[snippet_init_list_6

        value jv = { { "hello", 42 }, array{ "world", 43 } };

        assert( jv.is_array() );

        array& ja = jv.as_array();

        assert( ja[0].is_array() && ja[1].is_array());

        assert ( to_string(jv) == R"([["hello",42],["world",43]])" );

        //]

        (void)ja;
    }

    {
        //[snippet_init_list_7

        value jv = { { "mercury", 36 }, { "venus", 67 }, { "earth", 93 } };

        assert( jv.is_object() );

        assert( to_string(jv) == "{\"mercury\":36,\"venus\":67,\"earth\":93}" );

        array ja = { { "mercury", 36 }, { "venus", 67 }, { "earth", 93 } };

        assert( to_string(ja) == "[[\"mercury\",36],[\"venus\",67],[\"earth\",93]]" );

        //]

        (void)ja;
    }
    
    {
        //[snippet_init_list_8
        
        object jo = { { "mercury", { { "distance", 36 } } }, { "venus", { 67, "million miles" } }, { "earth", 93 } };

        assert( jo["mercury"].is_object() );

        assert( jo["venus"].is_array() );

        //]
    }

    {
        //[snippet_init_list_9
        
        object jo1 = { { "john", 100 }, { "dave", 500 }, { "joe", 300 } };
        
        value jv = { { "clients", std::move(jo1) } };

        object& jo2 = jv.as_object()["clients"].as_object();
        
        assert( ! jo2.empty() && jo1.empty() );

        assert( to_string(jv) == R"({"clients":{"john":100,"dave":500,"joe":300}})" );

        //]

        (void)jo2;
    }
}

//----------------------------------------------------------

void
usingArrays()
{
    {
        //[snippet_arrays_1

        array arr1; // empty array, uses the default memory resource

        array arr2( make_counted_resource<monotonic_resource>() ); // empty array, uses a counted monotonic resource

        //]
    }
    {
        //[snippet_arrays_2

        array arr( { "Hello", 42, true } );

        //]
    }
    try
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
    catch (...) 
    {
    }
}

//----------------------------------------------------------

void
usingObjects()
{
    {
        //[snippet_objects_1

        object obj1; // empty object, uses the default memory resource

        object obj2( make_counted_resource<monotonic_resource>() ); // empty object, uses a counted monotonic resource

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
    try
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
    catch (...)
    {
    }
}

//----------------------------------------------------------

void
usingStorage()
{
    {
        //[snippet_allocators_1

        value jv;                                   // uses the default memory resource
        storage_ptr sp;                             // refers to the default memory resource
        object obj( sp );                           // uses the default memory resource

        assert( jv.storage().get() == sp.get() );   // same memory resource
        assert( *jv.storage() == *sp );             // memory allocated by jv.storage() can be deallocated by sp

        assert( jv.storage().get() ==
                    obj.storage().get() );          // same memory resource

        //]
    }
    {
        //[snippet_allocators_2

        constexpr std::size_t size = 256;
        unsigned char buffer[size]{};

        monotonic_resource mr(buffer, size);

        // allocates 256 bytes with an alignment of 1
        // within the initial buffer
        void* buffer_alloc = mr.allocate(size, 1);

        // allocated memory is within the initial buffer
        assert( buffer_alloc >= std::begin(buffer) &&
            buffer_alloc <= std::end(buffer) );

        // allocates 256 bytes with an alignment of
        // alignof(std::max_align_t) within a dynamically allocated block
        void* dynamic_alloc = mr.allocate(size);

        // allocated memory was dynamically allocated
        assert( !(dynamic_alloc >= std::begin(buffer) &&
            dynamic_alloc <= std::end(buffer)) );

        //]

        (void)dynamic_alloc;
        (void)buffer_alloc;
    }
    {
        //[snippet_allocators_4
        
        monotonic_resource mr;

        storage_ptr sp = &mr;

        value jv( sp ); // all allocations for jv1 and its child elements will be done through mr

        assert( jv.storage().get() == &mr );

        //]
    }
    {
        //[snippet_allocators_5
        
        monotonic_resource mr;

        storage_ptr sp1 = &mr; // reference semantics

        assert ( ! sp1.is_counted() );

        storage_ptr sp2 = make_counted_resource<monotonic_resource>(); // shared ownership
        
        assert( sp2.is_counted() );

        //]
    }
    {
        //[snippet_allocators_6

        storage_ptr sp1; // default memory resource

        assert ( ! sp1.is_counted() );

        monotonic_resource mr;

        storage_ptr sp2 = &mr;

        assert( sp1 != sp2 );

        sp2 = storage_ptr(); // assigns the default memory resource

        assert( sp1 == sp2 );

        //]
    }
    {
        //[snippet_allocators_7

        storage_ptr sp = make_counted_resource<monotonic_resource>(); // counted resource

        assert ( sp.is_counted() );

        //]
    }
    {
        //[snippet_allocators_8

        storage_ptr sp1; // non-counted, refers to the default memory resource

        assert ( ! sp1.is_counted() );

        memory_resource* res = nullptr;

        {
            storage_ptr sp2 = make_counted_resource<monotonic_resource>(); // shared ownership

            assert( sp2.is_counted() );

            res = sp2.get();

            sp1 = sp2;
        } 
        
        assert( sp1.is_counted() && sp1.get() == res ); // even though sp2 was destroyed, the memory resource was not

        //]

        (void)res;
    }
    {
        //[snippet_allocators_10
        
        storage_ptr sp; // refers to the default memory resource

        assert ( ! sp.is_counted() );

        {
            monotonic_resource mr;

            sp = &mr; // non-counted reference to mr

            assert( ! sp.is_counted() );
        }

        // mr has been destroyed, sp refers to a non-existant memory resource

        //]
    }
    {
        //[snippet_allocators_12
        
        monotonic_resource mr;
        
        storage_ptr sp = &mr;

        // mr.do_allocate(32, 4); error: cannot access protected member of monotonic_resource

        auto ptr = sp->allocate(32); // calls mr.do_allocate(32, alignof(std::max_align_t))

        ptr = sp->allocate(32, 4); // calls mr.do_allocate(32, 4)

        // mr.do_deallocate(ptr, 32, 4); error: cannot access protected member of monotonic_resource

        sp->deallocate(ptr, 32, 4); // calls mr.do_deallocate(ptr, 32, 4)

        // mr.do_is_equal(*sp); error: cannot access protected member of monotonic_resource

        sp->is_equal(*sp); // calls mr.do_is_equal(*sp)

        //]
    }

    {
        //[snippet_allocators_13

        storage_ptr sp1; // refers to the default memory resource

        storage_ptr sp2; // refers to the default memory resource

        assert( sp1->is_equal(*sp2) ); // the default memory resource is shared across all instances of storage_ptr in the same program

        assert ( sp1 == sp2 ); // identical semantics

        //]
    }
}

//[snippet_allocators_3

value parse_fast( string_view s )
{
    return parse( s, make_counted_resource<monotonic_resource>() );
}

//]

void do_json(value const&) { }

//[snippet_allocators_11

void do_rpc( string_view cmd )
{
    monotonic_resource mr;

    value const jv = parse( cmd, &mr );

    do_json( jv );
}

//]

//[snippet_allocators_15

class logging_resource : public memory_resource
{
private:
    void* do_allocate(std::size_t bytes, std::size_t align) override
    {
        std::cout << "Allocating " << bytes << " bytes with alignment " << align << '\n';
        return ::operator new(bytes); // forward request to the global allocation function
    }
    
    void do_deallocate(void* ptr, std::size_t bytes, std::size_t align) override
    {
        std::cout << "Deallocating " << bytes << " bytes with alignment " << align << " @ address " << ptr << '\n';
        return ::operator delete(ptr); // forward request to the global deallocation function
    }

    bool do_is_equal(const memory_resource& other) const noexcept override
    {
        // since the global allocation and deallocation functions are used,
        // any instance of a logging_resource can deallocate memory allocated
        // by another instance of a logging_resource
        return dynamic_cast<const logging_resource*>(&other) != nullptr; 
    }
};

//]

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

        value jv = parse( "[1,2,3,4,5]", make_counted_resource<monotonic_resource>() );

        //]
    }
    {
        //[snippet_parsing_4

        parser p;
        error_code ec;

        // This must be called once before parsing every new JSON.
        p.reset();

        // Write the entire character buffer, indicating
        // to the parser that there is no more data.
        p.write( "[1,2,3,4,5]", 11, ec );
        
        if( ! ec )
            p.finish( ec );

        // Take ownership of the resulting value.
        value jv = p.release( ec );

        // At this point the parser may be re-used by calling p.reset() again.

        //]
    }
    {
        //[snippet_parsing_5

        parser p;
        error_code ec;

        // This must be called once before parsing every new JSON
        p.reset();

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
            value jv = p.release( ec );

        assert(! ec );

        // At this point the parser may be re-used by calling p.reset() again.

        //]
    }
    {
        //[snippet_parsing_6

        {
            parser p;
            error_code ec;

            monotonic_resource mr;

            // Use the monotonic resource for the parsed value
            p.reset( &mr );

            // Write the entire JSON
            p.write( "[1,2,3,4,5]", 11, ec );
            if( ! ec )
                p.finish( ec );

            // The value will use the monotonic resource created above
            value jv = p.release( ec );
        }

        //]
    }
    {
        //[snippet_parsing_7

        storage_ptr sp = 
            make_counted_resource<monotonic_resource>();

        parser p( sp );
        error_code ec;

        p.reserve( 1024 );

        // Fully parses a valid JSON string and
        // extracts the resulting value
        p.reset( sp );
        p.write( "[true, false, 1, 0]", 19, ec );
        p.finish( ec );

        assert( ! ec );

        value jv = p.release( ec );

        assert( ! ec );

        // The intermediate storage that was used
        // for the last value will be reused here.
        
        p.reset( sp );

        p.write( "[null]", 6, ec );
        p.finish( ec );

        assert( ! ec );

        jv = p.release( ec );

        assert( ! ec );

        //]
    }
    {
        //[snippet_parsing_8

        string_view good = "{\"valid\":\"json\"}";
        string_view bad = "{\"invalid\":\"json\",}";
            
        // A default constructed parser will not
        // accept non-standard JSON
        parser p;
        error_code ec;

        p.reset();
        p.write( good.data(), good.size(), ec );
            
        // Valid JSON
        assert( ! ec );

        ec.clear();

        p.reset();
        p.write( bad.data(), bad.size(), ec );

        // Error, trailing commas are not permitted
        // by the JSON specification
        assert( ec );

        //]
    }
    {
        //[snippet_parsing_9

        string_view comment = "/* example comment */[1, 2, 3]";
        
        // Comments are not permitted by default
        parser p1;

        parse_options po;
        po.allow_comments = true;

        // Constructs a parser that will treat
        // comments as whitespace.
        parser p2(po);

        error_code ec;

        p1.reset();
        p1.write( comment.data(), comment.size(), ec );
            
        // Error, invalid JSON
        assert( ec );
        
        ec.clear();

        p2.reset();
        p2.write( comment.data(), comment.size(), ec );

        // Ok, comments are permitted
        assert( ! ec );

        //]
    }
}

//----------------------------------------------------------

void
usingSerializing()
{
    (void)([]()
    {
        //[snippet_serializing_1

        value jv = { 1, 2, 3, 4, 5 };

        std::cout << jv << "\n";

        //]
    });
    {
        //[snippet_serializing_2

        value jv = { 1, 2, 3, 4, 5 };

        string s = to_string( jv );

        //]
    }
}

//----------------------------------------------------------

//[snippet_conv_3

template< class T >
struct vec3
{
    T x, y, z;
};

template< class T >
void tag_invoke( const value_from_tag&, value& jv, const vec3<T>& vec )
{
    jv = { 
        { "x", vec.x }, 
        { "y", vec.y }, 
        { "z", vec.z } 
    };
}

//]

#ifdef BOOST_JSON_DOCS
//[snippet_conv_5

template< class T, typename std::enable_if<
    std::is_floating_point< T >::value>::type* = nullptr >
void tag_invoke( const value_from_tag&, value& jv, T t )
{
    jv = std::llround( t );
}

//]
#endif

//[snippet_conv_8

struct customer
{
    std::uint64_t id;
    std::string name;
    bool late;

    customer() = default;

    customer( std::uint64_t i, const std::string& n, bool l )
        : id( i ), name( n ), late( l ) { }

    explicit customer( value const& );
};

void tag_invoke( const value_from_tag&, value& jv, customer const& c )
{
    // Assign a JSON value
    jv = {
        { "id", c.id },
        { "name", c.name },
        { "late", c.late }
    };
}

//]

//[snippet_conv_12

customer::customer( value const& jv )

    // at() throws if `jv` is not an object, or if the key is not found.
    // as_uint64() will throw if the value is not an unsigned 64-bit integer.

    : id( jv.at( "id" ).as_uint64() )

    // We already know that jv is an object from
    // the previous call to jv.as_object() suceeding,
    // now we use jv.get_object() which skips the
    // check. value_to will throw if jv.kind() != kind::string

    , name( value_to< std::string >( jv.get_object().at( "name" ) ) )
{
    // id and name are constructed from JSON in the member
    // initializer list above, but we can also use regular
    // assignments in the body of the function as shown below.
    // as_bool() will throw if kv.kind() != kind::bool
    
    late = jv.get_object().at( "late" ).as_bool();
}

//]

void
usingExchange()
{
    {
        //[snippet_conv_1

        std::vector< int > v1{ 1, 2, 3, 4 };

        // Convert the vector to a JSON array
        value jv = value_from( v1 );

        assert( jv.is_array() );

        array& ja = jv.as_array();

        assert( ja.size() == 4 );

        for ( std::size_t i = 0; i < v1.size(); ++i )
            assert( v1[i] == ja[i].as_int64() );

        // Convert back to vector< int >
        std::vector< int > v2 = value_to< std::vector< int > >( jv );

        assert( v1 == v2 );

        //]

        (void)ja;
    }
    {
        //[snippet_conv_4

        vec3< int > pos = { 4, 1, 4 };

        value jv = value_from( pos );

        assert( to_string( jv ) == "{\"x\":4,\"y\":1,\"z\":4}" );

        //]
    }
    {
        //[snippet_conv_6

       value jv = value_from( 1.5 ); // error

        //]
    }
    {
        //[snippet_conv_7

        std::map< std::string, vec3< int > > positions = {
            { "Alex", { 42, -60, 18 } },
            { "Blake", { 300, -60, -240} },
            { "Carol", { -60, 30, 30 } }
        };

        // conversions are applied recursively;
        // the key type and value type will be converted
        // using value_from as well
        value jv = value_from( positions );

        assert( jv.is_object() );
        
        object& jo = jv.as_object();

        assert( jo.size() == 3 );

        // The sum of the coordinates is 0
        assert( std::accumulate( jo.begin(), jo.end(), std::int64_t(0),
            []( std::int64_t total, const key_value_pair& jp )
            {
                assert ( jp.value().is_object() );
                
                const object& pos = jp.value().as_object();

                return total + pos.at( "x" ).as_int64() +
                    pos.at( "y" ).as_int64() +
                    pos.at( "z" ).as_int64();

            } ) == 0 );
   
        //]

        (void)jo;
    }
    {
        //[snippet_conv_9

        std::vector< customer > customers = {
            customer( 0, "Alison", false ),
            customer( 1, "Bill", false ),
            customer( 3, "Catherine", true ),
            customer( 4, "Doug", false )
         };

        storage_ptr sp = make_counted_resource< monotonic_resource >();

        value jv = value_from( customers, sp );

        assert( jv.storage() == sp );

        assert( jv.is_array() );

        //]
    }

    {
        //[snippet_conv_10

        // Satisfies both FromMapLike and FromContainerLike
        std::unordered_map< std::string, bool > available_tools = {
            { "Crowbar", true },
            { "Hammer", true },
            { "Drill", true },
            { "Saw", false },
        };

        value jv = value_from( available_tools );

        assert( jv.is_object() );
        
        //]
    }
    {
        //[snippet_conv_11

        std::complex< double > c1 = { 3.14159, 2.71828 };

        // Convert a complex number to JSON
        value jv = value_from( c1 );

        assert ( jv.is_array() );

        // Convert back to a complex number

        std::complex< double > c2 = value_to< std::complex< double > >( jv );

        //]

        (void)c2;
    }
    {
        //[snippet_conv_13

        customer c1( 5, "Ed", false );

        // Convert customer to value
        value jv = value_from( c1 );

        // Convert the result back to customer
        customer c2 = value_to< customer >( jv );

        // The resulting customer is unchanged
        assert( c1.name == c2.name );

        //]
    }
    {
        //[snippet_conv_14

        value available_tools = {
            { "Crowbar", true },
            { "Hammer", true },
            { "Drill", true },
            { "Saw", false }
        };

        assert( available_tools.is_object() );

        auto as_map = value_to< std::map< std::string, bool > >( available_tools );

        assert( available_tools.as_object().size() == as_map.size() );

        //]
    }
}

BOOST_STATIC_ASSERT(
    has_value_from<customer>::value);

BOOST_STATIC_ASSERT(
    has_value_from<std::complex<float>>::value);
BOOST_STATIC_ASSERT(
    has_value_from<std::complex<double>>::value);
BOOST_STATIC_ASSERT(
    has_value_from<std::complex<long double>>::value);

BOOST_STATIC_ASSERT(
    has_value_to<std::complex<float>>::value);
BOOST_STATIC_ASSERT(
    has_value_to<std::complex<double>>::value);
BOOST_STATIC_ASSERT(
    has_value_to<std::complex<long double>>::value);

} // (anon)

} // json
} // boost

//----------------------------------------------------------


namespace {

class my_null_deallocation_resource { };

} // (anon)

//[snippet_allocators_14
namespace boost {
namespace json {

template<>
struct is_deallocate_null<my_null_deallocation_resource>
{
  static constexpr bool deallocate_is_null() noexcept
  {
    return true;
  }
};

} // json
} // boost

//]

namespace boost {
namespace json {

class snippets_test
{
public:
    void
    run()
    {
        usingValues();
        usingInitLists();
        usingStorage();
        usingExchange();
        usingArrays();
        usingObjects();
        usingStrings();
        usingSerializing();
        usingParsing();
        do_rpc("null");
        (void)parse_fast("null");

        BOOST_TEST_PASS();
    }
};

TEST_SUITE(snippets_test, "boost.json.snippets");

} // json
} // boost
