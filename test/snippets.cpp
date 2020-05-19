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
        (void)jv;
    }

    {
        //[snippet_init_list_2

        value jv = { true, 2, "hello", nullptr };

        assert( jv.is_array() );

        assert( jv.as_array().size() == 4 );

        assert( to_string(jv) == "[true,2,\"hello\",null]" );

        //]
        (void)jv;
    }

    {
        //[snippet_init_list_3

        value jv = { true, 2, "hello", { "bye", nullptr, false } };

        assert( jv.is_array() );

        assert( jv.as_array().back().is_array() );

        assert( to_string(jv) == "[true,2,\"hello\",[\"bye\",null,false]]" );

        //]
        (void)jv;
    }

    {
        //[snippet_init_list_4

        // Should this be an array or an object?
        value jv = { { "hello", 42 }, { "world", 43 } }; 

        //]
        (void)jv;
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
        (void)jv1;
        (void)jv2;
        (void)jv3;
        (void)jv4;
    }

    {
        //[snippet_init_list_6

        value jv = { { "hello", 42 }, array{ "world", 43 } };

        assert( jv.is_array() );

        array& ja = jv.as_array();

        assert( ja[0].is_array() && ja[1].is_array());

        assert ( to_string(jv) == R"([["hello",42],["world",43]])" );

        //]
        (void)jv;
    }

    {
        //[snippet_init_list_7

        value jv1 = { { "mercury", 36 }, { "venus", 67 }, { "earth", 93 } };

        assert( jv1.is_object() );

        assert( to_string(jv1) == "{\"mercury\":36,\"venus\":67,\"earth\":93}" );

        array ja = { { "mercury", 36 }, { "venus", 67 }, { "earth", 93 } };
        
        for (value& jv2 : ja)
          assert( jv2.is_array() );

        assert( to_string(ja) == "[[\"mercury\",36],[\"venus\",67],[\"earth\",93]]" );

        //]
        (void)jv1;
        (void)ja;
    }
    
    {
        //[snippet_init_list_8
        
        object jo = { { "mercury", { { "distance", 36 } } }, { "venus", { 67, "million miles" } }, { "earth", 93 } };

        assert( jo["mercury"].is_object() );

        assert( jo["venus"].is_array() );

        //]
        (void)jo;
    }

    {
        //[snippet_init_list_9
        
        object jo1 = { { "john", 100 }, { "dave", 500 }, { "joe", 300 } };
        
        value jv = { { "clients", std::move(jo1) } };

        object& jo2 = jv.as_object()["clients"].as_object();
        
        assert( ! jo2.empty() && jo1.empty() );

        assert( to_string(jv) == R"({"clients":{"john":100,"dave":500,"joe":300}})" );

        //]
        (void)jo1;
        (void)jv;
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
        void* dynamic_allocd = mr.allocate(size);

        // allocated memory was dynamically allocated
        assert( buffer_alloc < std::begin(buffer) &&
            buffer_alloc > std::end(buffer) );

        //]
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

            monotonic_resource mr;

            // Use the monotonic resource for the parsed value
            p.start( &mr );

            // Write the entire JSON
            p.write( "[1,2,3,4,5]", 11, ec );
            if( ! ec )
                p.finish( ec );

            // The value will use the monotonic resource created above
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
        usingInitLists();
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
