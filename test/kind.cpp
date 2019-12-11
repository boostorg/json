//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/kind.hpp>

#include <type_traits>

#include "test_suite.hpp"

namespace boost {
namespace json {

class kind_test
{
public:
    BOOST_STATIC_ASSERT(
        std::is_enum<kind>::value);

    void
    testEnum()
    {
        BOOST_TEST_PASS();
    }

    void run()
    {
        testEnum();
    }
};

TEST_SUITE(kind_test, "boost.json.kind");

} // json
} // boost
