//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/value_builder.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

class value_builder_test
{
public:
    void
    testBuilder()
    {
        error_code ec;
        value_builder vb;
        vb.reset();
        vb.on_document_begin(ec);
        vb.on_null(ec);
        vb.on_document_end(ec);
        vb.release();
    }

    void
    run()
    {
        testBuilder();
    }
};

TEST_SUITE(value_builder_test, "boost.json.value_builder");

} // json
} // boost
