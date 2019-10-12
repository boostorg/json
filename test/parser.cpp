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
    run()
    {
        log <<
            "sizeof(parser) == " <<
            sizeof(parser) << "\n";
        testParser();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,parser);

} // json
} // boost
