//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

//#define RAPIDJSON_SSE42

#include "lib/nlohmann/single_include/nlohmann/json.hpp"

#include "lib/rapidjson/include/rapidjson/rapidjson.h"
#include "lib/rapidjson/include/rapidjson/document.h"
#include "lib/rapidjson/include/rapidjson/writer.h"
#include "lib/rapidjson/include/rapidjson/stringbuffer.h"

#include <boost/json.hpp>
#include <boost/beast/_experimental/unit_test/dstream.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <cstdio>
#include <vector>

/*

References

https://github.com/nst/JSONTestSuite

http://seriot.ch/parsing_json.php

*/

namespace beast = boost::beast;

namespace boost {
namespace json {

using clock_type = std::chrono::steady_clock;
using string_view = boost::string_view;

beast::unit_test::dstream dout{std::cerr};

//----------------------------------------------------------

class any_impl
{
public:
    virtual ~any_impl() = default;
    virtual string_view name() const noexcept = 0;
    virtual void parse(string_view s, int repeat) const = 0;
    virtual void serialize(string_view s, int repeat) const = 0;
};

//----------------------------------------------------------

class boost_impl : public any_impl
{
public:
    string_view
    name() const noexcept override
    {
        return "boost(block)";
    }

    void
    parse(
        string_view s,
        int repeat) const override
    {
        while(repeat--)
        {
            scoped_storage<
                block_storage> ss;
            json::parse(s, ss);
        }
    }

    void
    serialize(
        string_view s,
        int repeat) const override
    {
        scoped_storage<
            block_storage> ss;
        auto jv = json::parse(s, ss);
        while(repeat--)
            to_string(jv);
    }
};

//----------------------------------------------------------

class boost_default_impl : public any_impl
{
public:
    string_view
    name() const noexcept override
    {
        return "boost(default)";
    }

    void
    parse(
        string_view s,
        int repeat) const override
    {
        while(repeat--)
            json::parse(s);
    }

    void
    serialize(
        string_view s,
        int repeat) const override
    {
        auto jv = json::parse(s);
        while(repeat--)
            to_string(jv);
    }
};

//----------------------------------------------------------

class boost_vec_impl : public any_impl
{
    struct vec_parser : basic_parser
    {
        std::size_t n_ = std::size_t(-1);
        char buf[256];
        std::vector<value> vec_;

        void
        on_stack_info(
            stack& s) noexcept override
        {
            s.base = buf;
            s.capacity = sizeof(buf);
        }

        void
        on_stack_grow(
            stack&,
            unsigned,
            error_code& ec) override
        {
            ec = error::too_deep;
        }

        void
        on_document_begin(
            error_code&) override
        {
        }

        void
        on_object_begin(
            error_code&) override
        {
        }

        void
        on_object_end(
            error_code&) override
        {
        }

        void
        on_array_begin(
            error_code&) override
        {
        }

        void
        on_array_end(
            error_code&) override
        {
        }

        void
        on_key_data(
            string_view,
            error_code&) override
        {
        }

        void
        on_key_end(
            string_view,
            error_code&) override
        {
        }
        
        void
        on_string_data(
            string_view,
            error_code&) override
        {
        }

        void
        on_string_end(
            string_view,
            error_code&) override
        {
        }

        void
        on_number(
            ieee_decimal dec,
            error_code&) override
        {
            vec_.emplace_back(dec);
        }

        void
        on_bool(
            bool,
            error_code&) override
        {
        }

        void
        on_null(error_code&) override
        {
        }

    public:
        vec_parser() = default;

        explicit
        vec_parser(
            std::size_t n)
            : n_(n)
        {
        }
    };
public:
    string_view
    name() const noexcept override
    {
        return "boost(vector)";
    }

    void
    parse(
        string_view s,
        int repeat) const override
    {
        while(repeat--)
        {
            vec_parser p;
            error_code ec;
            p.write(s.data(), s.size(), ec);
        }
    }

    void
    serialize(
        string_view,
        int) const override
    {
    }
};

//----------------------------------------------------------

struct rapidjson_impl : public any_impl
{
    string_view
    name() const noexcept override
    {
        return "rapidjson";
    }

