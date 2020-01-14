//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/detail/number.hpp>

#include <cstdlib>
#include <iomanip>
#include <type_traits>

#include "test_suite.hpp"

#define ACCURATE_CONVERSION 0

namespace boost {
namespace json {
namespace detail {

bool
operator==(
    number const& lhs,
    number const& rhs) noexcept
{
    if(lhs.kind != rhs.kind)
        return false;
    switch(lhs.kind)
    {
    case json::kind::int64:
        return lhs.i == rhs.i;
    case json::kind::uint64:
        return lhs.u == rhs.u;
    default:
        break;
    }
    return
        std::signbit(lhs.d) ==
            std::signbit(rhs.d) &&
        lhs.d == rhs.d;
}

struct double_diagnoser
{

    static void emit_hex(std::ostream& os, double d)
    {
        std::uint64_t binary;
        static_assert(sizeof(binary) == sizeof(d), "");

        std::memcpy(&binary, &d, sizeof(d));
        auto oldflags = os.flags();

        try
        {
            os << std::hex << std::setw(16) << std::setfill('0') << binary;
            os.flags(oldflags);
        }
        catch(...)
        {
            os.flags(oldflags);
        }
    }
    static void emit_pow2(std::ostream& os, double d)
    {
        auto oldflags = os.flags();
        try
        {
            int exponent = 0;
            auto mantissa = std::frexp(d, &exponent);
            os << std::fixed << mantissa;
            os << " *2^ ";
            os << exponent;
            os.flags(oldflags);
        }
        catch(...)
        {
            os.flags(oldflags);
            throw;
        }
    }

    static void emit_scientific(std::ostream& os, double d)
    {
        auto oldflags = os.flags();
        try
        {
            os
            << std::setprecision(std::numeric_limits<double>::max_digits10)
            << d;
            os.flags(oldflags);
        }
        catch(...)
        {
            os.flags(oldflags);
            throw;
        }
    }

    friend
    std::ostream&
    operator<<(std::ostream& os, double_diagnoser diag)
    {
        emit_scientific(os, diag.d);
        os << "(0x";
        emit_hex(os, diag.d);
        os << ") ";
        emit_pow2(os, diag.d);
        return os;
    }

    double d;
};

auto diagnose(double d) -> double_diagnoser
{
    return double_diagnoser { d };
}

bool
are_close(
    double x,
    double y)
{
    std::uint64_t bx, by;
    std::memcpy(&bx, &x, sizeof(x));
    std::memcpy(&by, &y, sizeof(y));

    auto diff = bx - by;
    switch (diff)
    {
    case 0:
    case 1:
    case 0xffffffffffffffff:
        return true;
    default:
        break;
    }
    return false;
}


class number_test
{
public:
    test_suite::log_type log;

    template<class F>
    static
    void
    grind(
        string_view s,
        F&& f)
    {
        {
            error_code ec;
            number_parser p;
            p.write(s.data(), s.size(), ec);
            if(BOOST_TEST(! ec))
            {
                BOOST_TEST(p.is_done());
                f(p.get());
            }
        }
        for(;;)
        {
            error_code ec;
            number_parser p;
            p.write_some(s.data(), s.size(), ec);
            if(! BOOST_TEST(! ec))
                break;
            auto n = p.write_some("x", 1, ec);
            if(! BOOST_TEST(
                p.is_done()))
                break;
            if(! BOOST_TEST(n == 0))
                break;
            f(p.get());
            p.finish(ec);
            BOOST_TEST(! ec);
            break;
        }
        for(size_t i = 1; i < s.size(); ++i)
        {
            error_code ec;
            number_parser p;
            p.write_some(s.data(), i, ec);
            if(! BOOST_TEST(! ec))
                continue;
            p.write(
                s.data() + i, s.size() - i, ec);
            if(! BOOST_TEST(! ec))
                continue;
            BOOST_TEST(p.is_done());
            f(p.get());
        }
    }

    //------------------------------------------------------

