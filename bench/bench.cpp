//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

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
#include <random>
#include <cstdio>
#include <vector>

/*

References

https://github.com/nst/JSONTestSuite

http://seriot.ch/parsing_json.php

*/

namespace beast = boost::beast;
namespace json = boost::json;

using clock_type = std::chrono::steady_clock;
using string_view = boost::string_view;

boost::beast::unit_test::dstream dout{std::cerr};

//----------------------------------------------------------

class any_impl
{
public:
    virtual ~any_impl() = default;

    virtual string_view name() const noexcept = 0;
    virtual void work(string_view s) const = 0;

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
        return "Boost.JSON";
    }

    static
    std::string
    to_string(
        json::value const& jv,
        std::size_t size_hint)
    {
        std::string s;
        std::size_t len = 0;
        s.resize(size_hint);
        json::serializer p(jv);
        for(;;)
        {
            auto const used = p.next(
                &s[len], s.size() - len);
            len += used;
            s.resize(len);
            if(p.is_done())
                break;
            s.resize((len * 3) / 2);
        }
        return s;
    }

    void
    work(string_view s) const override
    {
        std::string s2;
        {
            json::parser p;
            boost::system::error_code ec;
            p.write(s.data(), s.size(), ec);
            s2 = to_string(p.get(), s.size() * 2);
            dout << "s2.size() == " << s2.size() << std::endl;
        }
        {
            json::parser p;
            boost::system::error_code ec;
            p.write(s2.data(), s2.size(), ec);
            dout << "ec.message() == " << ec.message() << std::endl;
        }
    }

    void
    parse(
        string_view s,
        int repeat) const override
    {
        while(repeat--)
        {
            auto sp = json::make_storage<
                json::block_storage>();
            json::parse(s, sp);
        }
    }

    void
    serialize(
        string_view s,
        int repeat) const override
    {
        auto jv = json::parse(s);
        while(repeat--)
            json::to_string(jv);
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
    work(string_view s) const override
    {
        rapidjson::StringBuffer s2;
        {
            rapidjson::Document d;
            d.Parse(s.data(), s.size());
            s2.Clear();
            rapidjson::Writer<
                rapidjson::StringBuffer> wr(s2);
            d.Accept(wr);
            dout << "s2.GetSize() == " << s2.GetSize() << std::endl;
        }
        {
            rapidjson::Document d;
            d.Parse(s2.GetString(), s2.GetSize());
        }
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
    work(string_view s) const override
    {
        auto jv = nlohmann::json::parse(
            s.begin(), s.end());
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

class factory
{
    std::string s_;
    std::mt19937 g_;
    std::string lorem_;
    int depth_;
    int indent_ = 4;
    int max_depth_ = 6;

    std::size_t
    rand(std::size_t n)
    {
        return static_cast<std::size_t>(
            std::uniform_int_distribution<
                std::size_t>{0, n-1}(g_));
    }

public:
    factory()
        : lorem_(
            // 40 characters
            "Lorem ipsum dolor sit amet, consectetur " //  1
            "adipiscing elit, sed do eiusmod tempor i" //  2
            "ncididunt ut labore et dolore magna aliq" //  3
            "ua. Ut enim ad minim veniam, quis nostru" //  4
            "d exercitation ullamco laboris nisi ut a" //  5
            "liquip ex ea commodo consequat. Duis aut" //  6
            "e irure dolor in reprehenderit in volupt" //  7
            "ate velit esse cillum dolore eu fugiat n" //  8
            "ulla pariatur. Excepteur sint occaecat c" //  9
            "upidatat non proident, sunt in culpa qui" // 10
            " officia deserunt mollit anim id est lab" // 11
            "orum."
        )
    {
    }

    std::string const&
    key(std::string& s) noexcept
    {
        s.clear();
        s.reserve(20);
        auto const append =
            [this, &s]()
            {
                s.push_back(
                    "0123456789"
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[
                        this->rand(62)]);
            };
        append();
        append();
        for(;;)
        {
            append();
            if(! rand(5) || s.size() >= s.max_size())
                return s;
        }
    }

    string_view
    string() noexcept
    {
        return {
            lorem_.data(),
            1 + rand(lorem_.size()) };
            //1 + rand(20) };
    }

    std::size_t
    integer() noexcept
    {
        return rand(std::numeric_limits<
            std::size_t>::max());
    }

    //---

private:
    void
    append_key() noexcept
    {
        auto const append =
            [this]()
            {
                s_.push_back(
                    "0123456789"
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[
                        rand(62)]);
            };
        s_.push_back('"');
        append();
        append();
        append();
        append();
        append();
        for(;;)
        {
            append();
            if(! rand(5))
                break;
        }
        s_.append("\" : ", 4);
    }

    void
    append_object()
    {
        s_.append("{\n", 2);
        ++depth_;

        s_.append(depth_ * indent_, ' ');
        append_key();
        append_value();
        while(rand(40))
        {
            s_.append(",\n", 2);
            s_.append(depth_ * indent_, ' ');
            append_key();
            append_value();
        }
        s_.push_back('\n');

        --depth_;
        s_.append(depth_ * indent_, ' ');
        s_.push_back('}');
    }

    void
    append_array()
    {
        s_.append("[\n", 2);
        ++depth_;

        s_.append(depth_ * indent_, ' ');
        append_value();
        while(rand(20))
        {
            s_.append(",\n", 2);
            s_.append(depth_ * indent_, ' ');
            append_value();
        }
        s_.push_back('\n');

        --depth_;
        s_.append(depth_ * indent_, ' ');
        s_.push_back(']');
    }

    void
    append_string()
    {
        auto const v = string();
        s_.reserve(
            s_.size() + 1 + v.size() + 1);
        s_.push_back('\"');
        s_.append(v.data(), v.size());
        s_.push_back('\"');
    }

    void
    append_integer()
    {
        auto const ns = std::to_string(
            rand(std::numeric_limits<int>::max()));
        s_.append(ns.c_str(), ns.size());
    }

    void
    append_boolean()
    {
        if(rand(2))
            s_.append("true", 4);
        else
            s_.append("false", 5);
    }

    void
    append_null()
    {
        s_.append("null", 4);
    }

    void
    append_value()
    {
        switch(rand(depth_ < max_depth_ ? 6 : 4))
        {
        case 5: return append_object();
        case 4: return append_array();
        case 3: return append_string();
        case 2: return append_integer();
        case 1: return append_boolean();
        case 0: return append_null();
        }
    }

public:
    void
    max_depth(int n)
    {
        max_depth_ = n;
    }

    string_view
    make_document()
    {
        s_.clear();
        depth_ = 0;
        append_array();
        return s_;
    }
};

//----------------------------------------------------------

// parse random documents
void
benchParse(
    string_view doc,
    any_impl& impl,
    int repeat = 1)
{
    using clock_type = std::chrono::steady_clock;
    auto const when = clock_type::now();
    dout << impl.name();
    while(repeat--)
        impl.work(doc);
    auto const elapsed =
        std::chrono::duration_cast<
            std::chrono::milliseconds>(
                clock_type::now() - when);
    dout <<
        " parse " <<
        doc.size() << " bytes"
        " in " << elapsed.count() << "ms" <<
        "\n";
    dout.flush();
}

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
    std::vector<std::string> const& vs,
    std::vector<std::unique_ptr<
        any_impl const>> const& vi)
{
    for(int i = 0; i < vs.size(); ++i)
    {
        dout << "File " + std::to_string(i+1) +
            " (" + std::to_string(vs[i].size()) + " bytes)" <<
            std::endl;
        for(int j = 0; j < vi.size(); ++j)
        {
            for(int k = 0; k < 3; ++k)
            {
                auto const when = clock_type::now();
                vi[j]->parse(vs[i], 100);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                dout << " " << vi[j]->name() << ": " <<
                    std::to_string(ms) << "ms" <<
                    std::endl;
            }
        }
    }
}

void
benchSerialize(
    std::vector<std::string> const& vs,
    std::vector<std::unique_ptr<
        any_impl const>> const& vi)
{
    for(int i = 0; i < vs.size(); ++i)
    {
        dout << "File " + std::to_string(i+1) +
            " (" + std::to_string(vs[i].size()) + " bytes)" <<
            std::endl;
        for(int j = 0; j < vi.size(); ++j)
        {
            for(int k = 0; k < 3; ++k)
            {
                auto const when = clock_type::now();
                vi[j]->serialize(vs[i], 1000);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                dout << " " << vi[j]->name() << ": " <<
                    std::to_string(ms) << "ms" <<
                    std::endl;
            }
        }
    }
}

int
main(
    int const argc,
    char const* const* const argv)
{
    std::vector<std::string> vs;
    if(argc > 1)
    {
        vs.reserve(argc - 1);
        for(int i = 1; i < argc; ++i)
            vs.emplace_back(load_file(argv[i]));
    }

    std::vector<std::unique_ptr<any_impl const>> vi;
    vi.reserve(10);
    vi.emplace_back(new boost_impl);
    vi.emplace_back(new rapidjson_impl);
    //vi.emplace_back(new nlohmann_impl);

    benchParse(vs, vi);
    //benchSerialize(vs, vi);

    return 0;
}

