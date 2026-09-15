//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/stream_parser.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include <iostream>
#include <random>
#include <cinttypes>

#include "test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace json {

template<std::size_t N, class... Args>
void
sprintf(char (&buf)[N],
    char const* format, Args&&... args)
{
#ifdef _MSC_VER
    sprintf_s(buf, format,
        std::forward<Args>(args)...);
#else
    std::snprintf(buf, N, format,
        std::forward<Args>(args)...);
#endif
}

class double_test
{
public:
    ::test_suite::log_type log;

    struct f_boost
    {
        source_location const& loc;

        static
        string_view
        name() noexcept
        {
            return "boost";
        }

        double
        operator()(string_view s, parse_options const& po = {}) const
        {
            BOOST_TEST_CHECKPOINT();
            system::error_code ec;
            stream_parser p({}, po);
            p.write(s.data(), s.size(), ec);
            if(BOOST_TEST(! ec, loc))
                p.finish(ec);
            if(! BOOST_TEST(! ec, loc))
                return 0;
            auto const jv = p.release();
            double const d = jv.as_double();
            grind_double(s, d, po, loc);
            return d;
        }
    };

    bool
    within_1ulp(double x, double y)
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

    static
    value
    from_string_test(
        string_view s,
        storage_ptr sp,
        parse_options const& po,
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        stream_parser p(storage_ptr(), po);
        system::error_code ec;
        p.reset(std::move(sp));
        p.write(s.data(), s.size(), ec);
        if(BOOST_TEST(! ec, loc))
            p.finish(ec);
        BOOST_TEST(! ec, loc);
        return p.release();
    }

    static
    void
    check_round_trip(
        value const& jv1,
        parse_options const& po,
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        auto const s2 = serialize(jv1);
        auto jv2 = from_string_test(s2, {}, po, loc);
        BOOST_TEST(equal(jv1, jv2), loc);
    }

    template<class F>
    static
    void
    grind_one(
        string_view s,
        storage_ptr sp,
        F const& f,
        parse_options const& po,
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        auto const jv = from_string_test(s, sp, po, loc);
        f(jv, po);
    }

    static
    void
    grind_one(
        string_view s, source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        grind_one(
            s,
            storage_ptr(),
            [&loc](value const& jv, parse_options const& po)
            {
                check_round_trip(jv, po, loc);
            },
            parse_options());
    }

    template<class F>
    static
    void
    grind(
        string_view s,
        F const& f,
        parse_options const & po = {},
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        try
        {
            grind_one(s, {}, f, po, loc);

            fail_loop([&](storage_ptr const& sp)
            {
                grind_one(s, sp, f, po, loc);
            });

            if(s.size() > 1)
            {
                // Destroy the stream_parser at every
                // split point to check leaks.
                for(std::size_t i = 1;
                    i < s.size(); ++i)
                {
                    fail_resource mr;
                    mr.fail_max = 0;
                    stream_parser p(storage_ptr(), po);
                    system::error_code ec;
                    p.reset(&mr);
                    p.write(s.data(), i, ec);
                    if(BOOST_TEST(! ec, loc))
                        p.write(
                            s.data() + i,
                            s.size() - i, ec);
                    if(BOOST_TEST(! ec, loc))
                        p.finish(ec);
                    if(BOOST_TEST(! ec, loc))
                        f(p.release(), po);
                }
            }
        }
        catch(std::exception const&)
        {
            BOOST_TEST_FAIL(loc);
        }
    }

    static
    void
    grind(
        string_view s,
        parse_options const& po,
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        grind(
            s,
            [&loc](value const& jv, parse_options const& po)
            {
                check_round_trip(jv, po, loc);
            },
            po,
            loc);
    }

    static
    void
    grind_double(
        string_view s,
        double v,
        parse_options const& po = {},
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        grind(
            s,
            [v, &loc](value const& jv, parse_options const&)
            {
                if(! BOOST_TEST( jv.is_double(), loc ))
                    return;
                if( std::isnan(v) )
                    BOOST_TEST( std::isnan( jv.get_double() ), loc );
                else
                    BOOST_TEST( jv.get_double() == v, loc );
            },
            po,
            loc);
    }

