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

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <type_traits>

namespace boost {
namespace json {
namespace detail {

class number_test : public beast::unit_test::suite
{
public:
    template<class F>
    void
    grind(
        string_view s,
        F&& f)
    {
        {
            error_code ec;
            number_parser p;
            p.write(s.data(), s.size(), ec);
            if(BEAST_EXPECTS(! ec, ec.message()))
            {
                BEAST_EXPECT(p.is_done());
                f(p.get());
            }
        }
        for(;;)
        {
            error_code ec;
            number_parser p;
            p.write_some(s.data(), s.size(), ec);
            if(! BEAST_EXPECTS(
                ! ec, ec.message()))
                break;
            auto n = p.write_some("x", 1, ec);
            if(! BEAST_EXPECT(
                p.is_done()))
                break;
            if(! BEAST_EXPECT(n == 0))
                break;
            f(p.get());
            p.write_eof(ec);
            BEAST_EXPECTS(! ec, ec.message());
            break;
        }
        for(size_t i = 1; i < s.size(); ++i)
        {
            error_code ec;
            number_parser p;
            p.write_some(s.data(), i, ec);
            if(! BEAST_EXPECTS(! ec, ec.message()))
                continue;
            p.write(
                s.data() + i, s.size() - i, ec);
            if(! BEAST_EXPECTS(! ec, ec.message()))
                continue;
            BEAST_EXPECT(p.is_done());
            f(p.get());
        }
    }

    void
    check_bad(string_view s)
    {
        error_code xec;
        {
            number_parser p;
            p.write(s.data(), s.size(), xec);
            BEAST_EXPECT(xec);
        }
        for(size_t i = 1; i < s.size(); ++i)
        {
            error_code ec;
            number_parser p;
            p.write_some(s.data(), i, ec);
            if(ec == xec)
            {
                BEAST_PASS();
                continue;
            }
            if(! BEAST_EXPECTS(! ec, ec.message()))
                continue;
            p.write(
                s.data() + i, s.size() - i, ec);
            BEAST_EXPECT(ec == xec);
        }
    }

    void
    check_int64(
        string_view s,
        int64_t i)
    {
        grind(s,
            [&](detail::number num)
            {
                if( BEAST_EXPECT(
                        num.kind == kind::int64))
                    BEAST_EXPECT(num.i == i);
            });
    }

    void
    check_uint64(
        string_view s,
        uint64_t u)
    {
        grind(s,
            [&](detail::number num)
            {
                if( BEAST_EXPECT(
                        num.kind == kind::uint64))
                    BEAST_EXPECT(num.u == u);
            });
    }

    void
    check_double(
        string_view s,
        double d)
    {
        grind(s,
            [&](detail::number num)
            {
                if( BEAST_EXPECT(
                        num.kind == kind::double_))
                    BEAST_EXPECT(num.d == d);
            });
    }

