//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/value_cast.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

class value_cast_test
{
public:
    BOOST_STATIC_ASSERT(
        ! detail::has_value_cast_traits<short>::value);
    BOOST_STATIC_ASSERT(
        detail::has_direct_impl<short>::value);
    
    template<class T>
    void
    check(T t)
    {
        BOOST_TEST(value_cast<T>(value(t)) == t);
    }

    void
    testNumberCast()
    {
        check((short)-1);
        check((int)-2);
        check((long)-3);
        check((long long)-4);
        check((unsigned short)1);
        check((unsigned int)2);
        check((unsigned long)3);
        check((unsigned long long)4);
        check((float)1.5);
        check((double)2.5);
        check((long double)3.5);
        check(true);
    }

    void
    testJsonTypes()
    {
        value_cast<object>(value(object_kind));
        value_cast<array>(value(array_kind));
        value_cast<string>(value(string_kind));
    }

    void
    testGenerics()
    {
        check(std::string("test"));
    }

    void
    run()
    {
        testNumberCast();
        testJsonTypes();
        testGenerics();
    }
};

TEST_SUITE(value_cast_test, "boost.json.value_cast");

} // json
} // boost
