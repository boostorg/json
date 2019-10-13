//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/number.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

#include <cmath>

namespace boost {
namespace json {

class number_test : public beast::unit_test::suite
{
public:
    template<class I>
    void
    check(I v
        ,typename std::enable_if<
            std::is_signed<I>::value &&
            std::is_integral<I>::value
                >::type* = 0) noexcept
    {
        number n(v);
        number n2(0);
        n2 = n;
        BEAST_EXPECT(n == n2);
        BEAST_EXPECT(n.is_int64());
        BEAST_EXPECT(n.get_int64() == v);
        BEAST_EXPECT(n.get_double() == v);
    }

    template<class U>
    void
    check(U v
        ,typename std::enable_if<
            std::is_unsigned<U>::value
                >::type* = 0) noexcept
    {
        number n(v);
        number n2(0);
        n2 = n;
        BEAST_EXPECT(n == n2);
        BEAST_EXPECT(n.is_uint64());
        BEAST_EXPECT(number(static_cast<
            number::mantissa_type>(v),
                0, false) == n);
        BEAST_EXPECT(n.get_uint64() == v);
        BEAST_EXPECT(n.get_double() == v);
     }

    template<class F>
    void
    check(F v
        ,typename std::enable_if<
            std::is_floating_point<
                F>::value>::type* = 0) noexcept
    {
        number n(v);
        number n2(0);
        n2 = n;
        BEAST_EXPECT(n == n2);
        BEAST_EXPECT(static_cast<F>(
            n.get_double()) == v);
    }

    template<class F>
    void
    approx(F v) noexcept
    {
        number n(v);
        number n2(0);
        n2 = n;
        BEAST_EXPECT(n == n2);
        // VFALCO Unfortunately the results are not
        //        quite exact due to power of 10 conversion.
    #if 0
        auto v1 = n.get_double();
        auto const d = std::fabs(v1 - v);
        BEAST_EXPECT(static_cast<F>(
            n.get_double()) == v);
    #endif
    }

    void
    testConstruction()
    {
        // Only perform light checking on
        // the result of the construction.

        {
            number n((short)1);
            BEAST_EXPECT(n.get_int64() == 1);
        }
        {
            number n((int)2);
            BEAST_EXPECT(n.get_int64() == 2);
        }
        {
            number n((long)3);
            BEAST_EXPECT(n.get_int64() == 3);
        }
        {
            number n((long long)4);
            BEAST_EXPECT(n.get_int64() == 4);
        }
        {
            number n((unsigned short)1);
            BEAST_EXPECT(n.get_uint64() == 1);
        }
        {
            number n((unsigned int)2);
            BEAST_EXPECT(n.get_uint64() == 2);
        }
        {
            number n((unsigned long)3);
            BEAST_EXPECT(n.get_uint64() == 3);
        }
        {
            number n((unsigned long long)4);
            BEAST_EXPECT(n.get_uint64() == 4);
        }
        {
            number n((float)1.5f);
            BEAST_EXPECT(n.get_double() == 1.5);
        }
        {
            number n((double)2.5);
            BEAST_EXPECT(n.get_double() == 2.5);
        }
    }

    void
    testAssignment()
    {
        // Only perform light checking on
        // the result of the construction.

        {
            number n;
            n = (short)1;
            BEAST_EXPECT(n.get_int64() == 1);
            n = (int)2;
            BEAST_EXPECT(n.get_int64() == 2);
            n = (long)3;
            BEAST_EXPECT(n.get_int64() == 3);
            n = (long long)4;
            BEAST_EXPECT(n.get_int64() == 4);
            n = (unsigned short)5;
            BEAST_EXPECT(n.get_uint64() == 5);
            n = (unsigned int)6;
            BEAST_EXPECT(n.get_uint64() == 6);
            n = (unsigned long)7;
            BEAST_EXPECT(n.get_uint64() == 7);
            n = (unsigned long long)8;
            BEAST_EXPECT(n.get_uint64() == 8);
            n = (float)1.5;
            BEAST_EXPECT(n.get_double() == 1.5);
            n = (double)2.5;
            BEAST_EXPECT(n.get_double() == 2.5);
            n = (double)3.5l;
            BEAST_EXPECT(n.get_double() == 3.5);
        }
    }

    void
    testComparison()
    {
        // Only perform light checking on
        // the result of the comparison.

        number n = 1;
        BEAST_EXPECT(n == ((short)1));
        BEAST_EXPECT(n == ((int)1));
        BEAST_EXPECT(n == ((long)1));
        BEAST_EXPECT(n == ((long long)1));
        BEAST_EXPECT(n == ((unsigned short)1));
        BEAST_EXPECT(n == ((unsigned int)1));
        BEAST_EXPECT(n == ((unsigned long)1));
        BEAST_EXPECT(n == ((unsigned long long)1));

        n = 1.f;
        BEAST_EXPECT(n == 1.f);

        n = 1.;
        BEAST_EXPECT(n == 1.);
    }

