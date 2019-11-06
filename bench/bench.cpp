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
        std::vector<double> vec_;
        double d_ = 0;

        vec_parser()
        {
        }

        ~vec_parser()
        {
        }

        void
        on_document_begin(
            error_code&) override
        {
        }

        void
        on_document_end(
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
        on_key_part(
            string_view,
            error_code&) override
        {
        }

        void
        on_key(
            string_view,
            error_code&) override
        {
        }
        
        void
        on_string_part(
            string_view,
            error_code&) override
        {
        }

        void
        on_string(
            string_view,
            error_code&) override
        {
        }

        void
        on_int64(
            int64_t,
            error_code&) override
        {
        }

        void
        on_uint64(
            uint64_t,
            error_code&) override
        {
        }

        void
        on_double(
            double d,
            error_code&) override
        {
            vec_.push_back(d);
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
    };

public:
    string_view
    name() const noexcept override
    {
        return "boost(vec)";
    }

    void
    parse(
        string_view s,
        int repeat) const override
    {
        while(repeat--)
        {
            error_code ec;
            vec_parser p;
            p.write(s.data(), s.size(), ec);
        }
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
                if(k >= 0)
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
            for(unsigned k = 0; k < 5; ++k)
            {
                auto const when = clock_type::now();
                vi[j]->serialize(vs[i].text, 1000);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                if(k > 9)
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
        //vi.emplace_back(new boost_default_impl);
        vi.emplace_back(new boost_impl);
        //vi.emplace_back(new boost_vec_impl);
        vi.emplace_back(new rapidjson_impl);
        //vi.emplace_back(new nlohmann_impl);

        benchParse(vs, vi);
        //benchSerialize(vs, vi);
    }
    catch(system_error const& se)
    {
        dout << se.what() << std::endl;
    }

    return 0;
}
