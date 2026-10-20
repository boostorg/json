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

#include <map>
#include <string>
#include <vector>

namespace boost {
namespace json {

class parse_into_optionals_test
{
public:
    void
    run()
    {
#ifndef BOOST_NO_CXX17_HDR_OPTIONAL
        testParseInto< std::optional<int> >( std::nullopt );
        testParseInto< std::optional<int> >( 1 );
        testParseInto< std::optional<std::uint64_t> >( ULONG_MAX );
        testParseInto< std::optional<bool> >( true );
        testParseInto< std::optional<double> >( 33.77 );

        testParseInto< std::optional<std::vector<std::nullptr_t>> >(
            std::nullopt );
        testParseInto< std::optional<std::vector<std::nullptr_t>> >(
           std::vector<std::nullptr_t>{} );
        testParseInto< std::optional<std::vector<std::nullptr_t>> >(
           std::vector<std::nullptr_t>{nullptr, nullptr} );

        testParseInto< std::optional<std::vector<std::string>> >(
           std::vector<std::string>{"1", "2", "3"} );

        testParseInto< std::optional< std::map<std::string, int> > >(
           std::map<std::string, int>{ {"1", 1}, {"2", 2}, {"3", 3} } );

        testParseInto< std::vector< std::optional<int> > >(
            {1, 2, 3, std::nullopt, 5, std::nullopt, std::nullopt, 8});
#endif
    }
};

TEST_SUITE(parse_into_optionals_test, "boost.json.parse_into.optionals");

} // namespace json
} // namespace boost
