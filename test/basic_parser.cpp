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

#include "parse-vectors.hpp"

namespace boost {
namespace json {

class basic_parser_test : public beast::unit_test::suite
{
public:
    class test_parser
        : public basic_parser
    {
        std::size_t n_ = std::size_t(-1);

        void
        maybe_fail(error_code& ec)
        {
            if(n_ && --n_ > 0)
                return;
            ec = error::test_failure;
        }

        void
        on_document_begin(
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_object_begin(
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_object_end(
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_array_begin(
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_array_end(
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_key_data(
            string_view,
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_key_end(
            string_view,
            error_code& ec) override
        {
            maybe_fail(ec);
        }
        
        void
        on_string_data(
            string_view,
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_string_end(
            string_view,
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_number(
            number,
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_bool(
            bool,
            error_code& ec) override
        {
            maybe_fail(ec);
        }

        void
        on_null(error_code& ec) override
        {
            maybe_fail(ec);
        }

    public:
        test_parser() = default;

        test_parser(
            std::size_t n)
            : n_(n)
        {
        }
    };

    void
    parse_grind(
        string_view input,
        error_code ex)
    {
        if(input.size() > 100)
            return;
        for(std::size_t n = 0;
            n < input.size() - 1; ++n)
        {
            error_code ec;
            test_parser p;
            p.write_some(input.data(), n, ec);
            if(! ec)
                p.write_some(input.data() + n,
                    input.size() - n, ec);
            if(! ec)
                p.write_eof(ec);
            if(ec)
            {
                BEAST_EXPECTS(
                    ec == ex, std::string(input) +
                    " : " + ec.message());
                return;
            }
        }

        std::size_t n = 1;
        for(; n < 10000; ++n)
        {
            error_code ec;
            test_parser p{n};
            p.write(
                input.data(),
                input.size(),
                ec);
            if(ec != error::test_failure)
            {
                BEAST_EXPECTS(
                    ec == ex, std::string(input) +
                    " : " + ec.message());
                break;
            }
        }
        BEAST_EXPECT(n < 10000);
    }

    void
    good(string_view s)
    {
        error_code ec;
        for(std::size_t i = 0;
            i < s.size() - 1; ++i)
        {
            // write_some with 1 buffer
            {
                test_parser p;
                auto used = p.write_some(s.data(), i, ec);
                BEAST_EXPECT(used == i);
                BEAST_EXPECT(! p.is_done());
                if(! BEAST_EXPECTS(! ec, ec.message()))
                    continue;
                used = p.write_some(
                    s.data() + i, s.size() - i, ec);
                BEAST_EXPECT(used == s.size() - i);
                if(! BEAST_EXPECTS(! ec, ec.message()))
                    continue;
                p.write(nullptr, 0, ec);
                BEAST_EXPECTS(! ec, ec.message());
                BEAST_EXPECT(p.is_done());
            }
            // write with 1 buffer
            {
                test_parser p;
                auto used = p.write(s.data(), s.size(), ec);
                BEAST_EXPECT(used = s.size());
                BEAST_EXPECTS(! ec, ec.message());
            }
        }
    }

    void
    bad(string_view s)
    {
        error_code ec;
        test_parser p;
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
    testMonostates()
    {
        good("true");
        good(" true");
        good("true ");
        good("\ttrue");
        good("true\t");
        good("\r\n\t true\r\n\t ");
        bad ("truu");
        bad ("tu");
        bad ("t");

        good("false");
        bad ("fals");
        bad ("fel");
        bad ("f");

        good("null");
        bad ("nul");
        bad ("no");
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
            test_parser p;
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
        testMonostates();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,basic_parser);

} // json
} // boost