    void
    testMembers()
    {
        // maybe_init
        {
            number_parser p;
            BOOST_TEST(! p.maybe_init(0));
            BOOST_TEST(! p.maybe_init('A'));
            BOOST_TEST(! p.maybe_init('a'));
            BOOST_TEST(! p.maybe_init('.'));
            BOOST_TEST(! p.maybe_init('!'));
            BOOST_TEST(! p.maybe_init(' '));
            BOOST_TEST(p.maybe_init('0')); p.reset();
            BOOST_TEST(p.maybe_init('1')); p.reset();
            BOOST_TEST(p.maybe_init('2')); p.reset();
            BOOST_TEST(p.maybe_init('3')); p.reset();
            BOOST_TEST(p.maybe_init('4')); p.reset();
            BOOST_TEST(p.maybe_init('5')); p.reset();
            BOOST_TEST(p.maybe_init('6')); p.reset();
            BOOST_TEST(p.maybe_init('7')); p.reset();
            BOOST_TEST(p.maybe_init('8')); p.reset();
            BOOST_TEST(p.maybe_init('9')); p.reset();
            BOOST_TEST(p.maybe_init('0')); p.reset();
            BOOST_TEST(p.maybe_init('-')); p.reset();
        }

        // finish
        {
            error_code ec;
            number_parser p;
            p.write_some("0x", 2, ec);
            if(BOOST_TEST(! ec))
            {
                p.finish(ec);
                BOOST_TEST(! ec);
            }
        }
    }

    //------------------------------------------------------

    void
    check_bad(string_view s)
    {
        error_code xec;
        {
            number_parser p;
            p.write(s.data(), s.size(), xec);
            BOOST_TEST(xec);
        }
        for(size_t i = 1; i < s.size(); ++i)
        {
            error_code ec;
            number_parser p;
            p.write_some(s.data(), i, ec);
            if(ec == xec)
            {
                BOOST_TEST_PASS();
                continue;
            }
            if(! BOOST_TEST(! ec))
                continue;
            p.write(
                s.data() + i, s.size() - i, ec);
            BOOST_TEST(ec == xec);
        }
    }

    void
    check_int64(
        string_view s,
        int64_t i)
    {
        grind(s,
            [&](number num)
            {
                if( BOOST_TEST(
                        num.kind == kind::int64))
                    BOOST_TEST(num.i == i);
            });
    }

    void
    check_uint64(
        string_view s,
        uint64_t u)
    {
        grind(s,
            [&](number num)
            {
                if( BOOST_TEST(
                        num.kind == kind::uint64))
                    BOOST_TEST(num.u == u);
            });
    }

    void
    testIntegers()
    {
        check_int64( "-9223372036854775808", INT64_MIN);
        check_int64( "-9223372036854775807", -9223372036854775807);
        check_int64( "-999999999999999999", -999999999999999999);
        check_int64( "-99999999999999999", -99999999999999999);
        check_int64( "-9999999999999999", -9999999999999999);
        check_int64( "-999999999999999", -999999999999999);
        check_int64( "-99999999999999", -99999999999999);
        check_int64( "-9999999999999", -9999999999999);
        check_int64( "-999999999999", -999999999999);
        check_int64( "-99999999999", -99999999999);
        check_int64( "-9999999999", -9999999999);
        check_int64( "-999999999", -999999999);
        check_int64( "-99999999", -99999999);
        check_int64( "-9999999", -9999999);
        check_int64( "-999999", -999999);
        check_int64( "-99999", -99999);
        check_int64( "-9999", -9999);
        check_int64( "-999", -999);
        check_int64( "-99", -99);
        check_int64( "-9", -9);
        check_int64(  "0", 0);
        check_int64(  "9", 9);
        check_int64(  "99", 99);
        check_int64(  "999", 999);
        check_int64(  "9999", 9999);
        check_int64(  "99999", 99999);
        check_int64(  "999999", 999999);
        check_int64(  "9999999", 9999999);
        check_int64(  "99999999", 99999999);
        check_int64(  "999999999", 999999999);
        check_int64(  "9999999999", 9999999999);
        check_int64(  "99999999999", 99999999999);
        check_int64(  "999999999999", 999999999999);
        check_int64(  "9999999999999", 9999999999999);
        check_int64(  "99999999999999", 99999999999999);
        check_int64(  "999999999999999", 999999999999999);
        check_int64(  "9999999999999999", 9999999999999999);
        check_int64(  "99999999999999999", 99999999999999999);
        check_int64(  "999999999999999999", 999999999999999999);
        check_int64(  "9223372036854775807", INT64_MAX);

        check_uint64( "9223372036854775808", 9223372036854775808ULL);
        check_uint64( "9999999999999999999", 9999999999999999999ULL);
        check_uint64( "18446744073709551615", UINT64_MAX);
    }

