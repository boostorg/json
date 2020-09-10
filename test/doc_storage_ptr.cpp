//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#include <boost/json.hpp>

#include <iostream>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

//----------------------------------------------------------

static void set1() {

//----------------------------------------------------------
{
//[doc_storage_ptr_1
storage_ptr sp1;
storage_ptr sp2;

assert( sp1.get() != nullptr );                         // always points to a valid resource
assert( sp1.get() == sp2.get() );                       // both point to the default resource
assert( *sp1.get() == *sp2.get() );                     // the default resource compares equal
//]
}
//----------------------------------------------------------
{
//[doc_storage_ptr_2
value jv;
array arr;
object obj;

assert( jv.storage().get() == storage_ptr().get() );    // uses the default memory resource
assert( jv.storage().get() == arr.storage().get() );    // both point to the default resource
assert( *arr.storage() == *obj.storage() );             // containers use equivalent resources
//]
}
//----------------------------------------------------------
{
//[doc_storage_ptr_3
monotonic_resource mr;

value const jv = parse( "[1,2,3]", &mr );
//]
}
//----------------------------------------------------------

} // set1

//----------------------------------------------------------

//[doc_storage_ptr_4
value parse_value( string_view s)
{
    return parse( s, make_counted_resource< monotonic_resource >() );
}
//]

//----------------------------------------------------------

//[doc_storage_ptr_5
template< class Handler >
void do_rpc( string_view s, Handler&& h )
{
    unsigned char buffer[ 8192 ];                       // Small stack buffer to avoid most allocations during parse
    monotonic_resource mr( buffer );                    // This resource will use our local buffer first
    value const jv = parse( s, &mr );                   // Parse the input string into a value that uses our resource
    h( jv );                                            // Call the handler to perform the RPC command
}
//]

//----------------------------------------------------------

void set2() {

//----------------------------------------------------------
{
//[doc_storage_ptr_6
unsigned char buffer[ 8192 ];
static_resource mr( buffer );                           // The resource will use our local buffer
//]
}
//----------------------------------------------------------
{
//[doc_storage_ptr_7
monotonic_resource mr;
array arr( &mr );                                       // construct an array using our resource
arr.emplace_back( "boost" );                            // insert a string
assert( *arr[0].as_string().storage() == mr );          // the resource is propagated to the string
//]
}
//----------------------------------------------------------
{
//[doc_storage_ptr_8
{
    monotonic_resource mr;

    array arr( &mr );                                   // construct an array using our resource

    assert( ! arr.storage().is_counted() );             // no shared ownership
}
//]
}
//----------------------------------------------------------
{
//[doc_storage_ptr_9
storage_ptr sp = make_counted_resource< monotonic_resource >();

string str( sp );

assert( sp.is_counted() );                              // shared ownership
assert( str.storage().is_counted() );                   // shared ownership
//]
}
//----------------------------------------------------------

} // set2

//----------------------------------------------------------
//[doc_storage_ptr_10
class logging_resource : public memory_resource
{
private:
    void* do_allocate( std::size_t bytes, std::size_t align ) override
    {
        std::cout << "Allocating " << bytes << " bytes with alignment " << align << '\n';

        return ::operator new( bytes );
    }
    
    void do_deallocate( void* ptr, std::size_t bytes, std::size_t align ) override
    {
        std::cout << "Deallocating " << bytes << " bytes with alignment " << align << " @ address " << ptr << '\n';

        return ::operator delete( ptr );
    }

    bool do_is_equal( memory_resource const& other ) const noexcept override
    {
        // since the global allocation and deallocation functions are used,
        // any instance of a logging_resource can deallocate memory allocated
        // by another instance of a logging_resource

        return dynamic_cast< logging_resource const* >( &other ) != nullptr; 
    }
};
//]

//----------------------------------------------------------

#if 0
void do_rpc( string_view s )
{
    // The parser will use this storage for its temporary needs
    unsigned char temp[ 4000 ];

    // The null resource guarantees we will never dynamically allocate
    null_resource mr1;

    // Construct a strict parser using the temp buffer and no dynamic memory
    parser p( &mr1, parse_options(), temp );

    // Now we need a buffer to hold the actual JSON values
    unsigned char buf[ 6000 ];

    // The static resource is monotonic, using only a caller-provided buffer
    static_resource mr2( buf );

    // We need to catch any exceptions thrown by the two memory resources
    try
    {
        // This error code indicates errors not related to memory exhaustion
        error_code ec;

        // Parse the entire string we received from the network client
        p.write( s, ec );

        // Inform the parser that the complete input has been provided
        if(! ec )
            p.finish( ec );

        if(! ec )
        {
            // Retrieve the value. It will use `buf` for storage.
            value jv = p.release();

            // At this point we can inspect jv and perform the requested RPC.
        }
        else
        {
            // An error occurred. A real program would report the error
            // message back to the network client, indicating that the
            // received JSON was invalid.
        }

    }
    catch(std::bad_alloc const&)
    {
        // The memory needed to parse this JSON exceeded our statically
        // define upper limits. A real program would send an error message
        // back to the network client informing that their JSON is too large.
    }
}
#endif

//----------------------------------------------------------

class doc_storage_ptr_test
{
public:
    void
    run()
    {
        (void)&set1;
        BOOST_TEST_PASS();
    }
};

TEST_SUITE(doc_storage_ptr_test, "boost.json.doc_storage_ptr");

BOOST_JSON_NS_END
