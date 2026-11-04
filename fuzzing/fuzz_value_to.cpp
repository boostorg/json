                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    //
// Copyright 2026 Arthur SC Chan (arthur.chan@adalogics.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//
// Fuzzer for the describe-based conversion API: value_to / try_value_to /
// value_from over a described struct hierarchy (containers, map, tuple,
// variant, optional) and scalar targets, exercising the error paths for
// kind mismatches and out-of-range values.
//
// Input layout: the first byte selects the conversion target; the rest is
// parsed as a JSON document.
//

#include <boost/json.hpp>

#if !defined(BOOST_DESCRIBE_CXX14)

#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE( "This example requires C++14" )

int main() {}

#else

#include <boost/describe.hpp>
#include <boost/variant2/variant.hpp>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#ifndef BOOST_NO_CXX17_HDR_OPTIONAL
# include <optional>
# define IF_CXX17_HDR_OPTIONAL(...) __VA_ARGS__
#else
# define IF_CXX17_HDR_OPTIONAL(...)
#endif // BOOST_NO_CXX17_HDR_OPTIONAL

using namespace boost::json;

struct Rec
{
    bool b = false;
    std::int64_t i = 0;
    std::uint64_t u = 0;
    double d = 0;
    std::string s;
    std::vector<std::int64_t> vi;
    std::vector<std::string> vs;
    std::map<std::string, double> m;
    std::tuple<bool, std::int64_t, std::string> t;
    boost::variant2::variant<bool, std::int64_t, std::string> var;

#ifndef BOOST_NO_CXX17_HDR_OPTIONAL
    std::optional<std::int64_t> o;
#endif // BOOST_NO_CXX17_HDR_OPTIONAL
};

BOOST_DESCRIBE_STRUCT(Rec, (),
    (b, i, u, d, s, vi, vs, m, t, var,
    IF_CXX17_HDR_OPTIONAL(o)))

std::size_t volatile sink_dump;

extern "C"
int
LLVMFuzzerTestOneInput(
    uint8_t const* data, std::size_t size)
{
    if(size < 1)
        return 0;

    uint8_t const tgt = data[0];
    string_view const jtext{
        reinterpret_cast<char const*>(data + 1), size - 1};

    boost::system::error_code ec;
    value jv = parse(jtext, ec);

    std::size_t sink = 0;
    try
    {
        switch(tgt & 7)
        {
        case 0:
        {
            // described struct: convert, re-serialise, convert back
            auto r = try_value_to<Rec>(jv);
            if(r)
            {
                value back = value_from(*r);
                sink += serialize(back).size();
                auto r2 = try_value_to<Rec>(back);
                if(r2)
                    ++sink;
            }
            break;
        }
        case 1:
        {
            auto r = try_value_to<std::vector<std::int64_t>>(jv);
            if(r)
                sink += r->size();
            break;
        }
        case 2:
        {
            auto r = try_value_to<std::string>(jv);
            if(r)
                sink += r->size();
            break;
        }
        case 3:
        {
            auto r =
                try_value_to<std::map<std::string, double>>(jv);
            if(r)
                sink += r->size();
            break;
        }
        case 4:
        {
            auto r = try_value_to<std::int64_t>(jv);
            if(r)
                sink += static_cast<std::size_t>(*r & 0xff);
            break;
        }
        case 5:
        {
            auto r = try_value_to<bool>(jv);
            if(r)
                sink += *r ? 1u : 0u;
            break;
        }
        case 6:
        {
            // round-trip of the default-constructed struct
            value v = value_from(Rec{});
            auto r = try_value_to<Rec>(v);
            if(r)
            {
                value v2 = value_from(*r);
                sink += serialize(v2).size();
            }
            break;
        }
        default:
        {
            // throwing variant: error paths surface as exceptions
            Rec x = value_to<Rec>(jv);
            sink += x.s.size();
            break;
        }
        }

        // a partially-fitting document exercises the "wrong kind for
        // member" and "missing member" error paths
        value w = object{
            {"i", std::int64_t(tgt)},
            {"s", string_view("x", 1)},
            {"d", 1.5},
        };
        auto rr = try_value_to<Rec>(w);
        if(rr)
            sink += static_cast<std::size_t>(rr->i & 0xff);
    }
    catch(...)
    {
    }
    sink_dump = sink;
    return 0;
}

#endif // !defined(BOOST_DESCRIBE_CXX14)
