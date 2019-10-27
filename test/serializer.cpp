//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/serializer.hpp>

#include <boost/json/parser.hpp>
#include <boost/beast/_experimental/unit_test/suite.hpp>
#include "parse-vectors.hpp"

#define SOFT_FAIL

namespace boost {
namespace json {

class serializer_test : public beast::unit_test::suite
{
public:
    static
    unsigned
    common(
        string_view s1,
        string_view s2)
    {
        unsigned n = 0;
        auto p1 = s1.data();
        auto p2 = s2.data();
        auto end = s1.size() > s2.size() ?
            s2.end() : s1.end();
        while(p1 < end)
        {
            ++n;
            if(*p1++ != *p2++)
                break;
        }
        return n;
    }

    void
    round_trip(
        string_view name,
        string_view s0)
    {
        error_code ec;
        auto jv0 = parse(s0, ec);
    #ifdef SOFT_FAIL
        if(ec)
            return;
    #else
        if( ! BEAST_EXPECTS(
            ! ec, ec.message()))
            return;
    #endif
        auto s1 = to_string(jv0);
        parser p;
        auto n = p.write(
            s1.data(), s1.size(), ec);
        auto jv1 = p.release();
    #ifdef SOFT_FAIL
        if(ec)
    #else
        if( ! BEAST_EXPECTS(
            ! ec, ec.message()))
    #endif
        {
            auto c1 = s1.data() + n;
            if( n > 60)
                n = 60;
            auto c0 = c1 - n;
            log <<
                "context\n"
                "  " << string_view(c0, c1-c0) << std::endl;
            log <<
                name << "\n"
                "  " << s0 << "\n"
                "  " << s1 << std::endl << std::endl;
            return;
        }
        auto s2 = to_string(jv1);
    #ifdef SOFT_FAIL
        if(s1 != s2)
    #else
        if(! BEAST_EXPECT(s1 == s2))
    #endif
        {
            auto c = common(s1, s2);
            log <<
                name << "\n"
                "  " << s0 << "\n"
                "  " << s1.substr(0, c) << "\n"
                "  " << s2.substr(0, c) << std::endl << std::endl;
        }
    }

    void
    print_grind(
        string_view name,
        json::value const& jv)
    {
        auto const s0 = to_string(jv);
        log << s0 << std::endl;
        round_trip(name, s0);
        for(std::size_t i = 1;
            i < s0.size() - 1; ++i)
        {
            std::string s;
            s.resize(s0.size());
            serializer sr(jv);
            auto const n1 =
                sr.next(&s[0], i);
            if(BEAST_EXPECT(n1 == i))
                sr.next(
                    &s[n1], s.size() - n1);
            if(! BEAST_EXPECT(s == s0))
            {
                log <<
                    "  " << s0 << "\n"
                    "  " << s << std::endl << std::endl;
            }
        }
    }

    void
    testSerializer()
    {
        value jv;

        // serializer(value)
        {
            serializer sr(jv);
        }

        // is_done()
        {
            serializer sr(jv);
            BEAST_EXPECT(! sr.is_done());
        }

        // next()
        {
            serializer sr(jv);
            char buf[1024];
            auto n = sr.next(
                buf, sizeof(buf));
            BEAST_EXPECT(sr.is_done());
            BEAST_EXPECT(string_view(
                buf, n) == "null");
        }
    }

    void
    testRoundTrips()
    {
        parse_vectors const pv;
        for(auto const e : pv)
        {
            if(e.result != 'y')
                continue;

            round_trip(e.name, e.text);
        }
    }

    bool
    good(string_view s)
    {
        error_code ec;
        auto jv = parse(s, ec);
        return !ec;
    }

    template<std::size_t N>
    void
    tv(char const (&s)[N])
    {
        round_trip(
            "",
            string_view(s, N - 1));
    }

    void
    doTestStrings()
    {
        tv(R"("")");
        tv(R"("x")");
        tv(R"("xyz")");
        tv(R"("x z")");
        tv(R"("\"")");
        tv(R"("\\")");
        tv(R"("\/")");
        tv(R"("\b")");
        tv(R"("\f")");
        tv(R"("\n")");
        tv(R"("\r")");
        tv(R"("\t")");
        tv(R"("\u0000")");
        tv(R"("\u0001")");
        tv(R"("\u0002")");
        tv(R"("\u0003")");
        tv(R"("\u0004")");
        tv(R"("\u0005")");
        tv(R"("\u0006")");
        tv(R"("\u0007")");
        tv(R"("\u0008")");
        tv(R"("\u0009")");
        tv(R"("\u000a")");
        tv(R"("\u000b")");
        tv(R"("\u000c")");
        tv(R"("\u000d")");
        tv(R"("\u000e")");
        tv(R"("\u000f")");
        tv(R"("\u0010")");
        tv(R"("\u0011")");
        tv(R"("\u0012")");
        tv(R"("\u0013")");
        tv(R"("\u0014")");
        tv(R"("\u0015")");
        tv(R"("\u0016")");
        tv(R"("\u0017")");
        tv(R"("\u0018")");
        tv(R"("\u0019")");
        tv(R"("\u0020")");
        tv(R"("\u0020")");

        tv(R"(0)");
        tv(R"(-0)");
        tv(R"(1)");
        tv(R"(-1)");
        tv(R"(99999)");
        tv(R"(-99999)");

        tv(R"(true)");
        tv(R"(false)");

        tv(R"(null)");
    }

    void
    doTestVectors()
    {
        parse_vectors const pv;
        for(auto const e : pv)
        {
            if( e.result == 'y' ||
                good(e.text))
            {
                //log << i++ << " " << e.text << std::endl;
                round_trip(e.name, e.text);
            }
        }
    }

    void
    run()
    {
        testSerializer();
        doTestStrings();
        doTestVectors();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,serializer);

} // json
} // boost
