//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#include <boost/json.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <iostream>

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

    auto jv = parse( cmd, sp );

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

BOOST_JSON_STATIC_ASSERT(
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
        p.finish( "[1,2,3,4,5]", 11 );

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
            p.finish( "3,4,5]", 6, ec );

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
            p.finish( "[1,2,3,4,5]", 11, ec );

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

} // (anon)

class snippets_test : public beast::unit_test::suite
{
public:
    void
    run() override
    {
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,snippets);

} // json
} // boost
