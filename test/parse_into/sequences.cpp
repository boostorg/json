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

#include <boost/container/flat_set.hpp>
#include <boost/json/parse_into.hpp>

#include <array>
#include <map>

namespace boost {
namespace json {

class parse_into_sequences_test
{
public:
    void
    run()
    {
        testParseInto<std::vector<std::nullptr_t>>( { nullptr, nullptr } );

        testParseInto< std::vector<bool> >( {} );
        testParseInto< std::vector<bool> >( { true, false } );

        testParseInto< std::vector<int> >( {} );
        testParseInto< std::vector<int> >( { 1, 2, 3 } );
        testParseInto< std::vector<std::uint64_t> >( { 1, 2, UINT64_MAX } );

        testParseInto< std::vector<float> >( {} );
        testParseInto< std::vector<float> >( { 1.02f, 2.11f, 3.14f } );

        testParseInto< std::vector<std::string> >( {} );
        testParseInto< std::vector<std::string> >( { "one", "two", "three" } );

        testParseInto< std::vector<std::vector<int>> >( {} );
        testParseInto< std::vector<std::vector<int>> >( { {}, { 1 }, { 2, 3 }, { 4, 5, 6 } } );

        testParseInto< std::vector<std::map<std::string, int>> >(
            { {}, { {"1", 2}, {"3", 4} }, { { "5", 6 } } } );

        // clang <= 5 doesn't like when std::array is created from init-list
        std::array<int, 4> arr;
        arr.fill(17);
        testParseInto< std::array<int, 4> >( arr );

        testParseIntoErrors< std::vector<int> >( error::not_array, 1 );
        testParseIntoErrors< std::vector<char> >( error::not_array, "abcd" );
        testParseIntoErrors< std::vector<std::vector<int>> >(
            error::not_array, {1, 2, 3} );
        testParseIntoErrors< std::array<int, 4> >(
            error::size_mismatch, {1, 2, 3} );
        testParseIntoErrors< std::array<int, 4> >(
            error::size_mismatch, {1, 2, 3, 4, 5, 6, 7, 8} );

        testParseInto< std::vector<std::array<int, 4>> >( {arr,arr,arr} );

        std::vector<int> v;
        parse_into(v, "[1,2,3,4]");
        BOOST_TEST( v.size() == 4 );

        parse_into(v, "[5,6,7]");
        BOOST_TEST( v.size() == 3 );

        // containers that are filled through an insert iterator
        testParseInto< std::vector<container::flat_set<int>> >(
            { {}, { 1 }, { 2, 3 }, { 4, 5, 6 } } );

        container::flat_set<int> s = { 1, 2, 3, 4 };
        parse_into(s, "[5,6,7]");
        BOOST_TEST( s.size() == 3 );
    }
};

TEST_SUITE(parse_into_sequences_test, "boost.json.parse_into.sequences");

} // namespace json
} // namespace boost
