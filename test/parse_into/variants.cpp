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

#include <array>
#include <map>
#include <string>
#include <vector>

namespace boost {
namespace json {

class parse_into_variants_test
{
public:
    template< template<class...> class Variant, class Monostate >
    void testVariant()
    {
        testParseInto< Variant<int> >( 1 );
        testParseInto< Variant<int, std::string> >( 1 );
        testParseInto< Variant<int, std::string> >( "qwerty" );
        testParseInto< Variant<Monostate, int, std::string> >( {} );
        testParseInto< Variant<Monostate, int, std::string> >( 1 );
        testParseInto< Variant<Monostate, int, std::string> >( "qwerty" );
        testParseInto< Variant<bool, std::uint64_t> >( true );
        testParseInto< Variant<bool, std::uint64_t> >( UINT64_MAX );

        testParseInto< Variant< std::vector<int> > >(
            std::vector<int>{1, 2, 3, 4, 5} );
        testParseInto< Variant< Monostate, std::vector<int> > >(
            std::vector<int>{1, 2, 3, 4, 5} );

        testParseInto< std::vector< Variant<int, std::string> > >(
            {1, 2, 3, "four", 5, "six", "seven", 8});

        // an array-like first alternative nested in a composite
        testParseInto< std::vector< Variant<std::vector<int>, int> > >(
            { std::vector<int>{1}, 2 } );
        testParseInto< std::map<std::string, Variant<std::vector<int>, int> > >(
            { { "a", std::vector<int>{1, 2} }, { "b", 3 } } );

        // an incomplete array must not satisfy a fixed-size alternative
        testParseIntoErrors< Variant< std::array<int, 3> > >(
            error::exhausted_variants, {1, 2} );

        using V = Variant<
            std::vector< int >,
            std::tuple< int, std::string, std::map<std::string, int> >,
            std::tuple< int, std::string, std::map<std::string, double> > >;
        testParseInto< V >(
            std::make_tuple(
                5,
                "five",
                std::map<std::string, double>{ {"one", 1}, {"pi", 3.14} }));

        testParseIntoErrors< Variant<Monostate> >(
            error::exhausted_variants, "a" );
        testParseIntoErrors< Variant<int> >(
            error::exhausted_variants, "a" );
        testParseIntoErrors< Variant<Monostate, int, bool> >(
            error::exhausted_variants, "a" );
    }

    void run()
    {
        testVariant<variant2::variant, variant2::monostate>();
#ifndef BOOST_NO_CXX17_HDR_VARIANT
        testVariant<std::variant, std::monostate>();
#endif
    }
};

TEST_SUITE(parse_into_variants_test, "boost.json.parse_into.variants");

} // namespace boost
} // namespace json
