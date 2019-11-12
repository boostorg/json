//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

namespace boost {
namespace json {

struct zjson_test : public beast::unit_test::suite
{
    void
    run() override
    {
        log <<
            "sizeof()\n"
            "  object        == " << sizeof(object) << "\n"
            "    value_type  == " << sizeof(object::value_type) << "\n"
            "  array         == " << sizeof(array) << "\n"
            "  string        == " << sizeof(string) << "\n"
            "  value         == " << sizeof(value) << "\n"
            "  serializer    == " << sizeof(serializer) << "\n"
            "  number_parser == " << sizeof(detail::number_parser) << "\n"
            "  basic_parser  == " << sizeof(basic_parser) << "\n"
            "  parser        == " << sizeof(parser) << "\n"
            << std::endl;
            ;
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,zjson);

} // json
} // boost