    void
    parse(string_view s, int repeat) const override
    {
        while(repeat--)
        {
            rapidjson::Document d;
            d.Parse(s.data(), s.size());
        }
    }

    void
    serialize(string_view s, int repeat) const override
    {
        rapidjson::Document d;
        d.Parse(s.data(), s.size());
        while(repeat--)
        {
            rapidjson::StringBuffer st;
            st.Clear();
            rapidjson::Writer<
                rapidjson::StringBuffer> wr(st);
            d.Accept(wr);
        }
    }
};

//----------------------------------------------------------

struct nlohmann_impl : public any_impl
{
    string_view
    name() const noexcept override
    {
        return "nlohmann";
    }

    void
    parse(string_view s, int repeat) const override
    {
        while(repeat--)
            nlohmann::json::parse(
                s.begin(), s.end());
    }

    void
    serialize(string_view, int) const override
    {
    }
};

//----------------------------------------------------------

struct file_item
{
    string_view name;
    std::string text;
};

using file_list = std::vector<file_item>;

std::string
load_file(char const* path)
{
    FILE* f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    auto const size = ftell(f);
    std::string s;
    s.resize(size);
    fseek(f, 0, SEEK_SET);
    fread(&s[0], 1, size, f);
    fclose(f);
    return s;
}

void
benchParse(
    file_list const& vs,
    std::vector<std::unique_ptr<
        any_impl const>> const& vi)
{
    for(unsigned i = 0; i < vs.size(); ++i)
    {
        dout <<
            "Parse File " << std::to_string(i+1) <<
                " " << vs[i].name << " (" <<
                std::to_string(vs[i].text.size()) << " bytes)" <<
                std::endl;
        for(unsigned j = 0; j < vi.size(); ++j)
        {
            for(unsigned k = 0; k < 10; ++k)
            {
                auto const when = clock_type::now();
                vi[j]->parse(vs[i].text, 250);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                if(k > 4)
                    dout << " " << vi[j]->name() << ": " <<
                        std::to_string(ms) << "ms" <<
                        std::endl;
            }
        }
        dout << std::endl;
    }
}

void
benchSerialize(
    file_list const& vs,
    std::vector<std::unique_ptr<
        any_impl const>> const& vi)
{
    for(unsigned i = 0; i < vs.size(); ++i)
    {
        dout <<
            "Serialize File " << std::to_string(i+1) <<
                " " << vs[i].name << " (" <<
                std::to_string(vs[i].text.size()) << " bytes)" <<
                std::endl;
        for(unsigned j = 0; j < vi.size(); ++j)
        {
            for(unsigned k = 0; k < 10; ++k)
            {
                auto const when = clock_type::now();
                vi[j]->serialize(vs[i].text, 1000);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                if(k >  4)
                    dout << " " << vi[j]->name() << ": " <<
                        std::to_string(ms) << "ms" <<
                        std::endl;
            }
        }
        dout << std::endl;
    }
}

} // json
} // boost

int
main(
    int const argc,
    char const* const* const argv)
{
    using namespace boost::json;
    file_list vs;
    if(argc > 1)
    {
        vs.reserve(argc - 1);
        for(int i = 1; i < argc; ++i)
            vs.emplace_back(
                file_item{argv[i],
                load_file(argv[i])});
    }

    try
    {
        std::vector<std::unique_ptr<any_impl const>> vi;
        vi.reserve(10);
        //vi.emplace_back(new boost_vec_impl);
        //vi.emplace_back(new boost_default_impl);
        vi.emplace_back(new boost_impl);
        vi.emplace_back(new rapidjson_impl);
        //vi.emplace_back(new nlohmann_impl);

        benchParse(vs, vi);
        benchSerialize(vs, vi);
    }
    catch(system_error const& se)
    {
        dout << se.what() << std::endl;
    }

    return 0;
}

