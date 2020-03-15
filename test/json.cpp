//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json.hpp>

#include "test_suite.hpp"

namespace boost {

struct json_test
{
    ::test_suite::log_type log;

    void
    run()
    {
        using namespace json;
        log <<
            "sizeof(alignof)\n"
            "  object        == " << sizeof(object) << " (" << alignof(object) << ")\n"
            "    value_type  == " << sizeof(object::value_type) << " (" << alignof(object::value_type) << ")\n"
            "  array         == " << sizeof(array) << " (" << alignof(array) << ")\n"
            "  string        == " << sizeof(string) << " (" << alignof(string) << ")\n"
            "  value         == " << sizeof(value) << " (" << alignof(value) << ")\n"
            "  serializer    == " << sizeof(serializer) << "\n"
            "  basic_parser  == " << sizeof(basic_parser) << "\n"
            "  parser        == " << sizeof(parser)
            ;
        BOOST_TEST_PASS();
    }
};

TEST_SUITE(json_test, "boost.json.zsizes");

} // boost
