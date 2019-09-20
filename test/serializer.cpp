//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/serializer.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

namespace boost {
namespace json {

class serializer_test : public beast::unit_test::suite
{
public:
    void
    testSerializer()
    {
        value jv = {
          {"pi", 3.141},
          {"happy", true},
          {"name", "Niels"},
          {"nothing", nullptr},
          {"answer", {
            {"everything", 42}
          }},
#if 0
          {"list", {1, 0, 2}},
          {"object", {
            {"currency", "USD"},
            {"value", 42.99}
          }}
#endif
        };

        auto& arr = jv.as_object().
            emplace("arr", kind::array).first->second.as_array();
        arr.emplace_back(1);
        arr.emplace_back(2);
        arr.emplace_back(3);

        serializer sr(jv);
        char buf[2048];
        auto const n =
            sr.next({buf, sizeof(buf)});
        string_view s(buf, n);
        log << s << "\n";
    }

    void
    run()
    {
        testSerializer();
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,serializer);

} // json
} // boost
