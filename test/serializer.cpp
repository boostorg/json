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

//#define SOFT_FAIL

namespace boost {
namespace json {

class serializer_test : public beast::unit_test::suite
{
public:
    static
    std::string
    to_string(value const& jv)
    {
        std::string s;
        unsigned long cap = 1024;
        for(;;)
        {
            s.resize(cap);
            serializer p(jv);
            s.resize(p.next(
                &s[0], s.size()));
            if(p.is_done())
                break;
            cap *= 2;
        }
        return s;
    }

    static
    value
    from_string(
        string_view s,
        error_code& ec)
    {
        parser p;
        p.write(
            s.data(),
            s.size(),
            ec);
        return p.release();
    }

    void
    round_trip(string_view s0)
    {
        error_code ec;
        auto jv0 = from_string(s0, ec);
    #ifdef SOFT_FAIL
        if(ec)
            return;
    #else
        if( ! BEAST_EXPECTS(
            ! ec, ec.message()))
            return;
    #endif
        auto s1 = to_string(jv0);
        auto jv1 = from_string(s1, ec);
    #ifdef SOFT_FAIL
        if(ec)
    #else
        if( ! BEAST_EXPECTS(
            ! ec, ec.message()))
    #endif
        {
            log <<
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
            log <<
                "  " << s1 << "\n"
                "  " << s2 << std::endl << std::endl;
    }

    void
    print_grind(
        json::value const& jv)
    {
        auto const s0 = to_string(jv);
        log << s0 << std::endl;
        round_trip(s0);
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
    testRoundTrips()
    {
        parse_vectors const pv;
        for(auto const e : pv)
        {
            if(e.result != 'y')
                continue;

            round_trip(e.text);
#if 0
            error_code ec;
            std::string s1, s2;
            {
                auto jv = from_string(
                    e.text, ec);
                if(! BEAST_EXPECTS(
                    ! ec, ec.message()))
                    continue;
                s1 = to_string(jv);
            }
            {
                auto jv =
                    from_string(s1, ec);
                if(! BEAST_EXPECTS(
                    ! ec, ec.message()))
                {
                    log <<
                        "  " << e.text << "\n" <<
                        "  " << s1 << "\n" <<
                        std::endl << std::endl;
                    continue;
                }
                s2 = to_string(jv);
            }
            if(! BEAST_EXPECT(s1 == s2))
            if(s1 != s2)
            {
                log <<
                    "  " << s1 << "\n" <<
                    "  " << s2 << "\n" <<
                    std::endl << std::endl;
            }
#endif
        }
    }

    bool
    good(string_view s)
    {
        error_code ec;
        auto jv = from_string(s, ec);
        return !ec;
    }

    template<std::size_t N>
    void
    tv(char const (&s)[N])
    {
        round_trip(string_view(s, N - 1));
    }

    void
    run()
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

#if 0
        parse_vectors const pv;
        for(auto const e : pv)
        {
            if( e.result == 'y' ||
                good(e.text))
            {
                log << e.name << std::endl;
                round_trip(e.text);
            }
        }
#endif
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,serializer);

} // json
} // boost
