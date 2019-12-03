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

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/json/pool.hpp>
#include <boost/json/serializer.hpp>
#include <sstream>

#include "parse-vectors.hpp"
#include "test.hpp"

namespace boost {
namespace json {

class parser_test : public beast::unit_test::suite
{
public:
    value
    from_string_test(
        string_view s,
        storage_ptr sp = {})
    {
        parser p;
        error_code ec;
        p.start(std::move(sp));
        p.write(
            s.data(),
            s.size(),
            ec);
        if(! ec)
            p.finish(ec);
        BEAST_EXPECTS(! ec,
            ec.message());
        //log << "  " << to_string_test(p.get()) << std::endl;
        return p.release();
    }

    void
    check_round_trip(
        value const& jv1,
        string_view s1)
    {
        auto const s2 =
            //to_string_test(jv1); // use this if serializer is broken
            to_string(jv1);
        auto jv2 =
            from_string_test(s2);
        if(! BEAST_EXPECT(equal(jv1, jv2)))
            log <<
                "  " << s1 << "\n" <<
                "  " << s2 << std::endl;
    }

    void
    grind_one(string_view s)
    {
        auto const jv =
            from_string_test(s);
        check_round_trip(jv, s);
    }

    void
    grind(string_view s)
    {
        grind_one(s);

        fail_loop([&](storage_ptr const& sp)
        {
            auto const jv =
                from_string_test(s, sp);
            check_round_trip(jv, s);
        });

        if(s.size() > 1)
        {
            // Destroy the parser at every
            // split point to check leaks.
            for(std::size_t i = 1;
                i < s.size(); ++i)
            {
                scoped_storage<
                    fail_storage> ss;
                ss->fail_max = 0;
                parser p;
                error_code ec;
                p.start(ss);
                p.write(s.data(), i, ec);
                if(! ec)
                    p.finish(ec);
            }
        }

    }
    
    void
    testObjects()
    {
        grind("{}");
        grind("{\"\":[]}");
        grind("{\"1\":[],\"2\":[]}");
        grind(
            "{\"1\":{\"2\":{}},\"3\":{\"4\":{},\"5\":{}},"
            "\"6\":{\"7\":{},\"8\":{},\"9\":{}}}");
        grind(
            "{\"1\":{},\"2\":[],\"3\":\"x\",\"4\":1,"
            "\"5\":-1,\"6\":1.0,\"7\":false,\"8\":null}");

        // big strings
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
    }

    void
    testArrays()
    {
        grind("[]");
        grind("[[]]");
        grind("[[],[]]");
        grind("[[],[],[]]");
        grind("[[[]],[[],[]],[[],[],[]]]");
        grind("[{},[],\"x\",1,-1,1.0,true,null]");
    }

    void
    testStrings()
    {
        grind("\"\"");
        grind("\"x\"");
        grind("\"\\\"\"");
        grind("\"\\\\\"");
        grind("\"\\/\"");
        grind("\"\\b\"");
        grind("\"\\f\"");
        grind("\"\\n\"");
        grind("\"\\r\"");
        grind("\"\\t\"");
        grind("\"\\u0000\"");
        grind("\"xxxxxxxxxx\"");
        grind("\"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\"");
        grind("\""
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
            "\"");

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
                if(BEAST_EXPECTS(! ec,
                    ec.message()))
                {
                    p.finish(js.data() + N,
                        js.size() - N, ec);
                    if(BEAST_EXPECTS(! ec,
                        ec.message()))
                        check_round_trip(
                            p.release(), js);
                }   
            }
        }
    }

    void
    testNumbers()
    {
        grind("-9223372036854775808"); // INT64_MIN
        grind("-999999999999999999");
        grind("-0");
        grind( "0");
        grind( "1");
        grind( "999999999999999999");
        grind( "9223372036854775807"); // INT64_MAX
        grind( "9223372036854775808");
        grind( "18446744073709551615"); // UINT64_MAX

        grind("-1e-10");
        grind( "1e-10");
        grind( "1e+10");
        grind( "1e+100");
    }

    void
    testBool()
    {
        grind("true");
        grind("false");
    }

    void
    testNull()
    {
        grind("null");
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
            BEAST_EXPECTS(
                ec == error::need_start,
                ec.message());
        }

        // destroy after start
        {
            parser p;
            p.start();
        }

        // release before done
        {
            parser p;
            BEAST_THROWS(
                p.release(),
                std::logic_error);
        }

        // reserve
        {
            parser p;
            p.reserve(16384);
            p.reserve(100);
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
                BEAST_EXPECTS(! ec, ec.message());
                check_round_trip(jv, js);
            }
            {
                error_code ec;
                auto jv = parse("xxx", ec);
                BEAST_EXPECT(ec);
                BEAST_EXPECT(jv.is_null());
            }
        }

        // parse(string_view, storage_ptr, error_code)
        {
            {
                error_code ec;
                scoped_storage<pool> sp;
                auto jv = parse(js, ec, sp);
                BEAST_EXPECTS(! ec, ec.message());
                check_round_trip(jv, js);
            }

            {
                error_code ec;
                scoped_storage<pool> sp;
                auto jv = parse("xxx", ec, sp);
                BEAST_EXPECT(ec);
                BEAST_EXPECT(jv.is_null());
            }
        }

        // parse(string_view)
        {
            {
                check_round_trip(
                    parse(js),
                    js);
            }

            {
                value jv;
                BEAST_THROWS(
                    jv = parse("{,"),
                    system_error);
            }
        }

        // parse(string_view, storage_ptr)
        {
            {
                scoped_storage<pool> sp;
                check_round_trip(parse(js, sp), js);
            }

            {
                scoped_storage<pool> sp;
                value jv;
                BEAST_THROWS(
                    jv = parse("xxx", sp),
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
        p.finish(in.data(), in.size(), ec);
        if(BEAST_EXPECTS(! ec, ec.message()))
        {
            BEAST_EXPECT(to_string(p.release()) ==
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

    // https://github.com/vinniefalco/json/issues/15
    void
    testIssue15()
    {
        BEAST_EXPECT(
            json::parse("{\"port\": 12345}")
                .as_object()
                .at("port")
                .as_int64() == 12345);
    }

    void
    run()
    {
        testObjects();
        testArrays();
        testStrings();
        testNumbers();
        testBool();
        testNull();
        testMembers();
        testFreeFunctions();
        testSampleJson();
        testIssue15();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,parser);

} // json
} // boost
