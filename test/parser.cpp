//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/parser.hpp>

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/serializer.hpp>

#include <sstream>
#include <iostream>

#include "parse-vectors.hpp"
#include "test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace json {

class parser_test
{
public:
    ::test_suite::log_type log;

    static
    value
    from_string_test(
        string_view s,
        storage_ptr sp = {})
    {
        parser p;
        error_code ec;
        p.start(std::move(sp));
        p.write(s.data(), s.size(), ec);
        if(BOOST_TEST(! ec))
            p.finish(ec);
        BOOST_TEST(! ec);
        return p.release();
    }

    void
    static
    check_round_trip(value const& jv1)
    {
        auto const s2 =
            //to_string_test(jv1); // use this if serializer is broken
            to_string(jv1);
        auto jv2 =
            from_string_test(s2);
        BOOST_TEST(equal(jv1, jv2));
    }

    template<class F>
    void
    static
    grind_one(
        string_view s,
        storage_ptr sp,
        F const& f)
    {
        auto const jv =
            from_string_test(s, sp);
        f(jv);
    }

    static
    void
    grind_one(string_view s)
    {
        auto const jv =
            from_string_test(s);
        check_round_trip(jv);
    }

    template<class F>
    static
    void
    grind(string_view s, F const& f)
    {
        try
        {
            grind_one(s, {}, f);

            fail_loop([&](storage_ptr const& sp)
            {
                grind_one(s, sp, f);
            });

            if(s.size() > 1)
            {
                // Destroy the parser at every
                // split point to check leaks.
                for(std::size_t i = 1;
                    i < s.size(); ++i)
                {
                    fail_resource mr;
                    mr.fail_max = 0;
                    parser p;
                    error_code ec;
                    p.start(&mr);
                    p.write(s.data(), i, ec);
                    if(BOOST_TEST(! ec))
                        p.write(
                            s.data() + i,
                            s.size() - i, ec);
                    if(BOOST_TEST(! ec))
                        p.finish(ec);
                    if(BOOST_TEST(! ec))
                        f(p.release());
                }
            }
        }
        catch(std::exception const&)
        {
            BOOST_TEST_FAIL();
        }
    }

    static
    void
    grind(string_view s)
    {
        grind(s,
            [](value const& jv)
            {
                check_round_trip(jv);
            });
    }

    static
    void
    grind_int64(string_view s, int64_t v)
    {
        grind(s,
            [v](value const& jv)
            {
                if(! BOOST_TEST(jv.is_int64()))
                    return;
                BOOST_TEST(jv.get_int64() == v);
            });
    }

    static
    void
    grind_uint64(string_view s, uint64_t v)
    {
        grind(s,
            [v](value const& jv)
            {
                if(! BOOST_TEST(jv.is_uint64()))
                    return;
                BOOST_TEST(jv.get_uint64() == v);
            });
    }

    static
    void
    grind_double(string_view s, double v)
    {
        grind(s,
            [v](value const& jv)
            {
                if(! BOOST_TEST(jv.is_double()))
                    return;
                BOOST_TEST(jv.get_double() == v);
            });
    }

    //------------------------------------------------------

    void
    testNull()
    {
        grind("null");
        grind(" null");
        grind("  null");
        grind("null\n");
        grind("null\n\n");
        grind("\r null\t ");
    }

    void
    testBool()
    {
        grind("true");
        grind(" true");
        grind("  true");
        grind("true\n");
        grind("true\n\n");
        grind("\r true\t ");

        grind("false");
        grind(" false");
        grind("  false");
        grind("false\n");
        grind("false\n\n");
        grind("\r false\t ");
    }

    //------------------------------------------------------

    void
    testString()
    {
        grind("\"\"");
        grind("\"x\"");
        grind(" \"x\"");
        grind("  \"x\"");
        grind("\"x\"\n");
        grind("\"x\"\n\n");
        grind("\r \"x\"\t ");

        grind("\"abcdefghij\"");
        grind("\""
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "\"");
        grind("\""
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"
            "\"");

        // escapes
        grind("\"\\\"\"");
        grind("\"\\\\\"");
        grind("\"\\/\"");
        grind("\"\\b\"");
        grind("\"\\f\"");
        grind("\"\\n\"");
        grind("\"\\r\"");
        grind("\"\\t\"");

        // unicode
        grind("\"\\u0000\"");
        grind("\"\\ud7fF\"");
        grind("\"\\ue000\"");
        grind("\"\\ufFfF\"");
        grind("\"\\ud800\\udc00\"");
        grind("\"\\udbff\\udffF\"");

        // big string
        {
            std::string const big(4000, '*');
            {
                std::string js;
                js = "\"" + big + "\"";
                auto const N = js.size() / 2;
                error_code ec;
                parser p;
                p.start();
                p.write(js.data(), N, ec);
                if(BOOST_TEST(! ec))
                {
                    p.write(js.data() + N,
                        js.size() - N, ec);
                    if(BOOST_TEST(! ec))
                        p.finish(ec);
                    if(BOOST_TEST(! ec))
                        check_round_trip(
                            p.release());
                }   
            }
        }
    }

