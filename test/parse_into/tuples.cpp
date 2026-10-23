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

#include <boost/json/error.hpp>

#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace boost {
namespace json {

class parse_into_tuples_test
{
public:
    void run()
    {
        testParseInto<std::pair<std::nullptr_t, std::uint64_t>>(
            {nullptr, UINT64_MAX} );

        testParseInto<std::tuple<int, float, std::string>>( {} );
        testParseInto<std::tuple<int, float, std::string>>( std::make_tuple(1, 3.14f, "hello") );

        testParseInto<std::vector<std::pair<int, int>>>( {} );
        testParseInto<std::vector<std::pair<int, int>>>( { { 1, 2 }, { 3, 4 } } );

        testParseInto<std::vector<std::vector<std::pair<int, int>>>>( {} );
        testParseInto<std::vector<std::vector<std::pair<int, int>>>>( { { { 1, 2 }, { 3, 4 } } } );
        testParseInto<std::vector<std::vector<std::pair<int, int>>>>( { { { 1, 2 }, { 3, 4 } }, { { 5, 6 }, { 7, 8 } } } );

        testParseInto<std::map<std::string, std::vector<std::pair<int, int>>>>( {} );
        testParseInto<std::map<std::string, std::vector<std::pair<int, int>>>>( { { "one", {} } } );
        testParseInto<std::map<std::string, std::vector<std::pair<int, int>>>>( { { "one", { { 1, 2 }, { 3, 4 } } } } );

        testParseInto<std::pair<std::vector<int>, std::map<std::string, std::pair<int, bool>>>>( {} );
        testParseInto<std::pair<std::vector<int>, std::map<std::string, std::pair<int, bool>>>>( { { 1, 2, 3 }, { { "one", { 7, true } } } } );

        testParseIntoErrors< std::pair<int, int> >( error::not_array, 1 );
        testParseIntoErrors< std::pair<int, int> >(
            error::size_mismatch, {1, 2, 3} );
        testParseIntoErrors< std::tuple<int, int, int> >(
            error::size_mismatch, {1, 2} );
        testParseIntoErrors< std::tuple<std::vector<int>>  >(
            error::size_mismatch, {{1,2}, {3,4}} );
        testParseIntoErrors<std::map<std::string, std::tuple<int, int>>>(
            error::size_mismatch, { {"tup", array()} });
    }
};

TEST_SUITE(parse_into_tuples_test, "boost.json.parse_into.tuples");

} // namespace json
} // namespace boost
