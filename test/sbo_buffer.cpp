//
// Copyright (c) 2026 Ramya Eliger (ramya@digiscrypt.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/detail/sbo_buffer.hpp>
#include <boost/json/string_view.hpp>

#include <string>

#include "test_suite.hpp"

namespace boost {
namespace json {

class sbo_buffer_test
{
public:
    void
    testInlineStorage()
    {
        detail::sbo_buffer<32> buf;
        std::size_t const cap = buf.capacity();
        BOOST_TEST(cap >= 32);

        char const* p = buf.append("1.2", 3);
        BOOST_TEST(buf.size() == 3);
        BOOST_TEST(string_view(p, buf.size()) == "1.2");
        BOOST_TEST(buf.capacity() == cap);

        p = buf.append("5e10", 4);
        BOOST_TEST(buf.size() == 7);
        BOOST_TEST(string_view(p, buf.size()) == "1.25e10");
        BOOST_TEST(buf.capacity() == cap);
    }

    void
    testReuse()
    {
        detail::sbo_buffer<32> buf;
        std::size_t const cap = buf.capacity();
        for(int i = 0; i < 40; ++i)
        {
            buf.clear();
            char const* p = buf.append("1.25", 4);
            BOOST_TEST(buf.size() == 4);
            BOOST_TEST(string_view(p, buf.size()) == "1.25");
        }
        BOOST_TEST(buf.capacity() == cap);
    }

    void
    testGrowth()
    {
        detail::sbo_buffer<32> buf;
        std::string const s(1000, 'x');

        char const* p = buf.append(s.data(), s.size());
        BOOST_TEST(buf.size() == s.size());
        BOOST_TEST(buf.capacity() >= s.size());
        BOOST_TEST(string_view(p, buf.size()) == s);

        std::size_t const cap = buf.capacity();
        buf.clear();
        p = buf.append("1.25", 4);
        BOOST_TEST(buf.size() == 4);
        BOOST_TEST(string_view(p, buf.size()) == "1.25");
        BOOST_TEST(buf.capacity() == cap);

        buf.reset();
        BOOST_TEST(buf.size() == 0);
        BOOST_TEST(buf.capacity() == 32);
    }

    void
    run()
    {
        testInlineStorage();
        testReuse();
        testGrowth();
    }
};

TEST_SUITE(sbo_buffer_test, "boost.json.sbo_buffer");

} // namespace json
} // namespace boost
