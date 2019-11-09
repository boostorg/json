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

namespace boost {
namespace json {

static
void
print(
    std::ostream& os,
    json::value const& jv);

static
void
print(
    std::ostream& os,
    object const& obj)
{
    os << "{";
    for(auto it = obj.begin();
        it != obj.end(); ++it)
    {
        if(it != obj.begin())
            os << ",";
        os << "\"" << it->key() << "\":";
        print(os, it->value());
    }
    os << "}";
}

static
void
print(
    std::ostream& os,
    array const& arr)
{
    os << "[";
    for(auto it = arr.begin();
        it != arr.end(); ++it)
    {
        if(it != arr.begin())
            os << ",";
        print(os, *it);
    }
    os << "]";
}

static
void
print(
    std::ostream& os,
    json::value const& jv)
{
    switch(jv.kind())
    {
    case kind::object:
        print(os, jv.get_object());
        break;
    case kind::array:
        print(os, jv.get_array());
        break;
    case kind::string:
        os << "\"" << jv.get_string() << "\"";
        break;
    case kind::int64:
        os << jv.as_int64();
        break;
    case kind::uint64:
        os << jv.as_uint64();
        break;
    case kind::double_:
        os << jv.as_double();
        break;
    case kind::boolean:
        if(jv.as_bool())
            os << "true";
        else
            os << "false";
        break;
    case kind::null:
        os << "null";
        break;
    }
}

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
                    "a")->value().is_bool());
                BEAST_EXPECT(jv.as_object().find(
                    "b")->value().is_number());
                BEAST_EXPECT(jv.as_object().find(
                    "c")->value().is_string());
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
    testVectors()
    {
        parse_vectors pv;
        for(auto const& e : pv)
        {
            if(e.result != 'y')
                continue;
            auto const jv = parse(e.text);
            log << e.text << std::endl;
            print(log, jv);
            log << std::endl << std::endl;
        }
    }

    void
    run()
    {
        log <<
            "sizeof(parser) == " <<
            sizeof(parser) << "\n";
        testParser();
        testParse();
        testVectors();
#if 0
        error_code ec;
        auto jv = parse(
            "[\"abcdefg\",\"1\",\"2\",[\"a\", \"b\", \"c\"], \"d\"]"
            //"{ \"k\" : 1, \"j\" : [\"hello\"] }"
            //"{}"
            //"[1,2,[3,4],5,[6,7,8],9,true,false,null]"
            //"[]"
            //"1"
        , ec);
        if(BEAST_EXPECTS(! ec, ec.message()))
            log << to_string(jv) << std::endl;

#endif
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,parser);

} // json
} // boost
