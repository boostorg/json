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

    struct null_parser : basic_parser
    {
        null_parser() {}
        ~null_parser() {}
        void on_document_begin( error_code& ) override {}
        void on_document_end( error_code& ) override {}
        void on_object_begin( error_code& ) override {}
        void on_object_end( error_code& ) override {}
        void on_array_begin( error_code& ) override {}
        void on_array_end( error_code& ) override {}
        void on_key_part( string_view, error_code& ) override {}
        void on_key( string_view, error_code& ) override {}
        void on_string_part( string_view, error_code& ) override {}
        void on_string( string_view, error_code& ) override {}
        void on_int64( std::int64_t, error_code& ) override {}
        void on_uint64( std::uint64_t, error_code& ) override {}
        void on_double( double, error_code& ) override {}
        void on_bool( bool, error_code& ) override {}
        void on_null( error_code& ) override {}
    };

    // Parse with the null parser and return false on error
    null_parser p;
    error_code ec;
    p.finish( s.data(), s.size(), ec );
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
    split_grind(
        string_view s,
        error_code ex = {})
    {
        // make sure all split inputs
        // produce the same result.
        for(std::size_t i = 1;
            i < s.size(); ++i)
        {
            if(! BOOST_TEST(i < 100000))
                break;
            error_code ec;
            fail_parser p;
            auto const n =
                p.write_some(s.data(), i, ec);
            if(ec)
            {
                BOOST_TEST(ec == ex);
                continue;
            }
            p.write(
                s.data() + n,
                s.size() - n, ec);
            if(! ec)
                p.finish(ec);
            if(! BOOST_TEST(ec == ex))
                log << "should be " << ex.message() << std::endl;
        }
    }

    void
    fail_grind(
        string_view s,
        error_code ex = {})
    {
        // exercise all error paths
        for(std::size_t j = 1;;++j)
        {
            if(! BOOST_TEST(j < 100000))
                break;
            error_code ec;
            fail_parser p(j);
            p.write(
                s.data(), s.size(), ec);
            if(! ec)
                p.finish(ec);
            if(ec == error::test_failure)
                continue;
            BOOST_TEST(ec == ex);
            break;
        }
    }

    void
    throw_grind(
        string_view s,
        error_code ex = {})
    {
        // exercise all exception paths
        for(std::size_t j = 1;;++j)
        {
            if(! BOOST_TEST(j < 100000))
                break;
            error_code ec;
            throw_parser p(j);
            try
            {
                p.write(
                    s.data(), s.size(), ec);
                if(! ec)
                    p.finish(ec);
                BOOST_TEST(ec == ex);
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

    void
    grind(string_view s, bool good)
    {
        error_code ex;
        {
            fail_parser p;
            p.write(
                s.data(),
                s.size(),
                ex);
            if(! ex)
                p.finish(ex);
            if(good)
            {
                if(! BOOST_TEST(! ex))
                    return;
            }
            else
            {
                if(! BOOST_TEST(ex))
                    return;
            }
        }

        split_grind(s, ex);
        throw_grind(s, ex);
        fail_grind(s, ex);
    }

    void
    good(string_view s)
    {
        grind(s, true);
    }

    void
    bad(string_view s)
    {
        grind(s, false);
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
    }

    void
    testString()
    {
        good("\""   "x"         "\"");
        good("\""   "xy"        "\"");
        good("\""   "x y"       "\"");

        bad ("\""   "\t"        "\"");

        // control after escape
        bad ("\"\\\\\n\"");
    }

    void
    testNumber()
    {
        good("0");
        good("0.0");
        good("0.10");
        good("0.01");
        good("1");
        good("10");
        good("1.5");
        good("10.5");
        good("10.25");
        good("10.25e0");
        good("1e1");
        good("1e10");
        good("1e+0");
        good("1e+1");
        good("0e+10");
        good("0e-0");
        good("0e-1");
        good("0e-10");
        good("1E+1");
        good("-0");
        good("-1");
        good("-1e1");
        good("1.1e309");
        good(   "9223372036854775807");
        good(  "-9223372036854775807");
        good(  "18446744073709551615");
        good( "-18446744073709551615");
        good(  "[9223372036854775807]");
        good( "[-9223372036854775807]");
        good( "[18446744073709551615]");
        good("[-18446744073709551615]");

        bad ("");
        bad ("-");
        bad ("00");
        bad ("00.");
        bad ("00.0");
        bad ("1a");
        bad (".");
        bad ("1.");
        bad ("1+");
        bad ("0.0+");
        bad ("0.0e+");
        bad ("0.0e-");
        bad ("0.0e0-");
        bad ("0.0e");
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

        bad ("TRUE");
        bad ("tRUE");
        bad ("trUE");
        bad ("truE");
        bad ("truex");
        bad ("tru");
        bad ("tr");
        bad ("t");

        good("false");
        good(" false");
        good("false ");
        good("\tfalse");
        good("false\t");
        good("\r\n\t false\r\n\t ");

        bad ("FALSE");
        bad ("fALSE");
        bad ("faLSE");
        bad ("falSE");
        bad ("falsE");
        bad ("falsex");
        bad ("fals");
        bad ("fal");
        bad ("fa");
        bad ("f");
    }

    void
    testNull()
    {
        good("null");
        good(" null");
        good("null ");
        good("\tnull");
        good("null\t");
        good("\r\n\t null\r\n\t ");

        bad ("NULL");
        bad ("nULL");
        bad ("nuLL");
        bad ("nulL");
        bad ("nullx");
        bad ("nul");
        bad ("nu");
        bad ("n");
    }

    void
    testParser()
    {
        auto const check =
        [this]( string_view s,
            bool is_done)
        {
            fail_parser p;
            error_code ec;
            p.write_some(
                s.data(), s.size(),
                ec);
            if(! BOOST_TEST(! ec))
            {
                log << "    failed to parse: " << s;
                return;
            }
            BOOST_TEST(is_done ==
                p.is_done());
        };

        // is_done()

        check("{}", false);
        check("{} ", false);
        check("{}x", true);
        check("{} x", true);

        check("[]", false);
        check("[] ", false);
        check("[]x", true);
        check("[] x", true);

        check("\"a\"", false);
        check("\"a\" ", false);
        check("\"a\"x", true);
        check("\"a\" x", true);

        check("0", false);
        check("0 ", false);
        check("0x", true);
        check("0 x", true);
        check("0.", false);
        check("0.0", false);
        check("0.0 ", false);
        check("0.0 x", true);

        check("true", false);
        check("true ", false);
        check("truex", true);
        check("true x", true);

        check("false", false);
        check("false ", false);
        check("falsex", true);
        check("false x", true);

        check("null", false);
        check("null ", false);
        check("nullx", true);
        check("null x", true);

        // depth(), max_depth(), is_done()
        {
            {
                error_code ec;
                fail_parser p;
                BOOST_TEST(
                    p.depth() == 0);
                BOOST_TEST(
                    p.max_depth() > 0);
                p.max_depth(1);
                p.write("[{}]", 4, ec);
                BOOST_TEST(
                    ec == error::too_deep);
                BOOST_TEST(! p.is_done());
            }
            {
                error_code ec;
                fail_parser p;
                BOOST_TEST(
                    p.max_depth() > 0);
                p.max_depth(1);
                p.write_some("[", 1, ec);
                BOOST_TEST(p.depth() == 1);
                if(BOOST_TEST(! ec))
                {
                    p.write_some("{", 1, ec);
                    BOOST_TEST(
                        ec == error::too_deep);
                }
                BOOST_TEST(! p.is_done());
                ec = {};
                p.write_some("{}", 2, ec);
                BOOST_TEST(ec);
                p.reset();
                p.write("{}", 2, ec);
                if(! ec)
                    p.finish(ec);
                BOOST_TEST(! ec);
                BOOST_TEST(p.is_done());
            }
        }

        // maybe_flush
        {
            // VFALCO This must be equal to the size
            // of the temp buffer used in write_some.
            //
            int constexpr BUFFER_SIZE = 2048;

            {
                for(auto esc :
                    { "\\\"", "\\\\", "\\/", "\\b",
                      "\\f", "\\n", "\\r", "\\t", "\\u0000" })
                {
                    std::string big;
                    big = "\\\"" + std::string(BUFFER_SIZE-4, '*') + esc;
                    std::string s;
                    s = "{\"" + big + "\":\"" + big + "\"}";
                    fail_grind(s);
                }
            }

            {
                std::string big;
                big = "\\\"" +
                    std::string(BUFFER_SIZE + 1, '*');
                std::string s;
                s = "{\"" + big + "\":\"" + big + "\"}";
                fail_grind(s);
            }
        }

        // no input
        {
            error_code ec;
            fail_parser p;
            p.finish(ec);
            BOOST_TEST(ec);
        }
    }

    void
    testMembers()
    {
        // write_some(char const*, size_t, error_code&)
        {
            {
                error_code ec;
                fail_parser p;
                p.write_some("0", 1, ec);
                BOOST_TEST(! ec);
            }

            // partial write
            {
                error_code ec;
                fail_parser p;
                auto const n =
                    p.write_some("null x", 6, ec);
                BOOST_TEST(! ec);
                BOOST_TEST(n < 6);
            }
        }

        // write_some(char const*, size_t)
        {
            fail_parser p;
            BOOST_TEST_THROWS(
                p.write_some("x", 1),
                system_error);
        }

        // write(char const*, size_t, error_code&)
        {
            error_code ec;
            fail_parser p;
            p.write("0x", 2, ec);
            BOOST_TEST(
                ec == error::extra_data);
        }

        // write(char const*, size_t)
        {
            {
                fail_parser p;
                p.write("0", 1);
            }

            {
                fail_parser p;
                BOOST_TEST_THROWS(
                    p.write("0x", 2),
                    system_error);
            }
        }

        // finish(char const*, size_t, error_code&)
        {
            error_code ec;
            fail_parser p;
            p.finish("{", 1, ec);
            BOOST_TEST(
                ec == error::incomplete);
        }

        // finish(char const*, size_t)
        {
            {
                fail_parser p;
                p.finish("{}", 2);
            }

            {
                fail_parser p;
                BOOST_TEST_THROWS(
                    p.finish("{", 1),
                    system_error);
            }
        }

        // finish()
        {
            {
                fail_parser p;
                p.write("{}", 2);
                BOOST_TEST(! p.is_done());
                p.finish();
                BOOST_TEST(p.is_done());
            }

            {
                fail_parser p;
                p.write("{", 1);
                BOOST_TEST(! p.is_done());
                BOOST_TEST_THROWS(
                    p.finish(),
                    system_error);
            }
        }
    }

    void
    testParseVectors()
    {
        parse_vectors pv;
        for(auto const& v : pv)
        {
            if(v.result == 'i')
            {
                error_code ec;
                fail_parser p;
                p.write(
                    v.text.data(),
                    v.text.size(),
                    ec);
                grind(v.text,
                    ec ? false : true);
                continue;
            }
            if(v.result == 'y')
                grind(v.text, true);
            else
                grind(v.text, false);
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
        testObject();
        testArray();
        testString();
        testNumber();
        testBoolean();
        testNull();
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
