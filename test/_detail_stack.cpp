//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/detail/stack.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <string>

namespace boost {
namespace json {
namespace detail {

class stack_test : public beast::unit_test::suite
{
public:
    void
    testStack()
    {
        stack<std::string, 1> s;
        BEAST_EXPECT(s.empty());
        BEAST_EXPECT(s.capacity() > 0);
        BEAST_EXPECT(s.size() == 0);
        s.emplace_front("1");
        BEAST_EXPECT(s.size() == 1);
        BEAST_EXPECT(s[0] == "1");
        s.emplace_front("2");
        BEAST_EXPECT(s.size() == 2);
        BEAST_EXPECT(s[0] == "2");
        s.pop();
        BEAST_EXPECT(s.size() == 1);
        BEAST_EXPECT(s[0] == "1");
        s.pop();
        BEAST_EXPECT(s.empty());
    }

    void run() override
    {
        testStack();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,stack);

} // detail
} // json
} // boost