    //------------------------------------------------------
    
    struct f_boost
    {
        static
        string_view
        name() noexcept
        {
            return "boost";
        }

        double
        operator()(string_view s) const
        {
            BOOST_TEST_CHECKPOINT();
            error_code ec;
            parser p;
            p.start();
            p.write(s.data(), s.size(), ec);
            if(BOOST_TEST(! ec))
                p.finish(ec);
            if(! BOOST_TEST(! ec))
                return 0;
            auto const jv = p.release();
            double const d = jv.as_double();
            grind_double(s, d);
            return d;
        }
    };

    bool
    within_1ulp(double x, double y)
    {
        std::uint64_t bx, by;
        std::memcpy(&bx, &x, sizeof(x));
        std::memcpy(&by, &y, sizeof(y));

        auto diff = bx - by;
        switch (diff)
        {
        case 0:
        case 1:
        case 0xffffffffffffffff:
            return true;
        default:
            break;
        }
        return false;
    }

    // Verify that f converts to the
    // same double produced by `strtod`.
    // Requires `s` is not represented by an integral type.
    template<class F>
    void
    fc(std::string const& s, F const& f)
    {
        char* str_end;
        double const need =
            std::strtod(s.c_str(), &str_end);
        // BOOST_TEST(str_end == &s.back() + 1);
        double const got = f(s);
        auto same = got == need;
        auto close = same ?
            true : within_1ulp(got, need);

        if( !BOOST_TEST(close) )
        {
            std::cerr << "Failure on '" << s << "': " << got << " != " << need << "\n";
        }
    }

    void
    fc(std::string const& s)
    {
        fc(s, f_boost{});
        fc(s + std::string( 64, ' ' ), f_boost{});
    }