    // Verify that f converts to the
    // same double produced by `strtod`.
    // Requires `s` is not represented by an integral type.
    template<class F>
    void
    fc(
        std::string const& s,
        F const& f,
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        char* str_end;
        double const need = std::strtod(s.c_str(), &str_end);
        for (number_precision numbers : {
            number_precision::imprecise,
            number_precision::imprecise,
            number_precision::none})
        {
            parse_options po;
            po.numbers = numbers;

            double const got = f(s, po);
            if( numbers != number_precision::none)
            {
                bool const same = got == need;
                bool const close = same ? true : within_1ulp(got, need);

                if( !BOOST_TEST(close, loc) )
                {
                    char const* mode = numbers == number_precision::precise
                        ? "precise" : "imprecise" ;
                    log << "Failure on '" << s << "' (" << mode << "): "
                        << got << " != " << need << "\n";
                }
            }
        }
    }

    void
    fc(
        std::string const& s,
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        fc(s, f_boost{loc}, loc);
        fc(s + std::string(64, ' '), f_boost{loc}, loc);
    }

    void checkAccuracy(
        char const* nm,
        int max_ulp,
        parse_options const& opts = {},
        source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        double x = std::strtod( nm, 0 );
        double y = boost::json::parse( nm, {}, opts ).as_double();
        std::uint64_t bx, by;
        std::memcpy( &bx, &x, sizeof(x) );
        std::memcpy( &by, &y, sizeof(y) );
        std::int64_t diff = bx - by;
        if (!BOOST_TEST(std::abs(diff) <= max_ulp, loc))
        {
            char buf[1024];
            sprintf(
                buf,
                "%s: difference %" PRId64 " ulp\n"
                "  strtod:       %.13a %.16g\n"
                "  boost.json:   %.13a %.16g\n\n",
                nm, diff, x, x, y, y );
            log << buf;
        }
    }

