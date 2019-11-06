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

namespace boost {
namespace json {

class parser_test : public beast::unit_test::suite
{
public:
    void
    testParser()
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
            std::stringstream ss;
            ss << p.get();
            BEAST_EXPECT(ss.str() ==
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
    testParse()
    {
        auto const check =
            [&](json::value const& jv)
            {
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(jv.as_object().find(
                    "a")->second.is_bool());
                BEAST_EXPECT(jv.as_object().find(
                    "b")->second.is_number());
                BEAST_EXPECT(jv.as_object().find(
                    "c")->second.is_string());
            };

        string_view js =
            "{\"a\":true,\"b\":1,\"c\":\"x\"}";

        // parse(value)
        {
            check(parse(js));
        }

        // parse(value, storage_ptr)
        {
            check(parse(js, storage_ptr{}));
        }

        // parse(value, error_code)
        {
            error_code ec;
            auto jv = parse(js, ec);
            BEAST_EXPECTS(! ec, ec.message());
            check(jv);
        }

        // parse(value, storage_ptr, error_code)
        {
            error_code ec;
            auto jv = parse(js, storage_ptr{}, ec);
            BEAST_EXPECTS(! ec, ec.message());
            check(jv);
        }
    }

    void
    run()
    {
#if 0
        log <<
            "sizeof(parser) == " <<
            sizeof(parser) << "\n";
        testParser();
        testParse();
#else
        error_code ec;
        auto jv = parse(
            "[\"abcdefg\",\"1\",\"2\",[\"a\", \"b\", \"c\"], \"d\"]"
        , ec);
        if(BEAST_EXPECTS(! ec, ec.message()))
            log << to_string(jv) << std::endl;

#endif
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,parser);

} // json
} // boost
