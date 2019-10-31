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

#include <boost/json/detail/string.hpp>
#include <boost/json/detail/ieee_parser.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <type_traits>

namespace boost {
namespace json {

class number_test : public beast::unit_test::suite
{
public:
    void
    good(string_view s)
    {
        error_code ec;
        detail::ieee_parser p;
        p.write(s.data(), s.size(), ec);
        BEAST_EXPECTS(! ec, ec.message());
    }

    void
    bad(string_view s)
    {
        error_code ec;
        detail::ieee_parser p;
        p.write(s.data(), s.size(), ec);
        BEAST_EXPECTS(!! ec, ec.message());
    }

    void
    testParse()
    {
        auto const test =
            [&]( string_view s,
                decltype(ieee_decimal::mantissa) m,
                decltype(ieee_decimal::exponent) e,
                bool sign)
            {
                error_code ec;
                detail::ieee_parser p;
                p.write(s.data(), s.size(), ec);
                if(BEAST_EXPECTS(! ec, ec.message()))
                {
                    auto const dec = p.get();
                    if(! BEAST_EXPECT(
                        dec.mantissa == m &&
                        dec.exponent == e &&
                        dec.sign == sign))
                    {
                        log <<
                            "  " << s << "\n"
                            "  { " + std::to_string(dec.mantissa) +
                            ", " + std::to_string(dec.exponent) + " }" <<
                            std::endl;
                    }
                }
            };

        test( "0", 0, 0, false);
        test( "1e2", 1, 2, false);
        test( "1E2", 1, 2, false);
        test("-1E2", 1, 2, true);
        test("-1E-2", 1, -2, true);
        test( "9223372036854775807", 9223372036854775807, 0, false);
        test("-9223372036854775807", 9223372036854775807, 0, true);
        test("18446744073709551615", 18446744073709551615ULL, 0, false);
        test("-18446744073709551615", 18446744073709551615ULL, 0, true);
        test("1e308", 1, 308, false);
        test("10e308", 10, 308, false);

        test("1.0", 10, -1, false);

        good("0");
        good("0.0");
        good("0.10");
        good("0.01");
        good("1");
        good("10");
        good("1.5");
        good("10.5");
        good("10.25");
        good("10.25e0");
        good("1e1");
        good("1e10");
        good("1e+0");
        good("1e+1");
        good("0e+10");
        good("0e-0");
        good("0e-1");
        good("0e-10");
        good("1E+1");
        good("-0");
        good("-1");
        good("-1e1");

        bad ("");
        bad ("-");
        bad ("00");
        bad ("00.");
        bad ("00.0");
        bad ("1a");
        bad (".");
        bad ("1.");
        bad ("1+");
        bad ("0.0+");
        bad ("0.0e+");
        bad ("0.0e-");
        bad ("0.0e0-");
        bad ("0.0e");
    }

    void
    run() override
    {
        testParse();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,number);

} // json
} // boost