    void
    testDouble()
    {
        grind_double("-1.010", -1.01);
        grind_double("-0.010", -0.01);
        grind_double("-0.0", -0.0);
        grind_double("-0e0", -0.0);
        grind_double( "18.4",  18.4);
        grind_double("-18.4", -18.4);
        grind_double( "18446744073709551616",  1.8446744073709552e+19);
        grind_double("-18446744073709551616", -1.8446744073709552e+19);
        grind_double( "18446744073709551616.0",  1.8446744073709552e+19);
        grind_double( "18446744073709551616.00009",  1.8446744073709552e+19);
        grind_double( "1844674407370955161600000",  1.8446744073709552e+24);
        grind_double("-1844674407370955161600000", -1.8446744073709552e+24);
        grind_double( "1844674407370955161600000.0",  1.8446744073709552e+24);
        grind_double( "1844674407370955161600000.00009",  1.8446744073709552e+24);
        grind_double( "19700720435664.186294290058937593e13",  1.9700720435664185e+26);

        grind_double( "1.0", 1.0);
        grind_double( "1.1", 1.1);
        grind_double( "1.11", 1.11);
        grind_double( "1.11111", 1.11111);
        grind_double( "11.1111", 11.1111);
        grind_double( "111.111", 111.111);

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
    testWithinULP()
    {
        std::mt19937_64 rng;

        checkAccuracy("10199214983525025199.13135016100190689227e-308", 2);

        for( int i = 0; i < 1000000; ++i )
        {
            unsigned long long x1 = rng();
            unsigned long long x2 = rng();
            int x3 = std::uniform_int_distribution<>( -308, +308 )( rng );

            char buffer[ 128 ];
            sprintf( buffer, "%llu.%llue%d", x1, x2, x3 );

            parse_options precise;
            precise.numbers = number_precision::precise;
            checkAccuracy( buffer, 2 );
            checkAccuracy( buffer, 0, precise );
        }

        for( int i = -326; i <= +309; ++i )
        {
            char buffer[ 128 ];
            sprintf( buffer, "1e%d", i );

            int max_ulp = 0;
#if BOOST_JSON_ARCH == 32
            max_ulp = 1; // we may be more precise than strtod for 32 bits
#endif // BOOST_JSON_ARCH == 32
            checkAccuracy( buffer, max_ulp );
        }
    }

    void
    testExtraPrecision()
    {
        parse_options opts;
        opts.numbers = number_precision::precise;
        BOOST_TEST(
            parse("1002.9111801605201", {}, opts) == 1002.9111801605201 );
        BOOST_TEST(
            parse("-1.0346132515963697", {}, opts) == -1.0346132515963697 );
        BOOST_TEST(
            parse("-1207.1290929173115", {}, opts) == -1207.1290929173115 );
        BOOST_TEST(
            parse("-0.90521880279912548", {}, opts) == -0.90521880279912548 );
        BOOST_TEST(
            parse("370.91535570754445", {}, opts) == 370.91535570754445 );
        BOOST_TEST(
            parse("-2578.5523049665962", {}, opts) == -2578.5523049665962 );

        // test cases from https://www.icir.org/vern/papers/testbase-report.pdf
        // (A Program for Testing IEEE Decimal–Binary Conversion by Vern Paxson)
        checkAccuracy("5e125", 0, opts);
        checkAccuracy("69e267", 0, opts);
        checkAccuracy("999e-026", 0, opts);
        checkAccuracy("7861e-034", 0, opts);
        checkAccuracy("75569e-254", 0, opts);
        checkAccuracy("928609e-261", 0, opts);
        checkAccuracy("9210917e080", 0, opts);
        checkAccuracy("84863171e114", 0, opts);
        checkAccuracy("653777767e273", 0, opts);
        checkAccuracy("5232604057e-298", 0, opts);
        checkAccuracy("27235667517e-109", 0, opts);
        checkAccuracy("653532977297e-123", 0, opts);
        checkAccuracy("3142213164987e-294", 0, opts);
        checkAccuracy("46202199371337e-072", 0, opts);
        checkAccuracy("231010996856685e-073", 0, opts);
        checkAccuracy("9324754620109615e212", 0, opts);
        checkAccuracy("78459735791271921e049", 0, opts);
        checkAccuracy("272104041512242479e200", 0, opts);
        checkAccuracy("6802601037806061975e198", 0, opts);
        checkAccuracy("20505426358836677347e-221", 0, opts);
        checkAccuracy("836168422905420598437e-234", 0, opts);
        checkAccuracy("4891559871276714924261e222", 0, opts);
        checkAccuracy("9e-265", 0, opts);
        checkAccuracy("85e-037", 0, opts);
        checkAccuracy("623e100", 0, opts);
        checkAccuracy("3571e263", 0, opts);
        checkAccuracy("81661e153", 0, opts);
        checkAccuracy("920657e-023", 0, opts);
        checkAccuracy("4603285e-024", 0, opts);
        checkAccuracy("87575437e-309", 0, opts);
        checkAccuracy("245540327e122", 0, opts);
        checkAccuracy("6138508175e120", 0, opts);
        checkAccuracy("83356057653e193", 0, opts);
        checkAccuracy("619534293513e124", 0, opts);
        checkAccuracy("2335141086879e218", 0, opts);
        checkAccuracy("36167929443327e-159", 0, opts);
        checkAccuracy("609610927149051e-255", 0, opts);
        checkAccuracy("3743626360493413e-165", 0, opts);
        checkAccuracy("94080055902682397e-242", 0, opts);
        checkAccuracy("899810892172646163e283", 0, opts);
        checkAccuracy("7120190517612959703e120", 0, opts);
        checkAccuracy("25188282901709339043e-252", 0, opts);
        checkAccuracy("308984926168550152811e-052", 0, opts);
        checkAccuracy("6372891218502368041059e064", 0, opts);
    }

    void
    testSpecialNumbers()
    {
        parse_options with_special_numbers;
        with_special_numbers.allow_infinity_and_nan = true;

        grind_double(
            "Infinity",
            std::numeric_limits<double>::infinity(),
            with_special_numbers);

        grind_double(
            "-Infinity",
            -std::numeric_limits<double>::infinity(),
            with_special_numbers);

        grind_double(
            "NaN",
            std::numeric_limits<double>::quiet_NaN(),
            with_special_numbers);
    }

    void
    run()
    {
        testDouble();
        testWithinULP();
        testExtraPrecision();
        testSpecialNumbers();
    }
};

TEST_SUITE(double_test, "boost.json.double");

} // namespace json
} // namespace boost
