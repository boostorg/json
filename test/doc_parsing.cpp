//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/null_resource.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/parser.hpp>
#include <boost/json/static_resource.hpp>
#include <boost/json/stream_parser.hpp>

#include <iostream>
#include <string>

#include "test_suite.hpp"

namespace boost {
namespace json {

//----------------------------------------------------------

static void set1() {

//----------------------------------------------------------
{
// tag::doc_parsing_1[]
value jv = parse( "[1,2,3,4,5]" );
// end::doc_parsing_1[]
}
//----------------------------------------------------------
{
// tag::doc_parsing_2[]
boost::system::error_code ec;
value jv = parse( "[1,2,3,4,5]", ec );
if( ec )
    std::cout << "Parsing failed: " << ec.message() << "\n";
// end::doc_parsing_2[]
}
//----------------------------------------------------------
{
// tag::doc_parsing_3[]
try
{
    boost::system::error_code ec;
    value jv = parse( "[1,2,3,4,5]", ec );
    if( ec )
        std::cout << "Parsing failed: " << ec.message() << "\n";
}
catch( std::bad_alloc const& e)
{
    std::cout << "Parsing failed: " << e.what() << "\n";
}
// end::doc_parsing_3[]
}
//----------------------------------------------------------
{
// tag::doc_parsing_4[]
    monotonic_resource mr;
    value const jv = parse( "[1,2,3,4,5]", &mr );
// end::doc_parsing_4[]
}
//----------------------------------------------------------
{
// tag::doc_parsing_5[]
parse_options opt;                // all extensions default to off
opt.allow_comments = true;        // permit C and C++ style comments
                                  // to appear in whitespace
opt.allow_trailing_commas = true; // allow an additional trailing comma in
                                  // object and array element lists
opt.allow_invalid_utf8 = true;    // skip utf-8 validation of keys and strings
opt.allow_invalid_utf16 = true;   // replace invalid surrogate pair UTF-16 code point(s)
                                  // with the Unicode replacement character

value jv = parse( "[1,2,3,] // comment ", storage_ptr(), opt );
// end::doc_parsing_5[]
}
//----------------------------------------------------------
{
#if __cpp_designated_initializers >= 201707L
{
// tag::doc_parsing_6[]
value jv = parse(
    "[1,2,3,] // comment ",
    storage_ptr(),
    {
        .allow_comments = true,             // permit C and C++ style comments
                                            // to appear in whitespace
        .allow_trailing_commas = true,      // allow a trailing comma in object and array lists
        .allow_invalid_utf8 = true          // skip utf-8 validation of keys and strings
    });
// end::doc_parsing_6[]
}
{
// tag::doc_parsing_15[]
value jv = parse( "{\"command\":\"\\uDF3E\\uDEC2\"}", storage_ptr(),
    {
        .allow_invalid_utf16 = true       // replace illegal leading surrogate pair with ��
    });
// end::doc_parsing_15[]
}
#endif
}
//----------------------------------------------------------

} // set1

//----------------------------------------------------------
// tag::doc_parsing_7[]
class connection
{
    parser p_;                    // persistent data member

public:
    void do_read( string_view s ) // called for each complete message from the network
    {
        p_.reset();               // start parsing a new JSON using the default resource
        p_.write( s );            // parse the buffer, using exceptions to indicate error
        do_rpc( p_.release() );   // process the command
    }