/*
Parse File 1 array.json (609414 bytes)
 boost(block): 511ms
 boost(block): 497ms
 boost(block): 474ms
 boost(block): 469ms
 boost(block): 463ms
 rapidjson: 378ms
 rapidjson: 379ms
 rapidjson: 378ms
 rapidjson: 376ms
 rapidjson: 378ms

Parse File 2 arrays.json (1461524 bytes)
 boost(block): 1523ms
 boost(block): 1513ms
 boost(block): 1516ms
 boost(block): 1513ms
 boost(block): 1520ms
 rapidjson: 936ms
 rapidjson: 936ms
 rapidjson: 934ms
 rapidjson: 949ms
 rapidjson: 950ms

Parse File 3 strings.json (1016132 bytes)
 boost(block): 48ms
 boost(block): 48ms
 boost(block): 48ms
 boost(block): 48ms
 boost(block): 48ms
 rapidjson: 455ms
 rapidjson: 453ms
 rapidjson: 455ms
 rapidjson: 453ms
 rapidjson: 453ms

Parse File 4 twitter.json (646995 bytes)
 boost(block): 747ms
 boost(block): 746ms
 boost(block): 744ms
 boost(block): 747ms
 boost(block): 747ms
 rapidjson: 438ms
 rapidjson: 437ms
 rapidjson: 450ms
 rapidjson: 443ms
 rapidjson: 450ms

Parse File 5 citm_catalog.json (1777672 bytes)
 boost(block): 1485ms
 boost(block): 1485ms
 boost(block): 1485ms
 boost(block): 1487ms
 boost(block): 1485ms
 rapidjson: 910ms
 rapidjson: 909ms
 rapidjson: 909ms
 rapidjson: 907ms
 rapidjson: 910ms

Parse File 6 canada.json (2251060 bytes)
 boost(block): 4046ms
 boost(block): 4191ms
 boost(block): 4295ms
 boost(block): 4131ms
 boost(block): 4134ms
 rapidjson: 1412ms
 rapidjson: 1409ms
 rapidjson: 1410ms
 rapidjson: 1409ms
 rapidjson: 1411ms

Serialize File 1 array.json (609414 bytes)
 boost(block): 854ms
 boost(block): 853ms
 boost(block): 851ms
 boost(block): 853ms
 boost(block): 853ms
 rapidjson: 785ms
 rapidjson: 785ms
 rapidjson: 783ms
 rapidjson: 785ms
 rapidjson: 785ms

Serialize File 2 arrays.json (1461524 bytes)
 boost(block): 1269ms
 boost(block): 1259ms
 boost(block): 1257ms
 boost(block): 1259ms
 boost(block): 1260ms
 rapidjson: 3112ms
 rapidjson: 3110ms
 rapidjson: 3109ms
 rapidjson: 3109ms
 rapidjson: 3114ms

Serialize File 3 strings.json (1016132 bytes)
 boost(block): 582ms
 boost(block): 584ms
 boost(block): 582ms
 boost(block): 585ms
 boost(block): 584ms
 rapidjson: 2072ms
 rapidjson: 2072ms
 rapidjson: 2073ms
 rapidjson: 2074ms
 rapidjson: 2074ms

Serialize File 4 twitter.json (646995 bytes)
 boost(block): 658ms
 boost(block): 659ms
 boost(block): 659ms
 boost(block): 659ms
 boost(block): 659ms
 rapidjson: 871ms
 rapidjson: 871ms
 rapidjson: 872ms
 rapidjson: 872ms
 rapidjson: 872ms

Serialize File 5 citm_catalog.json (1777672 bytes)
 boost(block): 1049ms
 boost(block): 1049ms
 boost(block): 1048ms
 boost(block): 1047ms
 boost(block): 1047ms
 rapidjson: 898ms
 rapidjson: 896ms
 rapidjson: 898ms
 rapidjson: 898ms
 rapidjson: 898ms

Serialize File 6 canada.json (2251060 bytes)
 boost(block): 10521ms
 boost(block): 10616ms
 boost(block): 10560ms
 boost(block): 10553ms
 boost(block): 10446ms
 rapidjson: 10788ms
 rapidjson: 10776ms
 rapidjson: 10784ms
 rapidjson: 10805ms
 rapidjson: 10789ms

*/
