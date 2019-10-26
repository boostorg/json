//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/ieee_decimal.hpp>
#include <boost/json/detail/ieee_parser.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <type_traits>

namespace boost {
namespace json {

class ieee_decimal_test : public beast::unit_test::suite
{
public:
    void run() override
    {
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,ieee_decimal);

} // json
} // boost