    void
    testBad()
    {
        check_bad("");
        check_bad("x");
        check_bad("e");
        check_bad("1ex");
        check_bad("-");
        check_bad("1a");
        check_bad(".");
        check_bad("-.");
        check_bad("1.");
        check_bad("-1.");
        check_bad("1.x");
        check_bad("1+");
        check_bad("1-");
        check_bad("0.0+");
        check_bad("0.0e+");
        check_bad("0.0e-");
        check_bad("0.0e0-");
        check_bad("0.0e");
        check_bad("0.e1");
        check_bad("-e");
        check_bad("-x");
        check_bad("2.e+3");
        check_bad("-2.e+3");

        // leading 0 must be followed by [.eE] or nothing
        check_bad( "00");
        check_bad( "01");
        check_bad( "00.");
        check_bad( "00.0");
        check_bad("-00");
        check_bad("-01");
        check_bad("-00.");
        check_bad("-00.0");
    }

    //------------------------------------------------------

    struct f_boost
    {
        static
        string_view
        name() noexcept
        {
            return "boost";
        }

        double
        operator()(string_view s) const
        {
            error_code ec;
            number_parser p;
            p.write(s.data(), s.size(), ec);
            if(ec)
                BOOST_THROW_EXCEPTION(
                    system_error(ec));
            BOOST_TEST(p.is_done());
            auto const num = p.get();
            BOOST_ASSERT(
                num.kind == kind::double_);

            grind(s,
                [&](number num1)
                {
                    if( BOOST_TEST(
                            num1.kind == kind::double_))
                        BOOST_TEST(num1.d == num.d);
                });

            return num.d;
        }
    };

    // Verify that f converts to the
    // same double produced by `strtod`.
    // Requires `s` is not represented by an integral type.
    template<class F>
    void
    fcheck(std::string const& s, F const& f)
    {
        char* str_end;
        double const need =
            std::strtod(s.c_str(), &str_end);
        BOOST_TEST(str_end == &s.back() + 1);
        double const got = f(s);
        auto same = got == need;

#if !ACCURATE_CONVERSION
        auto close = same ? true : are_close(got, need);
        if(! BOOST_TEST(close))
        {
            log << "not close : " <<
                f.name() << "\n"
                "string: " << s << "\n"
                "need  : " << diagnose(need) << "\n"
                "got   : " << diagnose(got) <<
                std::endl;
        }
#else
        if (!BOOST_TEST(same))
        {
            log << "close but not close enough : " <<
                f.name() << "\n"
                "string: " << s << "\n"
                "need  : " << diagnose(need) << "\n"
                "got   : " << diagnose(got) <<
                std::endl;
        }
#endif
    }

