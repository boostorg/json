//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/basic_parser.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

#include "test.hpp"
#include "parse-vectors.hpp"

namespace boost {
namespace json {

class basic_parser_test : public beast::unit_test::suite
{
public:
    void
    grind(string_view s, bool good)
    {
        error_code ex;
        {
            fail_parser p;
            p.write(
                s.data(),
                s.size(),
                ex);
            if(good)
                BEAST_EXPECTS(
                    ! ex, ex.message());
            else
                BEAST_EXPECT(!!ex);
        }

        // make sure all split inputs
        // produce the same result.
        for(std::size_t i = 1;
            i < s.size(); ++i)
        {
            if(! BEAST_EXPECT(i < 100000))
                break;
            error_code ec;
            fail_parser p;
            p.write_some(s.data(), i, ec);
            if(ec == ex)
                continue;
            if(! BEAST_EXPECTS(
                ! ec, ec.message()))
                continue;
            p.write(
                s.data() + i,
                s.size() - i, ec);
            BEAST_EXPECTS(ec == ex,
                ec.message());
        }

        // exercise all exception paths
        for(std::size_t j = 1;;++j)
        {
            if(! BEAST_EXPECT(j < 100000))
                break;
            error_code ec;
            throw_parser p(j);
            try
            {
                p.write(
                    s.data(), s.size(), ec);
                BEAST_EXPECTS(ec == ex,
                    ec.message());
                break;
            }
            catch(test_exception const&)
            {
                continue;
            }
            catch(std::exception const& e)
            {
                BEAST_FAIL();
                log << "  " <<
                    e.what() << std::endl;
            }
        }

        // exercise all error paths
        for(std::size_t j = 1;;++j)
        {
            if(! BEAST_EXPECT(j < 100000))
                break;
            error_code ec;
            fail_parser p(j);
            p.write(
                s.data(), s.size(), ec);
            if(ec == error::test_failure)
                continue;
            BEAST_EXPECTS(ec == ex,
                ec.message());
            break;
        }
    }

    void
    good(string_view s)
    {
        grind(s, true);
    }

    void
    bad(string_view s)
    {
        grind(s, false);
    }

    void
    testObject()
    {
        good("{}");
        good("{ }");
        good("{ \t }");
        good("{ \"x\" : null }");
        good("{ \"x\" : {} }");
        good("{ \"x\" : { \"y\" : null } }");

        bad ("{");
        bad ("{{}}");
    }

    void
    testArray()
    {
        good("[]");
        good("[ ]");
        good("[ \t ]");
        good("[ \"\" ]");
        good("[ \" \" ]");
        good("[ \"x\" ]");
        good("[ \"x\", \"y\" ]");
        bad ("[");
        bad ("[ \"x\", ]");
    }

    void
    testString()
    {
        good("\""   "x"         "\"");
        good("\""   "xy"        "\"");
        good("\""   "x y"       "\"");

        bad ("\""   "\t"        "\"");
    }

    void
    testNumber()
    {
#if 0
        auto const test =
            []( string_view s,
                decltype(ieee_decimal::mantissa) m,
                decltype(ieee_decimal::exponent) e,
                bool sign)
            {
                auto const dec = parse_ieee_decimal(s);
                BEAST_EXPECTS(dec.mantissa == m, "mantissa=" + std::to_string(dec.mantissa));
                BEAST_EXPECTS(dec.exponent == e, "exponent=" + std::to_string(dec.exponent));
                BEAST_EXPECTS(dec.sign == sign, "sign=" + std::to_string(sign));
            };

        test(" 0", 0, 0, false);
        test(" 1e2", 1, 2, false);
        test(" 1E2", 1, 2, false);
        test("-1E2", 1, 2, true);
        test("-1E-2", 1, -2, true);

        test(" 9223372036854775807", 9223372036854775807, 0, false);
        test("-9223372036854775807", 9223372036854775807, 0, true);
        test(" 18446744073709551615", 18446744073709551615ULL, 0, false);
        test("-18446744073709551615", 18446744073709551615ULL, 0, true);
        test("1.0", 10, -1, false);
#endif

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
        good("1.1e309");

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
    testBoolean()
    {
        good("true");
        good(" true");
        good("true ");
        good("\ttrue");
        good("true\t");
        good("\r\n\t true\r\n\t ");

        bad ("TRUE");
        bad ("tRUE");
        bad ("trUE");
        bad ("truE");
        bad ("truex");
        bad ("tru");
        bad ("tr");
        bad ("t");

        good("false");
        good(" false");
        good("false ");
        good("\tfalse");
        good("false\t");
        good("\r\n\t false\r\n\t ");

        bad ("FALSE");
        bad ("fALSE");
        bad ("faLSE");
        bad ("falSE");
        bad ("falsE");
        bad ("falsex");
        bad ("fals");
        bad ("fal");
        bad ("fa");
        bad ("f");
    }

    void
    testNull()
    {
        good("null");
        good(" null");
        good("null ");
        good("\tnull");
        good("null\t");
        good("\r\n\t null\r\n\t ");

        bad ("NULL");
        bad ("nULL");
        bad ("nuLL");
        bad ("nulL");
        bad ("nullx");
        bad ("nul");
        bad ("nu");
        bad ("n");
    }

    void
    testParseVectors()
    {
        parse_vectors pv;
        for(auto const& v : pv)
        {
            if(v.result == 'i')
            {
                error_code ec;
                fail_parser p;
                p.write(
                    v.text.data(),
                    v.text.size(),
                    ec);
                grind(v.text,
                    ec ? false : true);
                continue;
            }
            if(v.result == 'y')
                grind(v.text, true);
            else
                grind(v.text, false);
        }
    }

    void
    run() override
    {
        log <<
            "sizeof(basic_parser) == " <<
            sizeof(basic_parser) << "\n";

        testObject();
        testArray();
        testString();
        testNumber();
        testBoolean();
        testNull();

        testParseVectors();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,basic_parser);

} // json
} // boost