    void
    testValues()
    {
        // Test the range of each convertible numeric
        // type, make sure they round-trip correctly.

        check(1.0/7);
        check((std::numeric_limits<short>::min)());
        check((std::numeric_limits<short>::max)());
        check((std::numeric_limits<short>::min)());
        check((std::numeric_limits<short>::max)());
        check((std::numeric_limits<int>::min)());
        check((std::numeric_limits<int>::max)());
        check((std::numeric_limits<int>::min)()/2);
        check((std::numeric_limits<int>::max)()/2);
        check((std::numeric_limits<long>::min)());
        check((std::numeric_limits<long>::max)());
        check((std::numeric_limits<long>::min)()/2);
        check((std::numeric_limits<long>::max)()/2);
        check((std::numeric_limits<long long>::min)());
        check((std::numeric_limits<long long>::max)());
        check((std::numeric_limits<long long>::min)()/2);
        check((std::numeric_limits<long long>::max)()/2);
        check((std::numeric_limits<unsigned short>::min)());
        check((std::numeric_limits<unsigned short>::max)());
        check((std::numeric_limits<unsigned short>::min)()/2);
        check((std::numeric_limits<unsigned short>::max)()/2);
        check((std::numeric_limits<unsigned int>::min)());
        check((std::numeric_limits<unsigned int>::max)());
        check((std::numeric_limits<unsigned int>::min)()/2);
        check((std::numeric_limits<unsigned int>::max)()/2);
        check((std::numeric_limits<unsigned long>::min)());
        check((std::numeric_limits<unsigned long>::max)());
        check((std::numeric_limits<unsigned long>::min)()/2);
        check((std::numeric_limits<unsigned long>::max)()/2);
        check((std::numeric_limits<unsigned long long>::min)());
        check((std::numeric_limits<unsigned long long>::max)());
        check((std::numeric_limits<unsigned long long>::min)()/2);
        check((std::numeric_limits<unsigned long long>::max)()/2);

        check((std::numeric_limits<float>::min)());
        check((std::numeric_limits<float>::max)());
        check((std::numeric_limits<float>::min)()/2);
        check((std::numeric_limits<float>::max)()/2);
        check((std::numeric_limits<double>::min)());
        check((std::numeric_limits<double>::max)());
        check((std::numeric_limits<double>::min)()/2);

        // not exact
        approx((std::numeric_limits<double>::max)()/2);
    }

    template<class V>
    void
    check_output(V v)
    {
        char buf[number::max_string_chars];
        auto const s = number(v).print(buf, sizeof(buf));
        BEAST_EXPECTS(
            s == std::to_string(v).c_str(),
            std::string(s.data(), s.size()));
    }

    void
    testOstream()
    {
        check_output(1);
        check_output(2);
        check_output(3);

        // These don't match std::to_string
        /*
        check_output(1.5f);
        check_output(2.5);
        check_output(3.5l);
        */

        check_output((std::numeric_limits<short>::min)());
        check_output((std::numeric_limits<short>::max)());
        check_output((std::numeric_limits<short>::min)());
        check_output((std::numeric_limits<short>::max)());
        check_output((std::numeric_limits<int>::min)());
        check_output((std::numeric_limits<int>::max)());
        check_output((std::numeric_limits<int>::min)()/2);
        check_output((std::numeric_limits<int>::max)()/2);
        check_output((std::numeric_limits<long>::min)());
        check_output((std::numeric_limits<long>::max)());
        check_output((std::numeric_limits<long>::min)()/2);
        check_output((std::numeric_limits<long>::max)()/2);
        check_output((std::numeric_limits<long long>::min)());
        check_output((std::numeric_limits<long long>::max)());
        check_output((std::numeric_limits<long long>::min)()/2);
        check_output((std::numeric_limits<long long>::max)()/2);
        check_output((std::numeric_limits<unsigned short>::min)());
        check_output((std::numeric_limits<unsigned short>::max)());
        check_output((std::numeric_limits<unsigned short>::min)()/2);
        check_output((std::numeric_limits<unsigned short>::max)()/2);
        check_output((std::numeric_limits<unsigned int>::min)());
        check_output((std::numeric_limits<unsigned int>::max)());
        check_output((std::numeric_limits<unsigned int>::min)()/2);
        check_output((std::numeric_limits<unsigned int>::max)()/2);
        check_output((std::numeric_limits<unsigned long>::min)());
        check_output((std::numeric_limits<unsigned long>::max)());
        check_output((std::numeric_limits<unsigned long>::min)()/2);
        check_output((std::numeric_limits<unsigned long>::max)()/2);
        check_output((std::numeric_limits<unsigned long long>::min)());
        check_output((std::numeric_limits<unsigned long long>::max)());
        check_output((std::numeric_limits<unsigned long long>::min)()/2);
        check_output((std::numeric_limits<unsigned long long>::max)()/2);
    }

    void
    run() override
    {
        testConstruction();
        testAssignment();
        testComparison();
        testValues();
        testOstream();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,number);

} // json
} // boost
