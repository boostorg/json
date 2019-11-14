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

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <string>

#include "test.hpp"
#include "parse-vectors.hpp"

namespace boost {
namespace json {

class basic_parser_test : public beast::unit_test::suite
{
public:
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
            if(! BEAST_EXPECT(i < 100000))
                break;
            error_code ec;
            fail_parser p;
            auto const n =
                p.write_some(s.data(), i, ec);
            if(ec)
            {
                BEAST_EXPECTS(ec == ex,
                    ec.message());
                continue;
            }
            p.write(
                s.data() + n,
                s.size() - n, ec);
            if(! BEAST_EXPECTS(ec == ex,
                ec.message()))
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
            if(! BEAST_EXPECT(j < 100000))
                break;
            error_code ec;
            fail_parser p(j);
            p.write(
                s.data(), s.size(), ec);
            if(ec == error::test_failure)
                continue;
            BEAST_EXPECTS(ec == ex,
                ec.message());
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
            if(! BEAST_EXPECT(j < 100000))
                break;
            error_code ec;
            throw_parser p(j);
            try
            {
                p.write(
                    s.data(), s.size(), ec);
                BEAST_EXPECTS(ec == ex,
                    ec.message());
                break;
            }
            catch(test_exception const&)
            {
                continue;
            }
            catch(std::exception const& e)
            {
                BEAST_FAIL();
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
            if(good)
            {
                if(! BEAST_EXPECTS(
                    ! ex, ex.message()))
                    return;
            }
            else
            {
                if(! BEAST_EXPECT(ex))
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
        []( string_view s,
            bool is_done)
        {
            fail_parser p;
            error_code ec;
            p.write_some(
                s.data(), s.size(),
                ec);
            if(! BEAST_EXPECTS(! ec,
                ec.message()))
                return;
            BEAST_EXPECT(is_done ==
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
        check("00", true);
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
                BEAST_EXPECT(
                    p.depth() == 0);
                BEAST_EXPECT(
                    p.max_depth() > 0);
                p.max_depth(1);
                p.write("[{}]", 4, ec);
                BEAST_EXPECTS(
                    ec == error::too_deep,
                    ec.message());
                BEAST_EXPECT(! p.is_done());
            }
            {
                error_code ec;
                fail_parser p;
                BEAST_EXPECT(
                    p.max_depth() > 0);
                p.max_depth(1);
                p.write_some("[", 1, ec);
                BEAST_EXPECT(p.depth() == 1);
                if(BEAST_EXPECTS(! ec,
                    ec.message()))
                {
                    p.write_some("{", 1, ec);
                    BEAST_EXPECTS(
                        ec == error::too_deep,
                        ec.message());
                }
                BEAST_EXPECT(! p.is_done());
                ec = {};
                p.write_some("{}", 2, ec);
                BEAST_EXPECT(ec);
                p.reset();
                p.write("{}", 2, ec);
                BEAST_EXPECTS(! ec, ec.message());
                BEAST_EXPECT(p.is_done());
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
            p.write_eof(ec);
            BEAST_EXPECT(ec);
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

    void
    run() override
    {
        testObject();
        testArray();
        testString();
        testNumber();
        testBoolean();
        testNull();
        testParser();
        testParseVectors();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,basic_parser);

} // json
} // boost
