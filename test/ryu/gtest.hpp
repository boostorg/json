//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef GTEST_HPP
#define GTEST_HPP

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/utility/string_view.hpp>

#define TEST(s1,s2) \
struct s2 ## s1 ## _test; \
BEAST_DEFINE_TESTSUITE(boost,Ryu,s2##s1); \
struct s2 ## s1 ## _test : ::boost::beast::unit_test::suite { \
  void run() override; }; void s2##s1##_test::run()

#define EXPECT_STREQ(s1, s2) BEAST_EXPECT(::boost::string_view(s1) == ::boost::string_view(s2))
#define ASSERT_STREQ(s1, s2) { auto const s1_ = (s1); auto const s2_ = (s2); \
    EXPECT_STREQ(s1_, s2_); }
#define ASSERT_EQ(e1, e2) BEAST_EXPECT((e1)==(e2))

#endif