    void do_rpc( value jv );
};
// end::doc_parsing_7[]

//----------------------------------------------------------

static void set2() {

//----------------------------------------------------------
{
// tag::doc_parsing_8[]
stream_parser p;
boost::system::error_code ec;
string_view s = "[1,2,3] %HOME%";
std::size_t n = p.write_some( s, ec );
assert( ! ec && p.done() && n == 8 );
s = s.substr( n );
value jv = p.release();
assert( s == "%HOME%" );
// end::doc_parsing_8[]
}
//----------------------------------------------------------
{
// tag::doc_parsing_9[]
parse_options opt;                     // All extensions default to off
opt.allow_comments = true;             // Permit C and C++ style comments to appear in whitespace
opt.allow_trailing_commas = true;      // Allow an additional trailing comma in
                                       // object and array element lists
opt.allow_invalid_utf8 = true;         // Skip utf-8 validation of keys and strings
stream_parser p( storage_ptr(), opt ); // The stream_parser will use the options
// end::doc_parsing_9[]
}
//----------------------------------------------------------

} // set2

//----------------------------------------------------------
// tag::doc_parsing_10[]
value read_json( std::istream& is, boost::system::error_code& ec )
{
    stream_parser p;
    std::string line;
    while( std::getline( is, line ) )
    {
        p.write( line, ec );
        if( ec )
            return nullptr;
    }
    p.finish( ec );
    if( ec )
        return nullptr;
    return p.release();
}
// end::doc_parsing_10[]

// tag::doc_parsing_14[]
std::vector<value> read_jsons( std::istream& is, boost::system::error_code& ec )
{
    std::vector< value > jvs;
    stream_parser p;
    std::string line;
    std::size_t n = 0;
    while( true )
    {
        if( n == line.size() )
        {
            if( !std::getline( is, line ) )
                break;
            n = 0;
        }

        n += p.write_some( line.data() + n, line.size() - n, ec );

        if( p.done() )
        {
            jvs.push_back( p.release() );
            p.reset();
        }
    }
    if( !p.done() )   // this part handles the cases when the last JSON text in
    {                 // the input is either incomplete or doesn't have a marker
        p.finish(ec); // for end of the value (e.g. it is a number)
        if( ec.failed() )
            return jvs;
        jvs.push_back( p.release() );
    }

    return jvs;
}
// end::doc_parsing_14[]

//----------------------------------------------------------

static void set3() {

//----------------------------------------------------------
{
// tag::doc_parsing_11[]
    monotonic_resource mr;

    stream_parser p;
    p.reset( &mr );                // Use mr for the resulting value
    p.write( "[1,2,3,4,5]" );      // Parse the input JSON
    value const jv = p.release();  // Retrieve the result
    assert( *jv.storage() == mr ); // Same memory resource
// end::doc_parsing_11[]
}
//----------------------------------------------------------
{
// tag::doc_parsing_12[]
unsigned char temp[ 4096 ]; // Declare our buffer
stream_parser p(
    storage_ptr(),          // Default memory resource
    parse_options{},        // Default parse options (strict parsing)
    temp);                  // Use our buffer for temporary storage
// end::doc_parsing_12[]
}
//----------------------------------------------------------

} // set3

//----------------------------------------------------------

// tag::doc_parsing_13[]
/*  Parse JSON and invoke the handler

    This function parses the JSON specified in `s`
    and invokes the handler, whose signature must
    be equivalent to:

        void( value const& jv );

    The operation is guaranteed not to perform any
    dynamic memory allocations. However, some
    implementation-defined upper limits on the size
    of the input JSON and the size of the resulting
    value are imposed.

    Upon error, an exception is thrown.
*/
template< class Handler >
void do_rpc( string_view s, Handler&& handler )
{
    unsigned char temp[ 4096 ]; // The parser will use this storage for its temporary needs
    parser p(                   // Construct a strict parser using
                                // the temp buffer and no dynamic memory
        get_null_resource(),    // The null resource never dynamically allocates memory
        parse_options(),        // Default constructed parse options allow only standard JSON
        temp );

    unsigned char buf[ 16384 ]; // Now we need a buffer to hold the actual JSON values
    static_resource mr2( buf ); // The static resource is monotonic,
                                // using only a caller-provided buffer
    p.reset( &mr2 );            // Use the static resource for producing the value
    p.write( s );               // Parse the entire string we received from the network client

    // Retrieve the value and invoke the handler with it.
    // The value will use `buf` for storage. The handler
    // must not take ownership, since monotonic resources
    // are inefficient with mutation.
    handler( p.release() );
}
// end::doc_parsing_13[]

//----------------------------------------------------------

void
testPrecise()
{
    // tag::doc_parsing_precise[]
    parse_options opt;
    opt.numbers = number_precision::precise;
    value jv = parse( "1002.9111801605201", storage_ptr(), opt );
    // end::doc_parsing_precise[]
    (void)jv;
    assert( jv == 1002.9111801605201 );
}

//----------------------------------------------------------

class doc_parsing_test
{
public:
    void
    run()
    {
        (void)&set1;
        (void)&set2;
        (void)&set3;
        {
            std::stringstream ss( "[1,2,3\n"
                                  ",4]nul\n"
                                  "l12345\n"
                                  "6\"!\n"
                                  "\"[2]3" );
            system::error_code ec;
            auto jvs = read_jsons( ss, ec );
            assert( !ec.failed() );
            assert( jvs.size() == 6 );
            assert(( jvs[0] == array{ 1, 2, 3, 4} ));
            assert(( jvs[1] == value() ));
            assert(( jvs[2] == 123456 ));
            assert(( jvs[3] == "!" ));
            assert(( jvs[4] == array{2} ));
            assert(( jvs[5] == 3 ));
        }

        testPrecise();
    }
};

TEST_SUITE(doc_parsing_test, "boost.json.doc_parsing");

} // namespace json
} // namespace boost