    void
    testNumber()
    {
        grind("0");
        grind(" 0");
        grind("  0");
        grind("0\n");
        grind("0\n\n");
        grind("\r 0\t ");

        grind_int64( "-9223372036854775808", INT64_MIN);
        grind_int64( "-9223372036854775807", -9223372036854775807);
        grind_int64( "-999999999999999999", -999999999999999999);
        grind_int64( "-99999999999999999", -99999999999999999);
        grind_int64( "-9999999999999999", -9999999999999999);
        grind_int64( "-999999999999999", -999999999999999);
        grind_int64( "-99999999999999", -99999999999999);
        grind_int64( "-9999999999999", -9999999999999);
        grind_int64( "-999999999999", -999999999999);
        grind_int64( "-99999999999", -99999999999);
        grind_int64( "-9999999999", -9999999999);
        grind_int64( "-999999999", -999999999);
        grind_int64( "-99999999", -99999999);
        grind_int64( "-9999999", -9999999);
        grind_int64( "-999999", -999999);
        grind_int64( "-99999", -99999);
        grind_int64( "-9999", -9999);
        grind_int64( "-999", -999);
        grind_int64( "-99", -99);
        grind_int64( "-9", -9);
        grind_int64( "-0", 0);
        grind_int64(  "0", 0);
        grind_int64(  "1", 1);
        grind_int64(  "9", 9);
        grind_int64(  "99", 99);
        grind_int64(  "999", 999);
        grind_int64(  "9999", 9999);
        grind_int64(  "99999", 99999);
        grind_int64(  "999999", 999999);
        grind_int64(  "9999999", 9999999);
        grind_int64(  "99999999", 99999999);
        grind_int64(  "999999999", 999999999);
        grind_int64(  "9999999999", 9999999999);
        grind_int64(  "99999999999", 99999999999);
        grind_int64(  "999999999999", 999999999999);
        grind_int64(  "9999999999999", 9999999999999);
        grind_int64(  "99999999999999", 99999999999999);
        grind_int64(  "999999999999999", 999999999999999);
        grind_int64(  "9999999999999999", 9999999999999999);
        grind_int64(  "99999999999999999", 99999999999999999);
        grind_int64(  "999999999999999999", 999999999999999999);
        grind_int64(  "9223372036854775807", INT64_MAX);

        grind_uint64( "9223372036854775808", 9223372036854775808ULL);
        grind_uint64( "9999999999999999999", 9999999999999999999ULL);
        grind_uint64( "18446744073709551615", UINT64_MAX);

        grind_double("-1.010", -1.01);
        grind_double("-0.010", -0.01);
        grind_double("-0.0", -0.0);
        grind_double("-0e0", -0.0);
        grind_double( "18446744073709551616",  1.8446744073709552e+19);
        grind_double("-18446744073709551616", -1.8446744073709552e+19);
        grind_double( "18446744073709551616.0",  1.8446744073709552e+19);
        grind_double( "18446744073709551616.00009",  1.8446744073709552e+19);
        grind_double( "1844674407370955161600000",  1.8446744073709552e+24);
        grind_double("-1844674407370955161600000", -1.8446744073709552e+24);
        grind_double( "1844674407370955161600000.0",  1.8446744073709552e+24);
        grind_double( "1844674407370955161600000.00009",  1.8446744073709552e+24);

        grind_double( "1.0", 1.0);
        grind_double( "1.1", 1.1);
        grind_double( "1.11", 1.11);
        grind_double( "1.11111", 1.11111);
        grind_double( "11.1111", 11.1111);
        grind_double( "111.111", 111.111);

        fc("-999999999999999999999");
        fc("-100000000000000000009");
        fc("-10000000000000000000");
        fc("-9223372036854775809");

        fc("18446744073709551616");
        fc("99999999999999999999");
        fc("999999999999999999999");
        fc("1000000000000000000000");
        fc("9999999999999999999999");
        fc("99999999999999999999999");

        fc("-0.9999999999999999999999");
        fc("-0.9999999999999999");
        fc("-0.9007199254740991");
        fc("-0.999999999999999");
        fc("-0.99999999999999");
        fc("-0.9999999999999");
        fc("-0.999999999999");
        fc("-0.99999999999");
        fc("-0.9999999999");
        fc("-0.999999999");
        fc("-0.99999999");
        fc("-0.9999999");
        fc("-0.999999");
        fc("-0.99999");
        fc("-0.9999");
        fc("-0.8125");
        fc("-0.999");
        fc("-0.99");
        fc("-1.0");
        fc("-0.9");
        fc("-0.0");
        fc("0.0");
        fc("0.9");
        fc("0.99");
        fc("0.999");
        fc("0.8125");
        fc("0.9999");
        fc("0.99999");
        fc("0.999999");
        fc("0.9999999");
        fc("0.99999999");
        fc("0.999999999");
        fc("0.9999999999");
        fc("0.99999999999");
        fc("0.999999999999");
        fc("0.9999999999999");
        fc("0.99999999999999");
        fc("0.999999999999999");
        fc("0.9007199254740991");
        fc("0.9999999999999999");
        fc("0.9999999999999999999999");
        fc("0.999999999999999999999999999");

        fc("-1e308");
        fc("-1e-308");
        fc("-9999e300");
        fc("-999e100");
        fc("-99e10");
        fc("-9e1");
        fc("9e1");
        fc("99e10");
        fc("999e100");
        fc("9999e300");
        fc("999999999999999999.0");
        fc("999999999999999999999.0");
        fc("999999999999999999999e5");
        fc("999999999999999999999.0e5");

        fc("0.00000000000000001");

        fc("-1e-1");
        fc("-1e0");
        fc("-1e1");
        fc("0e0");
        fc("1e0");
        fc("1e10");

        fc("0."
           "00000000000000000000000000000000000000000000000000" // 50 zeroes
           "1e50");
        fc("-0."
           "00000000000000000000000000000000000000000000000000" // 50 zeroes
           "1e50");

        fc("0."
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000" // 500 zeroes
           "1e600");
        fc("-0."
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000" // 500 zeroes
           "1e600");

        fc("0e"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000" // 500 zeroes
        );
    }

    //------------------------------------------------------

