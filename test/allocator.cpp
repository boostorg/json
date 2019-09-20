//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/allocator.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <vector>

namespace boost {
namespace json {

class allocator_test : public beast::unit_test::suite
{
public:
    void
    testAllocator()
    {
        auto a1 = make_storage_ptr(
            std::allocator<void>{});
        auto a2 = a1;
        auto a3 = std::move(a2);
        a2 = a1;
        a3 = std::move(a2);

        std::vector<char, allocator<char>> v(
            allocator<char>(make_storage_ptr(
                std::allocator<void>{})));
        v.resize(100);
        BEAST_EXPECT(v.capacity() >= 100);
    }

    void
    run()
    {
        testAllocator();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,allocator);

} // json
} // boost
