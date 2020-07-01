//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/number_cast.hpp>

#include <type_traits>
#include <limits.h>

#include "test_suite.hpp"

namespace boost {
namespace json {

namespace {

template<class T>
static
T max_of()
{
    return (std::numeric_limits<T>::max)();
}

template<class T>
static
T min_of()
{
    return (std::numeric_limits<T>::min)();
}

} // (anon)

class number_cast_test
{
public:
    void
    testNumberCast()
    {
#define EQAL(T) BOOST_TEST(number_cast<T>(jv) == V)
#define EQUS(T) BOOST_TEST((V >= 0) && number_cast<T>(jv) == static_cast<std::uint64_t>(V))
#define EQUF(T) BOOST_TEST(static_cast<float>(V) == static_cast<float>(number_cast<T>(jv)))
#define THRO(T) BOOST_TEST_THROWS(number_cast<T>(jv), system_error)

        BOOST_TEST_THROWS(number_cast<int>(value(object_kind)), system_error);
        BOOST_TEST_THROWS(number_cast<int>(value(array_kind)), system_error);
        BOOST_TEST_THROWS(number_cast<int>(value(string_kind)), system_error);
        BOOST_TEST_THROWS(number_cast<int>(value(false)), system_error);
        BOOST_TEST_THROWS(number_cast<int>(value(nullptr)), system_error);

        {
            unsigned char V = 0;
            value const jv(V);
            EQAL(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            EQAL(std::uint8_t);
            EQAL(std::uint16_t);
            EQAL(std::uint32_t);
            EQAL(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = max_of<std::int8_t>();
            value const jv(V);
            EQAL(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            EQAL(std::uint8_t);
            EQAL(std::uint16_t);
            EQUS(std::uint32_t);
            EQUS(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = max_of<std::int16_t>();
            value const jv(V);
            THRO(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            EQAL(std::uint16_t);
            EQUS(std::uint32_t);
            EQUS(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = max_of<std::int32_t>();
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            EQUS(std::uint32_t);
            EQUS(std::uint64_t);
            EQUF(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = max_of<std::int64_t>();
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            THRO(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            EQUS(std::uint64_t);
            EQUF(float);
            EQUF(double);
            EQUF(long double);
        }
        //---
        {
            auto V = max_of<std::uint8_t>();
            value const jv(V);
            THRO(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            EQAL(std::uint8_t);
            EQAL(std::uint16_t);
            EQAL(std::uint32_t);
            EQAL(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = max_of<std::uint16_t>();
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            EQAL(std::uint16_t);
            EQAL(std::uint32_t);
            EQAL(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = max_of<std::uint32_t>();
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            THRO(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            EQAL(std::uint32_t);
            EQAL(std::uint64_t);
            EQUF(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = max_of<std::uint64_t>();
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            THRO(std::int32_t);
            THRO(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            EQAL(std::uint64_t);
            EQUF(float);
            EQUF(double);
            EQUF(long double);
        }
        //---
        {
            auto V = min_of<std::int8_t>();
            value const jv(V);
            EQAL(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            THRO(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = min_of<std::int16_t>();
            value const jv(V);
            THRO(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            THRO(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = min_of<std::int32_t>();
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            THRO(std::uint64_t);
            EQUF(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = min_of<std::int64_t>();
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            THRO(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            THRO(std::uint64_t);
            EQUF(float);
            EQUF(double);
            EQUF(long double);
        }
        //---
        {
            auto V = double(1.5);
            value const jv(V);
            THRO(std::int8_t);
            THRO(std::int16_t);
            THRO(std::int32_t);
            THRO(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            THRO(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = double(2.0);
            value const jv(V);
            EQAL(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            EQAL(std::uint8_t);
            EQAL(std::uint16_t);
            EQAL(std::uint32_t);
            EQAL(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
        {
            auto V = double(-4.0);
            value const jv(V);
            EQAL(std::int8_t);
            EQAL(std::int16_t);
            EQAL(std::int32_t);
            EQAL(std::int64_t);
            THRO(std::uint8_t);
            THRO(std::uint16_t);
            THRO(std::uint32_t);
            THRO(std::uint64_t);
            EQAL(float);
            EQAL(double);
            EQAL(long double);
        }
    }

    void
    run()
    {
        testNumberCast();
    }
};

TEST_SUITE(number_cast_test, "boost.json.number_cast");

} // json
} // boost