    void
    testArray()
    {
        grind("[]");
        grind(" []");
        grind("[] ");
        grind(" [] ");
        grind(" [ ] ");
        grind("[1]");
        grind("[ 1]");
        grind("[1 ]");
        grind("[ 1 ]");
        grind("[1,2]");
        grind("[ 1,2]");
        grind("[1 ,2]");
        grind("[1, 2]");
        grind("[1,2 ]");
        grind("[ 1 ,2]");
        grind("[1 , 2]");
        grind("[1, 2 ]");

        grind("[[]]");
        grind("[[],[]]");
        grind("[[],[],[]]");
        grind("[[[]],[[],[]],[[],[],[]]]");
        grind("[{},[],\"x\",1,-1,1.0,true,null]");

        // depth
        {
            error_code ec;
            parser p;
            p.start();
            BOOST_TEST(
                p.depth() == 0);
            BOOST_TEST(
                p.max_depth() > 0);
            p.max_depth(0);
            BOOST_TEST(
                p.max_depth() == 0);
            p.write("[]", 2, ec);
            BOOST_TEST(
                ec == error::too_deep);
        }
    }

    //------------------------------------------------------

    void
    testObject()
    {
        grind("{}");
        grind(" {}");
        grind("{} ");
        grind(" {} ");
        grind(" { } ");
        grind("{\"1\":1}");
        grind("{ \"1\":1}");
        grind("{\"1\" :1}");
        grind("{\"1\": 1}");
        grind("{\"1\":1 }");
        grind("{ \"1\" :1 }");
        grind("{\"1\" : 1 }");
        grind("{\"1\":1,\"2\":2}");
        grind("{\"1\":1, \"2\":2}");
        grind("{\"1\":1, \"2\" : 2 }");

        grind("{\"\":[]}");
        grind("{\"1\":[],\"2\":[]}");

        grind(
            "{\"1\":{\"2\":{}},\"3\":{\"4\":{},\"5\":{}},"
            "\"6\":{\"7\":{},\"8\":{},\"9\":{}}}");

        grind(
            "{\"1\":{},\"2\":[],\"3\":\"x\",\"4\":1,"
            "\"5\":-1,\"6\":1.0,\"7\":false,\"8\":null}");

        // big keys
        {
            std::string const big(4000, '*');
            {
                std::string js;
                js = "{\"" + big + "\":null}";
                grind(js);
            }

            {
                std::string js;
                js = "{\"x\":\"" + big + "\"}";
                grind(js);
            }

            {
                std::string js;
                js = "{\"" + big + "\":\"" + big + "\"}";
                grind(js);
            }
        }

        // depth
        {
            error_code ec;
            parser p;
            p.start();
            BOOST_TEST(
                p.depth() == 0);
            BOOST_TEST(
                p.max_depth() > 0);
            p.max_depth(0);
            BOOST_TEST(
                p.max_depth() == 0);
            p.write("{}", 2, ec);
            BOOST_TEST(
                ec == error::too_deep);
        }
    }

    void
    testMembers()
    {
        // reserve
        {
            parser p;
            p.reserve(1024);
        }

        // need start error
        {
            parser p;
            error_code ec;
            p.write("", 0, ec);
            BOOST_TEST(
                ec == error::need_start);
        }

        // destroy after start
        {
            parser p;
            p.start();
        }

        // release before done
        {
            parser p;
            BOOST_TEST_THROWS(
                p.release(),
                std::logic_error);
        }

        // reserve
        {
            parser p;
            p.reserve(16384);
            p.reserve(100);
        }

        // write(char const*, size_t)
        {
            {
                parser p;
                p.start();
                BOOST_TEST(p.write(
                    "null", 4) == 4);
            }
            {
                parser p;
                p.start();
                BOOST_TEST_THROWS(
                    p.write("x", 1),
                    system_error);
            }
        }
    }

    //------------------------------------------------------