    void
    testParse()
    {
        check_double("-999999999999999999999", -999999999999999999999.0);
        check_double("-100000000000000000009", -100000000000000000009.0);
        check_double("-10000000000000000000", -10000000000000000000.0);
        check_double("-9223372036854775809", -9223372036854775809.0);
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
        check_double( "18446744073709551616", 18446744073709551616.0);
        check_double( "99999999999999999999", 99999999999999999999.0);
        check_double( "999999999999999999999", 999999999999999999999.0);
        check_double( "1000000000000000000000", 1000000000000000000000.0);
        check_double( "9999999999999999999999", 9999999999999999999999.0);
        check_double( "99999999999999999999999", 99999999999999999999999.0);

        check_double("-0.9999999999999999999999", -1.0000000000000002);
        check_double("-0.9999999999999999", -1.0000000000000000);
        check_double("-0.9007199254740991", -0.9007199254740991); // (2^53-1)
        check_double("-0.999999999999999", -0.99999999999999911);
        check_double("-0.99999999999999", -0.99999999999999001);
        check_double("-0.9999999999999", -0.99999999999990008);
        check_double("-0.999999999999", -0.99999999999900002);
        check_double("-0.99999999999", -0.99999999998999989);
        check_double("-0.9999999999", -0.99999999989999999);
        check_double("-0.999999999", -0.99999999900000003);
        check_double("-0.99999999", -0.99999999000000006);
        check_double("-0.9999999", -0.99999989999999994);
        check_double("-0.999999", -0.999999);
        check_double("-0.99999", -0.99999);
        check_double("-0.9999", -0.9999);
        check_double("-0.8125", -0.8125);
        check_double("-0.999", -0.999);
        check_double("-0.99", -0.99);
        check_double("-1.0", -1);
        check_double("-0.9", -0.9);
        check_double("-0.0", 0);
        check_double( "0.0", 0);
        check_double( "0.9", 0.9);
        check_double( "0.99", 0.99);
        check_double( "0.999", 0.999);
        check_double( "0.8125", 0.8125);
        check_double( "0.9999", 0.9999);
        check_double( "0.99999", 0.99999);
        check_double( "0.999999", 0.999999);
        check_double( "0.9999999", 0.99999989999999994);
        check_double( "0.99999999", 0.99999999000000006);
        check_double( "0.999999999", 0.99999999900000003);
        check_double( "0.9999999999", 0.99999999989999999);
        check_double( "0.99999999999", 0.99999999998999989);
        check_double( "0.999999999999", 0.99999999999900002);
        check_double( "0.9999999999999", 0.99999999999990008);
        check_double( "0.99999999999999", 0.99999999999999001);
        check_double( "0.999999999999999", 0.99999999999999911);
        check_double( "0.9007199254740991", 0.9007199254740991); // (2^53-1)
        check_double( "0.9999999999999999", 1.0000000000000000);
        check_double( "0.9999999999999999999999", 1.0000000000000002);
        check_double( "0.999999999999999999999999999", 1.0000000000000002);

        check_double("-1e308", -1e308);
        check_double("-1e-308", -1e-308);
        check_double("-9999e300", -9999e300);
        check_double("-999e100", -999e100);
        check_double("-99e10", -99e10);
        check_double("-9e1", -9e1);
        check_double( "9e1", 9e1);
        check_double( "99e10", 99e10);
        check_double( "999e100", 999e100);
        check_double( "9999e300", 9999e300);
        check_double( "999999999999999999.0", 999999999999999999.0);
        check_double( "999999999999999999999.0", 999999999999999999999.0);
        check_double( "999999999999999999999e5", 999999999999999999999e5);
        check_double( "999999999999999999999.0e5", 999999999999999999999.0e5);

        check_double("-1e-1", -1e-1);
        check_double("-1e0", -1);
        check_double("-1e1", -1e1);
        check_double( "0e0", 0);
        check_double( "1e0", 1);
        check_double( "1e10", 1e10);

        check_bad("");
        check_bad("x");
        check_bad("00");
        check_bad("e");
        check_bad("1ex");
        check_bad("-");
        check_bad("00");
        check_bad("00.");
        check_bad("00.0");
        check_bad("1a");
        check_bad(".");
        check_bad("1.");
        check_bad("1.x");
        check_bad("1+");
        check_bad("0.0+");
        check_bad("0.0e+");
        check_bad("0.0e-");
        check_bad("0.0e0-");
        check_bad("0.0e");
        check_bad("-e");
        check_bad("-x");
    }

    void
    testMembers()
    {
        // maybe_init
        {
            number_parser p;
            BEAST_EXPECT(! p.maybe_init(0));
            BEAST_EXPECT(! p.maybe_init('A'));
            BEAST_EXPECT(! p.maybe_init('a'));
            BEAST_EXPECT(! p.maybe_init('.'));
            BEAST_EXPECT(! p.maybe_init('!'));
            BEAST_EXPECT(! p.maybe_init(' '));
            BEAST_EXPECT(p.maybe_init('0')); p.reset();
            BEAST_EXPECT(p.maybe_init('1')); p.reset();
            BEAST_EXPECT(p.maybe_init('2')); p.reset();
            BEAST_EXPECT(p.maybe_init('3')); p.reset();
            BEAST_EXPECT(p.maybe_init('4')); p.reset();
            BEAST_EXPECT(p.maybe_init('5')); p.reset();
            BEAST_EXPECT(p.maybe_init('6')); p.reset();
            BEAST_EXPECT(p.maybe_init('7')); p.reset();
            BEAST_EXPECT(p.maybe_init('8')); p.reset();
            BEAST_EXPECT(p.maybe_init('9')); p.reset();
            BEAST_EXPECT(p.maybe_init('0')); p.reset();
            BEAST_EXPECT(p.maybe_init('-')); p.reset();
        }

        // write_eof
        {
            error_code ec;
            number_parser p;
            p.write_some("0x", 2, ec);
            if(BEAST_EXPECTS(! ec,
                ec.message()))
            {
                p.write_eof(ec);
                BEAST_EXPECTS(! ec,
                    ec.message());
            }
        }
    }

    void
    run() override
    {
        testParse();
        testMembers();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,number);

} // detail
} // json
} // boost
