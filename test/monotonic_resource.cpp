//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/monotonic_resource.hpp>

#include <boost/json/parser.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

class pool_test
{
public:
    void
    testStorage()
    {
        auto jv = parse(
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
        , make_counted_resource<monotonic_resource>());
        BOOST_TEST_PASS();
    }

    void
    run()
    {
        testStorage();
    }
};

TEST_SUITE(pool_test, "boost.json.monotonic_resource");

} // json
} // boost
