//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2021 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include "test_suite.hpp"
#include "parse_into/common.hpp"

#include <boost/json/parse_into.hpp>

#include <map>

namespace boost {
namespace json {

class parse_into_maps_test
{
public:
    void
    run()
    {
        testParseInto< std::map<std::string, int> >( {} );
        testParseInto< std::map<std::string, int> >( { { "one", 1 }, { "two", 2 } } );

        testParseInto< std::map<std::string, int> >( {} );
        testParseInto< std::map<std::string, int> >( { { "one", 1 }, { "two", 2 } } );

        testParseInto< std::map<std::string, std::vector<int>> >( {} );
        testParseInto< std::map<std::string, std::vector<int>> >( { { "one", { 1 } }, { "two", { 2, 3 } } } );

        testParseInto< std::map<std::string, std::map<std::string, int>> >( {} );
        testParseInto< std::map<std::string, std::map<std::string, int>> >( { { "one", {} }, { "two", { { "1", 1 }, { "2", 2 } } } } );

        testParseIntoErrors< std::map<std::string, int> >(
            error::not_object, { "1", 1, "2", 2} );
        testParseIntoErrors< std::map<std::string, std::map<std::string, int>> >(
            error::not_object, { {"1", {}}, {"2", {"3", 4}} } );

        std::map<std::string, int> m;
        parse_into(m, R"( {"1": 1, "2": 2, "3": 3} )");
        BOOST_TEST( m.size() == 3 );

        parse_into(m, R"( {"4": 4, "5": 5} )");
        BOOST_TEST( m.size() == 2 );
    }
};

TEST_SUITE(parse_into_maps_test, "boost.json.parse_into.maps");

} // namespace json
} // namespace boost
