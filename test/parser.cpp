//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/parser.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/json/parser.hpp>
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
        parser p(sp);
        error_code ec;
        p.write(
            s.data(),
            s.size(),
            ec);
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
            to_string_test(jv1);
        auto jv2 =
            from_string_test(s2);
        if(! BEAST_EXPECT(equal(jv1, jv2)))
            log <<
                "  " << s1 << "\n" <<
                "  " << s2 << std::endl;
    }

    void
    testParse()
    {
        string_view const js =
            "{\"1\":{},\"2\":[],\"3\":\"x\",\"4\":1,"
            "\"5\":-1,\"6\":1.0,\"7\":false,\"8\":null}";

        // parse(value)
        {
            check_round_trip(
                parse(js),
                js);
        }

        // parse(value, storage_ptr)
        {
            check_round_trip(
                parse(js, storage_ptr{}),
                js);
        }

        // parse(value, error_code)
        {
            error_code ec;
            auto jv = parse(js, ec);
            BEAST_EXPECTS(! ec, ec.message());
            check_round_trip(jv, js);
        }

        // parse(value, storage_ptr, error_code)
        {
            error_code ec;
            auto jv = parse(js, storage_ptr{}, ec);
            BEAST_EXPECTS(! ec, ec.message());
            check_round_trip(jv, js);
        }
    }

    void
    legacyTests()
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
        p.write(in.data(), in.size(), ec);
        if(BEAST_EXPECTS(! ec, ec.message()))
        {
            BEAST_EXPECT(to_string_test(p.get()) ==
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
    grind(string_view s)
    {
        {
            auto const jv =
                from_string_test(s);
            check_round_trip(jv, s);
        }
#if 0
        fail_loop([&](storage_ptr const& sp)
        {
            auto const jv =
                from_string_test(s, sp);
            check_round_trip(jv, s);
        });
#endif
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
    run()
    {
        log <<
            "sizeof(parser) == " <<
            sizeof(parser) << "\n";

        testParse();
        testObjects();
        testArrays();
        //testStrings();
        testNumbers();
        testBool();
        testNull();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,parser);

} // json
} // boost
