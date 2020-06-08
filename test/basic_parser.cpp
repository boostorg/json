//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/basic_parser.hpp>

#include <memory>
#include <string>
#include <utility>

#include "parse-vectors.hpp"
#include "test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace json {

namespace base64 {

std::size_t constexpr
decoded_size(std::size_t n)
{
    return n / 4 * 3; // requires n&3==0, smaller
}

signed char const*
get_inverse()
{
    static signed char constexpr tab[] = {
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //   0-15
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  16-31
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, //  32-47
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, //  48-63
         -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, //  64-79
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, //  80-95
         -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, //  96-111
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 112-127
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128-143
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 144-159
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160-175
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 176-191
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 192-207
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 208-223
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 224-239
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 240-255
    };
    return &tab[0];
}

std::pair<std::size_t, std::size_t>
decode(void* dest, char const* src, std::size_t len)
{
    char* out = static_cast<char*>(dest);
    auto in = reinterpret_cast<unsigned char const*>(src);
    unsigned char c3[3], c4[4];
    int i = 0;
    int j = 0;

    auto const inverse = base64::get_inverse();

    while(len-- && *in != '=')
    {
        auto const v = inverse[*in];
        if(v == -1)
            break;
        ++in;
        c4[i] = v;
        if(++i == 4)
        {
            c3[0] =  (c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
            c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
            c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

            for(i = 0; i < 3; i++)
                *out++ = c3[i];
            i = 0;
        }
    }

    if(i)
    {
        c3[0] = ( c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
        c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
        c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

        for(j = 0; j < i - 1; j++)
            *out++ = c3[j];
    }

    return {out - static_cast<char*>(dest),
        in - reinterpret_cast<unsigned char const*>(src)};
}

} // base64

namespace {

bool
validate( string_view s )
{
    // The null parser discards all the data

    class null_parser : public basic_parser
    {
        friend class boost::json::basic_parser;

    public:
        null_parser() {}
        ~null_parser() {}
        bool on_document_begin( error_code& ) { return true; }
        bool on_document_end( error_code& ) { return true; }
        bool on_object_begin( error_code& ) { return true; }
        bool on_object_end( std::size_t, error_code& ) { return true; }
        bool on_array_begin( error_code& ) { return true; }
        bool on_array_end( std::size_t, error_code& ) { return true; }
        bool on_key_part( string_view, error_code& ) { return true; }
        bool on_key( string_view, error_code& ) { return true; }
        bool on_string_part( string_view, error_code& ) { return true; }
        bool on_string( string_view, error_code& ) { return true; }
        bool on_int64( std::int64_t, error_code& ) { return true; }
        bool on_uint64( std::uint64_t, error_code& ) { return true; }
        bool on_double( double, error_code& ) { return true; }
        bool on_bool( bool, error_code& ) { return true; }
        bool on_null( error_code& ) { return true; }
        
        std::size_t
        write(
            char const* data,
            std::size_t size,
            error_code& ec)
        {
            auto const n =
                basic_parser::write_some(
                *this, false, data, size, ec);
            if(! ec && n < size)
                ec = error::extra_data;
            return n;
        }
    };

    // Parse with the null parser and return false on error
    null_parser p;
    error_code ec;
    p.write( s.data(), s.size(), ec );
    if( ec )
        return false;

    // The string is valid JSON.
    return true;
}

} // (anon)

class basic_parser_test
{
public:
    ::test_suite::log_type log;

    void
    grind(string_view s, bool good)
    {
        {
            error_code ec;
            fail_parser p;
            p.write(false,
                s.data(), s.size(), ec);
            BOOST_TEST((good && !ec) || (
                ! good && ec));
        }

        // split/errors matrix
        if(! s.empty())
        {
            for(std::size_t i = 1;
                i < s.size(); ++i)
            {
                for(std::size_t j = 1;;++j)
                {
                    error_code ec;
                    fail_parser p(j);
                    p.write(true, s.data(), i, ec);
                    if(ec == error::test_failure)
                        continue;
                    if(! ec)
                    {
                        p.write(false, s.data() + i,
                            s.size() - i, ec);
                        if(ec == error::test_failure)
                            continue;
                    }
                    BOOST_TEST((good && !ec) || (
                        ! good && ec));
                    break;
                }
            }
        }

        // split/exceptions matrix
        if(! s.empty())
        {
            for(std::size_t i = 1;
                i < s.size(); ++i)
            {
                for(std::size_t j = 1;;++j)
                {
                    error_code ec;
                    throw_parser p(j);
                    try
                    {
                        p.write(
                            true, s.data(), i, ec);
                        if(! ec)
                            p.write(
                                false, s.data() + i,
                                    s.size() - i, ec);
                        BOOST_TEST((good && !ec) || (
                            ! good && ec));
                        break;
                    }
                    catch(test_exception const&)
                    {
                        continue;
                    }
                    catch(std::exception const& e)
                    {
                        BOOST_TEST_FAIL();
                        log << "  " <<
                            e.what() << std::endl;
                    }
                }
            }
        }
    }

    void
    bad(string_view s)
    {
        grind(s, false);
    }

    void
    good(string_view s)
    {
        grind(s, true);
    }

    void
    bad_one(string_view s)
    {
        error_code ec;
        fail_parser p;
        p.write(false,
            s.data(), s.size(), ec);
        BOOST_TEST(ec);
    }

    void
    good_one(string_view s)
    {
        error_code ec;
        fail_parser p;
        p.write(false,
            s.data(), s.size(), ec);
        BOOST_TEST(! ec);
    }

    //------------------------------------------------------

    void
    testNull()
    {
        good("null");
        good(" null");
        good("null ");
        good("\tnull");
        good("null\t");
        good("\r\n\t null\r\n\t ");

        bad ("n     ");
        bad ("nu    ");
        bad ("nul   ");
        bad ("n---  ");
        bad ("nu--  ");
        bad ("nul-  ");

        bad ("NULL");
        bad ("Null");
        bad ("nulls");
    }

    void
    testBoolean()
    {
        good("true");
        good(" true");
        good("true ");
        good("\ttrue");
        good("true\t");
        good("\r\n\t true\r\n\t ");

        bad ("t     ");
        bad ("tr    ");
        bad ("tru   ");
        bad ("t---  ");
        bad ("tr--  ");
        bad ("tru-  ");
        bad ("TRUE");
        bad ("True");
        bad ("truer");

        good("false");
        good(" false");
        good("false ");
        good("\tfalse");
        good("false\t");
        good("\r\n\t false\r\n\t ");

        bad ("f     ");
        bad ("fa    ");
        bad ("fal   ");
        bad ("fals  ");
        bad ("f---- ");
        bad ("fa--- ");
        bad ("fal-- ");
        bad ("fals- ");
        bad ("FALSE");
        bad ("False");
        bad ("falser");
    }

    void
    testString()
    {
        good(R"jv( "x"   )jv");
        good(R"jv( "xy"  )jv");
        good(R"jv( "x y" )jv");

        // escapes
        good(R"jv(" \" ")jv");
        good(R"jv(" \\ ")jv");
        good(R"jv(" \/ ")jv");
        good(R"jv(" \b ")jv");
        good(R"jv(" \f ")jv");
        good(R"jv(" \n ")jv");
        good(R"jv(" \r ")jv");
        good(R"jv(" \t ")jv");

        // utf-16 escapes
        good(R"jv( " \u0000 "       )jv");
        good(R"jv( " \ud7ff "       )jv");
        good(R"jv( " \ue000 "       )jv");
        good(R"jv( " \uffff "       )jv");
        good(R"jv( " \ud800\udc00 " )jv");
        good(R"jv( " \udbff\udfff " )jv");
        good(R"jv( " \n\u0000     " )jv");

        // escape in key
        good(R"jv( {" \n":null} )jv");

        // incomplete
        bad ("\"");
        
        // illegal control character
        bad ({ "\"" "\x00" "\"", 3 });
        bad ("\"" "\x1f" "\"");
        bad ("\"" "\\n" "\x1f" "\"");

        // incomplete escape
        bad (R"jv( "\" )jv");

        // utf-16 escape, fast path,
        // invalid surrogate
        bad (R"jv( " \u----       " )jv");
        bad (R"jv( " \ud---       " )jv");
        bad (R"jv( " \ud8--       " )jv");
        bad (R"jv( " \ud80-       " )jv");
        // invalid low surrogate
        bad (R"jv( " \ud800------ " )jv");
        bad (R"jv( " \ud800\----- " )jv");
        bad (R"jv( " \ud800\u---- " )jv");
        bad (R"jv( " \ud800\ud--- " )jv");
        bad (R"jv( " \ud800\udc-- " )jv");
        bad (R"jv( " \ud800\udc0- " )jv");
        // illegal leading surrogate
        bad (R"jv( " \udc00       " )jv");
        bad (R"jv( " \udfff       " )jv");
        // illegal trailing surrogate
        bad (R"jv( " \ud800\udbff " )jv");
        bad (R"jv( " \ud800\ue000 " )jv");
    }

    void
    testNumber()
    {
        good("0");
        good("0e0");
        good("0E0");
        good("0e00");
        good("0E01");
        good("0e+0");
        good("0e-0");
        good("0.0");
        good("0.01");
        good("0.0e0");
        good("0.01e+0");
        good("0.02E-0");
        good("1");
        good("12");
        good("1");
        good("1e0");
        good("1E0");
        good("1e00");
        good("1E01");
        good("1e+0");
        good("1e-0");
        good("1.0");
        good("1.01");
        good("1.0e0");
        good("1.01e+0");
        good("1.02E-0");

        good("-0");
        good("-0e0");
        good("-0E0");
        good("-0e00");
        good("-0E01");
        good("-0e+0");
        good("-0e-0");
        good("-0.0");
        good("-0.01");
        good("-0.0e0");
        good("-0.01e+0");
        good("-0.02E-0");
        good("-1");
        good("-12");
        good("-1");
        good("-1e0");
        good("-1E0");
        good("-1e00");
        good("-1E01");
        good("-1e+0");
        good("-1e-0");
        good("-1.0");
        good("-1.01");
        good("-1.0e0");
        good("-1.01e+0");
        good("-1.02E-0");

        good("1.1e309");
        good(   "9223372036854775807");
        good(  "-9223372036854775807");
        good(  "18446744073709551615");
        good( "-18446744073709551615");

        good("0.900719925474099178");

        // non-significant digits
        good("1000000000000000000000000");
        good("1000000000000000000000000e1");
        good("1000000000000000000000000.0");
        good("1000000000000000000000000.00");
        good("1000000000000000000000000.000000000001");
        good("1000000000000000000000000.0e1");
        good("1000000000000000000000000.0 ");

        bad ( "");
        bad ("-");
        bad ( "00");
        bad ( "01");
        bad ( "00.");
        bad ( "00.0");
        bad ("-00");
        bad ("-01");
        bad ("-00.");
        bad ("-00.0");
        bad ( "1a");
        bad ( ".");
        bad ( "1.");
        bad ( "1+");
        bad ( "0.0+");
        bad ( "0.0e+");
        bad ( "0.0e-");
        bad ( "0.0e0-");
        bad ( "0.0e");
        bad ( "1000000000000000000000000.e");
    }

    void
    testArray()
    {
        good("[]");
        good("[ ]");
        good("[ \t ]");
        good("[ \"\" ]");
        good("[ \" \" ]");
        good("[ \"x\" ]");
        good("[ \"x\", \"y\" ]");
        good("[1,2,3]");
        good(" [1,2,3]");
        good("[1,2,3] ");
        good(" [1,2,3] ");
        good("[1,2,3]");
        good("[ 1,2,3]");
        good("[1 ,2,3]");
        good("[1, 2,3]");
        good("[1,2 ,3]");
        good("[1,2, 3]");
        good("[1,2,3 ]");
        good(" [  1 , 2 \t\n ,  \n3]");

        bad ("[");
        bad (" [");
        bad (" []]");
        bad ("[{]");
        bad ("[ \"x\", ]");

        bad (R"jv( [ null ; 1 ] )jv");
    }

    void
    testObject()
    {
        good("{}");
        good("{ }");
        good("{ \t }");
        good("{\"x\":null}");
        good("{ \"x\":null}");
        good("{\"x\" :null}");
        good("{\"x\": null}");
        good("{\"x\":null }");
        good("{ \"x\" : null }");
        good("{ \"x\" : {} }");
        good("{ \"x\" : [] }");
        good("{ \"x\" : { \"y\" : null } }");
        good("{ \"x\" : [{}] }");
        good("{ \"x\":1, \"y\":null}");
        good("{\"x\":1,\"y\":2,\"z\":3}");
        good(" {\"x\":1,\"y\":2,\"z\":3}");
        good("{\"x\":1,\"y\":2,\"z\":3} ");
        good(" {\"x\":1,\"y\":2,\"z\":3} ");
        good("{ \"x\":1,\"y\":2,\"z\":3}");
        good("{\"x\" :1,\"y\":2,\"z\":3}");
        good("{\"x\":1 ,\"y\":2,\"z\":3}");
        good("{\"x\":1,\"y\" :2,\"z\":3}");
        good("{\"x\":1,\"y\": 2,\"z\":3}");
        good("{\"x\":1,\"y\":2 ,\"z\":3}");
        good("{\"x\":1,\"y\":2, \"z\":3}");
        good("{\"x\":1,\"y\":2, \"z\" :3}");
        good("{\"x\":1,\"y\":2, \"z\": 3}");
        good("{\"x\":1,\"y\":2, \"z\":3 }");
        good(" \t { \"x\" \n  :   1, \"y\" :2, \"z\" : 3} \n");

        good("[{\"x\":[{\"y\":null}]}]");

        bad ("{");
        bad (" {");
        bad (" {}}");
        bad ("{{}}");
        bad ("{[]}");

        bad (R"jv( {"x";null} )jv");
        bad (R"jv( {"x":null . "y":0} )jv");
    }

    void
    testParser()
    {
        auto const check =
        [this]( string_view s,
            bool is_complete)
        {
            fail_parser p;
            error_code ec;
            p.write_some(
                true,
                s.data(), s.size(),
                ec);
            if(! BOOST_TEST(! ec))
            {
                log << "    failed to parse: " << s;
                return;
            }
            BOOST_TEST(is_complete ==
                p.is_complete());
        };

        // is_complete()

        check("{}", true);
        check("{} ", true);
        check("{}x", true);
        check("{} x", true);

        check("[]", true);
        check("[] ", true);
        check("[]x", true);
        check("[] x", true);

        check("\"a\"", true);
        check("\"a\" ", true);
        check("\"a\"x", true);
        check("\"a\" x", true);

        check("0", false);
        check("0 ", true);
        check("0x", true);
        check("0 x", true);
        check("0.", false);
        check("0.0", false);
        check("0.0 ", true);
        check("0.0 x", true);

        check("true", true);
        check("true ", true);
        check("truex", true);
        check("true x", true);

        check("false", true);
        check("false ", true);
        check("falsex", true);
        check("false x", true);

        check("null", true);
        check("null ", true);
        check("nullx", true);
        check("null x", true);

        // flush
        {
            {
                for(auto esc :
                    { "\\\"", "\\\\", "\\/", "\\b",
                      "\\f", "\\n", "\\r", "\\t", "\\u0000"
                    })
                {
                    std::string const big =
                        "\\\"" + std::string(
                        BOOST_JSON_PARSER_BUFFER_SIZE-4, '*') + esc;
                    std::string const s =
                        "{\"" + big + "\":\"" + big + "\"}";
                    good_one(s);
                }
            }
            {
                std::string big;
                big = "\\\"" +
                    std::string(
                        BOOST_JSON_PARSER_BUFFER_SIZE+ 1, '*');
                std::string s;
                s = "{\"" + big + "\":\"" + big + "\"}";
                good_one(s);
            }
        }

        // no input
        {
           // error_code ec;
           // fail_parser p;
           // p.write(false, nullptr, 0, ec);
           // BOOST_TEST(ec);
        }
    }

    void
    testMembers()
    {
        {
            fail_parser p;
            std::size_t n;
            error_code ec;
            n = p.write_some(true, "null", 4, ec );
            if(BOOST_TEST(! ec))
            {
                BOOST_TEST(n == 4);
                BOOST_TEST(p.is_complete());
                n = p.write_some(false, " \t42", 4, ec);
                BOOST_TEST(n == 2);
                BOOST_TEST(! ec);
            }
        }
    }

    void
    testParseVectors()
    {
        parse_vectors pv;
        for(auto const& v : pv)
        {
            // skip these , because basic_parser
            // doesn't have a max_depth setting.
            if( v.name == "structure_100000_opening_arrays" ||
                v.name == "structure_open_array_object")
            {
                continue;
            }
            if(v.result == 'i')
            {
                error_code ec;
                fail_parser p;
                p.write(
                    false,
                    v.text.data(),
                    v.text.size(),
                    ec);
                if(! ec)
                    good_one(v.text);
                else
                    bad_one(v.text);
                continue;
            }
            if(v.result == 'y')
                good_one(v.text);
            else
                bad_one(v.text);
        }
    }

    // https://github.com/vinniefalco/json/issues/13
    void
    testIssue13()
    {
        validate("\"~QQ36644632   {n");
    }

    void
    testIssue20()
    {
        string_view s =
            "WyL//34zOVx1ZDg0ZFx1ZGM4M2RcdWQ4M2RcdWRlM2M4dWRlMTlcdWQ4M2RcdWRlMzlkZWUzOVx1"
            "ZDg0ZFx1ZGM4M2RcdWQ4M2RcdWRlMzlcXHVkY2M4M1x1ZDg5ZFx1ZGUzOVx1ZDgzZFx1ZGUzOWRb"
            "IGZhbHNlLDMzMzMzMzMzMzMzMzMzMzMzNDMzMzMzMTY1MzczNzMwLDMzMzMzMzMzMzMzMzMzMzMz"
            "MzM3ODAsMzMzMzMzMzMzMzM0MzMzMzMxNjUzNzM3MzAsMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMz"
            "MzM3ODAsMzMzMzMzMzMzMzMzMzQzMzMzMzE2NTM3MzczMCwzMzMzMzMzMzMzMzMzMzMzMzMzNzgw"
            "LDMzMzMzMzM4MzU1MzMwNzQ3NDYwLDMzMTY2NTAwMDAzMzMzMzMwNzQ3MzMzMzMzMzc3OSwzMzMz"
            "MzMzMzMzMzMzMzMzNDMzMzMzMzMwNzQ3NDYwLDMzMzMzMzMzMzMzMzMzMzMzMzMzNzgwLDMzMzMz"
            "MzMzMzMzMzMzMzA4ODM1NTMzMDc0Mzc4MCwzMzMzMzMzMzMzMzMzMzMwODgzNTUzMzA3NDc0NjAs"
            "MzMzMzMzMzMxNjY1MDAwMDMzMzMzNDc0NjAsMzMzMzMzMzMzMzMzMzMzMzMzMzc4MCwzMzMzMzMz"
            "MzMzMzM3MzMzMzE2NjUwMDAwMzMzMzMzMDc0NzMzMzMzMzM3NzksMzMzMzMzMzMzMzMzMzMzMzQz"
            "MzMzMzMwNzQ3NDYwLDMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzc4MCwzMzMzMzMzMzMzNzgw"
            "LDMzMzMzMzMzMzMzMzA4ODM1NTMzMDc0NzQ2MCwzMzE2NjUwMDAwMzMzMzMzMDc0NzMzMzMzMzM3"
            "NzksMzMzMzMzMzMzMzMzMzMzMzQzMzMzMzMwNzQ3NDYwLDMzMzMzMzMzMzMzMzMzMzMzMzMzNzgw"
            "LDMzMzMzMzMzMzMzMzMzMzA4ODM1NTMzMDc0Mzc4MCwzMzMzMzMzMzMzMzMzMzMzMzMwODgzNTUz"
            "MzA3NDM3ODAsMzMzMzMzMzMzMzMzMzMzMDg4MzU1MzMwNzQ3NDYwLDMzMzMzMzMzMzMzMDczMzM3"
            "NDc0NjAsMzMzMzMzMzMzMzMzMzMzMzMzNzgwLDMzMzMzMzMzMzMzMzA4ODM1NTMzMDc0NzQ2MCwz"
            "MzE2NjUwMDAwMzMzMzMzMDc0NzMzMzMzMzM3NzksMzMzMzMzMzMzMzMzMzMzMzQzMzMzMzMzMDc0"
            "NzQ2MCwzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzMzM3ODAsMzMzMzMzMzMzMzMzMzMzMDg4"
            "MzU1MzMwNzQzNzgwLDMzMzMzMzMzMzMzMzMzMzA4ODM1NTMzMDc0NzQ2MCwzMzMzMzMzMzMzMzMz"
            "MzMzMzM0MjQ3LDMzMzMzMzMzMzMzMzMzMzQzMzMzMzMzMzMzMzMzMzM3MzMzMzQzMzMzMzMzMDc0"
            "NzQ2MCwzMzMzMzMzMzMzMzMzMzMzMzMzNzgwLDMzMzMzMzMzMzMzMzA4ODM1NTMzMDc0NzQ2MCwz"
            "MzE2NjUwMDAwMzMzMzMzMDc0NzMzMzMzMzM3NzksMzMzMzMzMzMzMzMzMzMzMzQzMzMzMzMwNzQ3"
            "NDYwLDMzMzMzMzMzMzMzMzMzMzMzMzMzNzgwLDMzMzMzMzMzMzMzMzMzMzA4ODM1NTMzMDc0Mzc4"
            "MCwzMzMzMzMzMzMzMzMzMzMwODgzNTUzMzA3NDc0NjAsMzMzMzMzMzMzLDMzMzMzMzMzMzMzMzMz"
            "MzMzMzM3ODAsMzMzMzMzMzMzMzc4MCwzMzMzMzMzMzMzMzMwODgzNTUzMzA3NDc0NjAsMzMxNjY1"
            "MDAwMDMzMzMzMzA3NDczMzMzMzMzNzc5LDMzMzMzMzMzMzM3ODAsMzMzMzMzMzgzNTUzMzA3NDc0"
            "NjAsMzMxNjY1MDAwMDMzMzMzMzA3NDczMzMzMzMzNzc5LDMzMzMzMzMzMzMzMzMzMzM0MzMzMzMz"
            "MzA3NDc0NjAsMzMzMzMzMzMzMzMzMzMzMzMzMzM3ODAsMzMzMzMzMzMzMzMzMzMzMDg4MzU1MzMw"
            "NzQzNzgwLDMzMzMzMzMzMzMzMzMzMzA4ODM1NTMzMDc0NzQ2MCwzMzMzMzMzMzE2NjUwMDAwMzMz"
            "MzM0NzQ2MCwzMzMzMzMzMzMzMzMzMzMzMzMzNzgwLDMzMzMzMzMzMzMzMzM0MzMzMzMxNjUzNzM3"
            "MzAsMzMzMzMzMzMzMzMzMzMzMzMzMzc4MCwzMzMzMzMzODM1NTMzMDc0NzQ2MCwzMzE2NjUwMDAw"
            "MzMzMzMzMDc0NzMzMzMzMzM3NzksMzMzMzMzMzMzMzMzMzMzMzQzMzMzMzMzMDc0NzQ2MCwzMzMz"
            "MzMzMzMzMzMzMzMzMzMzMzc4MCwzMzMzMzMzMzMzMzMzMzMwODgzNTUzMzA3NDM3ODAsMzMzMzMz"
            "MzMzMzMzMzMzMDg4MzU1MzMwNzQ3NDYwLDMzMzMzMzMzMTY2NTAwMDAzMzMzMzQ3NDYwLDMzMzMz"
            "MzMzMzMzMzMzMzMzMzM3ODAsMzMzMzMzMzMzMzMzNzMzMzM0MzMzMzMzMzA3NDc0NjAsMzMzMzMz"
            "MzMzMzMzMzMzMzMzMzc4MCwzMzMzMzMzMzMzMzMwODgzNTUzMzA3NDc0NjAsMzMxNjY1MDAwMDMz"
            "MzMzMzA3NDczMzMzMzMzNzc5LDMzMzMzMzMzMzMzMzMzMzM0MzMzMzNcdWQ4N2RcdWRlZGV1ZGM4"
            "ZGUzOVx1ZDg0ZFx1ZGM4M2RcdWQ4OGRcdWRlMzlcdWQ4OWRcdWRlMjM5MzMzZWUzOVxk";
        auto const len = base64::decoded_size(s.size());
        std::unique_ptr<char[]> p(new char[len]);
        base64::decode(
            p.get(), s.data(), s.size());
        string_view const js(p.get(), len);
        BOOST_TEST(! validate(js));
    }

    void
    run()
    {
        testNull();
        testBoolean();
        testString();
        testNumber();
        testArray();
        testObject();
        testParser();
        testMembers();
        testParseVectors();
        testIssue13();
        testIssue20();
    }
};

TEST_SUITE(basic_parser_test, "boost.json.basic_parser");

} // json
} // boost