    void
    testFreeFunctions()
    {
        string_view const js =
            "{\"1\":{},\"2\":[],\"3\":\"x\",\"4\":1,"
            "\"5\":-1,\"6\":1.0,\"7\":false,\"8\":null}";

        // parse(string_view, error_code)
        {
            {
                error_code ec;
                auto jv = parse(js, ec);
                BOOST_TEST(! ec);
                check_round_trip(jv);
            }
            {
                error_code ec;
                auto jv = parse("xxx", ec);
                BOOST_TEST(ec);
                BOOST_TEST(jv.is_null());
            }
        }

        // parse(string_view, storage_ptr, error_code)
        {
            {
                error_code ec;
                monotonic_resource mr;
                auto jv = parse(js, ec, &mr);
                BOOST_TEST(! ec);
                //check_round_trip(jv);
            }

            {
                error_code ec;
                monotonic_resource mr;
                auto jv = parse("xxx", ec, &mr);
                BOOST_TEST(ec);
                BOOST_TEST(jv.is_null());
            }
        }

        // parse(string_view)
        {
            {
                check_round_trip(
                    parse(js));
            }

            {
                value jv;
                BOOST_TEST_THROWS(
                    jv = parse("{,"),
                    system_error);
            }
        }

        // parse(string_view, storage_ptr)
        {
            {
                monotonic_resource mr;
                check_round_trip(parse(js, &mr));
            }

            {
                monotonic_resource mr;
                value jv;
                BOOST_TEST_THROWS(
                    jv = parse("xxx", &mr),
                    system_error);
            }
        }
    }

    void
    testSampleJson()
    {
        string_view in =
R"xx({
    "glossary": {
        "title": "example glossary",
		"GlossDiv": {
            "title": "S",
			"GlossList": {
                "GlossEntry": {
                    "ID": "SGML",
					"SortAs": "SGML",
					"GlossTerm": "Standard Generalized Markup Language",
					"Acronym": "SGML",
					"Abbrev": "ISO 8879:1986",
					"GlossDef": {
                        "para": "A meta-markup language, used to create markup languages such as DocBook.",
						"GlossSeeAlso": ["GML", "XML"]
                    },
					"GlossSee": "markup"
                }
            }
        }
    }
})xx"
        ;
        parser p;
        error_code ec;
        p.start();
        p.write(in.data(), in.size(), ec);
        if(BOOST_TEST(! ec))
            p.finish(ec);
        if(BOOST_TEST(! ec))
        {
            BOOST_TEST(to_string(p.release()) ==
                "{\"glossary\":{\"title\":\"example glossary\",\"GlossDiv\":"
                "{\"title\":\"S\",\"GlossList\":{\"GlossEntry\":{\"ID\":\"SGML\","
                "\"SortAs\":\"SGML\",\"GlossTerm\":\"Standard Generalized Markup "
                "Language\",\"Acronym\":\"SGML\",\"Abbrev\":\"ISO 8879:1986\","
                "\"GlossDef\":{\"para\":\"A meta-markup language, used to create "
                "markup languages such as DocBook.\",\"GlossSeeAlso\":[\"GML\",\"XML\"]},"
                "\"GlossSee\":\"markup\"}}}}}"
            );
        }
    }

    void
    testUnicodeStrings()
    {
        // Embedded NULL correctly converted
        {
            auto expected = string_view("Hello\x00World", 11);
            {
                auto s = string_view(R"json("Hello\u0000World")json");
                grind(s);
                BOOST_TEST(json::parse(s).as_string() == expected);
            }
            {
                auto s = string_view(R"json(["Hello\u0000World"])json");
                grind(s);
                BOOST_TEST(json::parse(s).as_array().at(0).as_string() == expected);
            }
        }

        // surrogate pairs correctly converted to UTF-8
        {
            auto expected = string_view("\xF0\x9D\x84\x9E", 4);
            {
                auto s = string_view(R"json("\uD834\uDD1E")json");
                grind(s);
                BOOST_TEST(json::parse(s).as_string() == expected);
            }
            {
                auto s = string_view(R"json(["\uD834\uDD1E"])json");
                grind(s);
                BOOST_TEST(json::parse(s).as_array().at(0).as_string() == expected);
            }
        }
    }

    //------------------------------------------------------

    // https://github.com/vinniefalco/json/issues/15
    void
    testIssue15()
    {
        BOOST_TEST(
            json::parse("{\"port\": 12345}")
                .as_object()
                .at("port")
                .as_int64() == 12345);
    }

    // https://github.com/vinniefalco/json/issues/45
    void
    testIssue45()
    {
        struct T
        {
            value jv;

            T(value jv_)
                : jv(jv_)
            {
            }
        };

        auto const jv = parse("[]");
        auto const t = T{jv};
        BOOST_TEST(to_string(t.jv) == "[]");
    }

    //------------------------------------------------------

    void
    run()
    {
        testNull();
        testBool();
        testString();
        testNumber();
        testArray();
        testObject();
        testMembers();
        testFreeFunctions();
        testSampleJson();
        testUnicodeStrings();
        testIssue15();
        testIssue45();
    }
};

TEST_SUITE(parser_test, "boost.json.parser");

} // json
} // boost
