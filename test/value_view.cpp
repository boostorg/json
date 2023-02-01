//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/json/value_view.hpp>
#include <boost/describe/enum.hpp>

#include "test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace json {

BOOST_DEFINE_ENUM(foobar, foo, bar);

struct value_view_test
{
    void testString()
    {
        value_view v{foobar::foo};
        BOOST_TEST(v == "foo");

        v = foobar::bar;
        BOOST_TEST(v == "bar");

        BOOST_TEST_THROWS(value_view{static_cast<foobar>(124)}, std::invalid_argument);
    }
    void run()
    {
        testString();
    }
};


TEST_SUITE(value_view_test, "boost.json.value_view");

}
}