    template<class F>
    void
    check_numbers(F const& f)
    {
        auto const fc =
            [&](std::string const& s)
            {
                fcheck(s, f);
            };

        fc("-999999999999999999999");
        fc("-100000000000000000009");
        fc("-10000000000000000000");
        fc("-9223372036854775809");

        fc("18446744073709551616");
        fc("99999999999999999999");
        fc("999999999999999999999");
        fc("1000000000000000000000");
        fc("9999999999999999999999");
        fc("99999999999999999999999");

        fc("-0.9999999999999999999999");
        fc("-0.9999999999999999");
        fc("-0.9007199254740991");
        fc("-0.999999999999999");
        fc("-0.99999999999999");
        fc("-0.9999999999999");
        fc("-0.999999999999");
        fc("-0.99999999999");
        fc("-0.9999999999");
        fc("-0.999999999");
        fc("-0.99999999");
        fc("-0.9999999");
        fc("-0.999999");
        fc("-0.99999");
        fc("-0.9999");
        fc("-0.8125");
        fc("-0.999");
        fc("-0.99");
        fc("-1.0");
        fc("-0.9");
        fc("-0.0");
        fc("0.0");
        fc("0.9");
        fc("0.99");
        fc("0.999");
        fc("0.8125");
        fc("0.9999");
        fc("0.99999");
        fc("0.999999");
        fc("0.9999999");
        fc("0.99999999");
        fc("0.999999999");
        fc("0.9999999999");
        fc("0.99999999999");
        fc("0.999999999999");
        fc("0.9999999999999");
        fc("0.99999999999999");
        fc("0.999999999999999");
        fc("0.9007199254740991");
        fc("0.9999999999999999");
        fc("0.9999999999999999999999");
        fc("0.999999999999999999999999999");

        fc("-1e308");
        fc("-1e-308");
        fc("-9999e300");
        fc("-999e100");
        fc("-99e10");
        fc("-9e1");
        fc("9e1");
        fc("99e10");
        fc("999e100");
        fc("9999e300");
        fc("999999999999999999.0");
        fc("999999999999999999999.0");
        fc("999999999999999999999e5");
        fc("999999999999999999999.0e5");

        fc("0.00000000000000001");

        fc("-1e-1");
        fc("-1e0");
        fc("-1e1");
        fc("0e0");
        fc("1e0");
        fc("1e10");

        fc("0."
           "00000000000000000000000000000000000000000000000000" // 50 zeroes
           "1e50");
        fc("-0."
           "00000000000000000000000000000000000000000000000000" // 50 zeroes
           "1e50");

        fc("0."
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000" // 500 zeroes
           "1e600");
        fc("-0."
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000" // 500 zeroes
           "1e600");

        fc("0e"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000" // 500 zeroes
        );
    }

    void
    testDoubles()
    {
        check_numbers(f_boost{});
    }

    static
    number
    int64_num(int64_t i) noexcept
    {
        number num;
        num.i = i;
        num.kind = kind::int64;
        return num;
    }

    static
    number
    uint64_num(uint64_t u) noexcept
    {
        number num;
        num.u = u;
        num.kind = kind::uint64;
        return num;
    }

    static
    number
    double_num(double d) noexcept
    {
        number num;
        num.d = d;
        num.kind = kind::double_;
        return num;
    }

    void
    testEdgeCases()
    {
        auto const parse =
            [&](string_view s)
            {
                error_code ec;
                number_parser p;
                p.write(s.data(), s.size(), ec);
                BOOST_TEST(! ec);
                return p.get();
            };

        BOOST_TEST(parse("-0.0") == double_num(-0.0));
        BOOST_TEST(parse("-0E0") == double_num(-0.0));
        BOOST_TEST(parse("-0") == int64_num(0));

        BOOST_TEST(parse("0") == int64_num(0));
        BOOST_TEST(parse("0.010") == double_num(0.01));
        BOOST_TEST(parse("-0.010") == double_num(-0.01));
        BOOST_TEST(parse("1.010") == double_num(1.01));
        BOOST_TEST(parse("-1.010") == double_num(-1.01));
    }

    void
    run()
    {
        testEdgeCases();
        testMembers();
        testIntegers();
        testBad();
        testDoubles();
    }
};

TEST_SUITE(number_test, "boost.json.detail.number");

} // detail
} // json
} // boost

#if 0

(for positive)
A. accumulate digits into unsigned u
    if(got('.'))
        if( have_dot )
            return error;
        dot_pos = pos
    else if(u > UINT64_MAX)
        goto state C
    else
        ++dig_;
        accumulate digit

(for negative)
B. accumulate digits into unsigned u
    if(got('.'))
        if( have_dot )
            return error;
        dot_pos = pos
    else if(u > abs(INT64_MIN))
        goto state C
    else
        ++dig_;
        accumulate digit

C. accumulate exponent offset
    if(got('e', 'E', '-', '+')
        ...
    else if(got('.'))
        if( have_dot )
            return error;
        dot_pos = pos
    else
        if( have_dot )
            // do nothing
        else
            ++dig_;

#endif
