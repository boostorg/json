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

namespace boost {
namespace json {
namespace detail {

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
            [&](detail::number num)
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
            [&](detail::number num)
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
                [&](detail::number num1)
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
    // Requires `s` does not fit in an integral type.
    template<class F>
    void
    fcheck(std::string const& s, F const& f)
    {
        char* str_end;
        double const need =
            std::strtod(s.c_str(), &str_end);
        BOOST_TEST(str_end == &s.back() + 1);
        double const got = f(s);
        if(! BOOST_TEST(got == need))
        {
            std::uint64_t uneed;
            std::uint64_t ugot;
            std::memcpy(&uneed, &need, sizeof(need));
            std::memcpy(&ugot, &got, sizeof(got));
            log << 
                std::hex <<
                std::setprecision(
                    std::numeric_limits<
                        double>::max_digits10) <<
                f.name() << "\n"
                "string: " << s << "\n"
                "need  : " << need << " (0x" << uneed << ")\n"
                "got   : " << got << " (0x" << ugot << ")" <<
                std::endl;
        }
    }

    template<class F>
    void
    check_numbers(F const& f)
    {
        auto const fc =
            [&f, this](std::string const& s)
            {
                fcheck(s, f);
            };

        fc( "-999999999999999999999" );
        fc( "-100000000000000000009");
        fc( "-10000000000000000000" );
        fc( "-9223372036854775809" );

        fc( "18446744073709551616" );
        fc( "99999999999999999999" );
        fc( "999999999999999999999" );
        fc( "1000000000000000000000" );
        fc( "9999999999999999999999" );
        fc( "99999999999999999999999" );

        fc("-0.9999999999999999999999" );
        fc("-0.9999999999999999" );
        fc("-0.9007199254740991" );
        fc("-0.999999999999999" );
        fc("-0.99999999999999" );
        fc("-0.9999999999999" );
        fc("-0.999999999999" );
        fc("-0.99999999999" );
        fc("-0.9999999999" );
        fc("-0.999999999" );
        fc("-0.99999999" );
        fc("-0.9999999" );
        fc("-0.999999" );
        fc("-0.99999" );
        fc("-0.9999" );
        fc("-0.8125" );
        fc("-0.999" );
        fc("-0.99" );
        fc("-1.0" );
        fc("-0.9" );
        fc("-0.0" );
        fc( "0.0" );
        fc( "0.9" );
        fc( "0.99" );
        fc( "0.999" );
        fc( "0.8125" );
        fc( "0.9999" );
        fc( "0.99999" );
        fc( "0.999999" );
        fc( "0.9999999" );
        fc( "0.99999999" );
        fc( "0.999999999" );
        fc( "0.9999999999" );
        fc( "0.99999999999" );
        fc( "0.999999999999" );
        fc( "0.9999999999999" );
        fc( "0.99999999999999" );
        fc( "0.999999999999999" );
        fc( "0.9007199254740991" );
        fc( "0.9999999999999999" );
        fc( "0.9999999999999999999999" );
        fc( "0.999999999999999999999999999" );

        fc("-1e308" );
        fc("-1e-308" );
        fc("-9999e300" );
        fc("-999e100" );
        fc("-99e10" );
        fc("-9e1" );
        fc( "9e1" );
        fc( "99e10" );
        fc( "999e100" );
        fc( "9999e300" );
        fc( "999999999999999999.0" );
        fc( "999999999999999999999.0" );
        fc( "999999999999999999999e5" );
        fc( "999999999999999999999.0e5" );

        fc( "0.00000000000000001" );

        fc("-1e-1" );
        fc("-1e0" );
        fc("-1e1" );
        fc( "0e0" );
        fc( "1e0" );
        fc( "1e10" );

        fc(
            "0."
            "00000000000000000000000000000000000000000000000000" // 50 zeroes
            "1e50" );

        fc(
            "0."
            "00000000000000000000000000000000000000000000000000" // 50 zeroes
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000"
            "1e600" );
    }

    void
    testDoubles()
    {
        check_numbers(f_boost{});
    }

    void
    run()
    {
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
