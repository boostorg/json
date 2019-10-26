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
    parse_grind(
        string_view input,
        error_code ex)
    {
        if(input.size() > 100)
            return;

        // iterate each split point
        if(input.size() > 0)
        {
            for(std::size_t i = 0;
                i < input.size() - 1; ++i)
            {
                BEAST_EXPECT(i < 10000);

                // test exceptions
                for(std::size_t j = 1;;++j)
                {
                    if(! BEAST_EXPECT(j < 10000))
                        break;
                    error_code ec;
                    throw_parser p(j);
                    try
                    {
                        auto const n = p.write_some(
                            input.data(), i, ec);
                        if(! ec)
                        {
                            p.write(input.data() + n,
                                input.size() - n, ec);
                        }
                        if(ec)
                            BEAST_EXPECTS(
                                ec == ex, std::string(input) +
                                " : " + ec.message());
                        break;
                    }
                    catch(test_exception const&)
                    {
                        continue;
                    }
                    catch(...)
                    {
                        BEAST_FAIL();
                    }
                }

                // test errors
                for(std::size_t j = 1;;++j)
                {
                    if(! BEAST_EXPECT(j < 10000))
                        break;
                    error_code ec;
                    fail_parser p(j);
                    auto n = p.write_some(
                        input.data(), i, ec);
                    if(ec == error::test_failure)
                        continue;
                    if(! ec)
                    {
                        p.write_some(input.data() + n,
                            input.size() - n, ec);
                        if(ec == error::test_failure)
                            continue;
                    }
                    if(! ec)
                    {
                        p.write_eof(ec);
                        if(ec == error::test_failure)
                            continue;
                    }
                    if(ec)
                        BEAST_EXPECTS(
                            ec == ex, std::string(input) +
                            " : " + ec.message());
                    break;
                }
            }
        }
    }

    void
    good(string_view s)
    {
        parse_grind(s, error_code{});
    }

    void
    bad(string_view s)
    {
        error_code ec;
        fail_parser p;
        auto const used = p.write_some(
            s.data(), s.size(), ec);
        if(! ec)
        {
            if(p.is_done())
            {
                if(BEAST_EXPECT(used != s.size()))
                    return;
            }
            else
            {
                p.write_eof(ec);
                if(BEAST_EXPECT(ec))
                    return;
            }
        }
        else
        {
            pass();
            return;
        }
        log << "fail: \"" << s << "\"\n";
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
        test("0e32767", 0, 32767, false);
        test("0e-32768", 0, -32768, false);
        test(" 9223372036854775807", 9223372036854775807, 0, false);
        test("-9223372036854775807", 9223372036854775807, 0, true);
        test(" 9223372036854775807e32767",  9223372036854775807,  32767, false);
        test(" 9223372036854775807e-32768", 9223372036854775807, -32768, false);
        test("-9223372036854775807e32767",  9223372036854775807,  32767, true);
        test("-9223372036854775807e-32768", 9223372036854775807, -32768, true);
        test(" 18446744073709551615", 18446744073709551615ULL, 0, false);
        test("-18446744073709551615", 18446744073709551615ULL, 0, true);
        test(" 18446744073709551615e32767",  18446744073709551615ULL,  32767, false);
        test(" 18446744073709551615e-32768", 18446744073709551615ULL, -32768, false);
        test("-18446744073709551615e32767",  18446744073709551615ULL,  32767, true);
        test("-18446744073709551615e-32768", 18446744073709551615ULL, -32768, true);
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
        std::size_t fail = 0;
        std::size_t info = 0;
        auto const tot = pv.size();
        for(auto const& v : pv)
        {
            error_code ec;
            fail_parser p;
            p.write(
                v.text.data(),
                v.text.size(),
                ec);
            if(v.result == 'i')
            {
                auto const s = ec ?
                    "reject" : "accept";
                ++info;
                log <<
                    "'" << v.result << "' " <<
                    v.name << " " << s << "\n";
                parse_grind(v.text, ec);
                continue;
            }
            char result;
            result = ec ? 'n' : 'y';
            if(result != v.result)
            {
                if(v.result == 'i')
                    ++info;
                else
                    ++fail;
                log <<
                    "'" << v.result << "' " <<
                    v.name;
                if(ec)
                    log << " " << ec.message() << "\n";
                else
                    log << "\n";
            }
            else
            {
                parse_grind(v.text, ec);
            }
        }
        if(fail > 0)
            log << fail << "/" << tot <<
            " parse vector failures, " <<
            info << " informational.\n";
    }

    void
    run() override
    {
        log <<
            "sizeof(basic_parser) == " <<
            sizeof(basic_parser) << "\n";
        testParseVectors();

        testObject();
        testArray();
        testString();
        testNumber();
        testBoolean();
        testNull();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,basic_parser);

} // json
} // boost
