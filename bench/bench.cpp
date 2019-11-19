//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
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
    
    virtual void parse_reuse(string_view s, int repeat) const
    {
        parse(s, repeat);
    }
    
    virtual void serialize(string_view s, int repeat) const = 0;
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
    parse_reuse(
        string_view s,
        int repeat) const override
    {
        parser p;
        while(repeat--)
        {
            p.start();
            error_code ec;
            p.finish(s.data(), s.size(), ec);
            auto jv = p.release();
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

class boost_pool_impl : public any_impl
{
public:
    string_view
    name() const noexcept override
    {
        return "boost(pool)";
    }

    void
    parse(
        string_view s,
        int repeat) const override
    {
        while(repeat--)
        {
            scoped_storage<
                pool> ss;
            json::parse(s, ss);
        }
    }

    void
    parse_reuse(
        string_view s,
        int repeat) const override
    {
        scoped_storage<
            pool> ss;
        parser p;
        while(repeat--)
        {
            p.start(ss);
            error_code ec;
            p.finish(s.data(), s.size(), ec);
            auto jv = p.release();
        }
    }

    void
    serialize(
        string_view s,
        int repeat) const override
    {
        scoped_storage<
            pool> ss;
        auto jv = json::parse(s, ss);
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

        vec_parser() {}
        ~vec_parser() {}
        void on_document_begin(error_code&) override {}
        void on_document_end(error_code&) override {}
        void on_object_begin(error_code&) override {}
        void on_object_end(error_code&) override {}
        void on_array_begin(error_code&) override {}
        void on_array_end(error_code&) override {}
        void on_key_part(string_view, error_code&) override {}
        void on_key( string_view, error_code&) override {}
        void on_string_part(string_view, error_code&) override {}
        void on_string(string_view, error_code&) override {}
        void on_int64(std::int64_t, error_code&) override {}
        void on_uint64(std::uint64_t, error_code&) override {}
        void on_double(double d, error_code&) override
        {
            vec_.push_back(d);
        }
        void on_bool(bool, error_code&) override {}
        void on_null(error_code&) override {}
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
            if(! ec)
                p.finish(ec);
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

class boost_null_impl : public any_impl
{
    struct null_parser : basic_parser
    {
        null_parser() {}
        ~null_parser() {}
        void on_document_begin(error_code&) override {}
        void on_document_end(error_code&) override {}
        void on_object_begin(error_code&) override {}
        void on_object_end(error_code&) override {}
        void on_array_begin(error_code&) override {}
        void on_array_end(error_code&) override {}
        void on_key_part(string_view, error_code&) override {}
        void on_key( string_view, error_code&) override {}
        void on_string_part(string_view, error_code&) override {}
        void on_string(string_view, error_code&) override {}
        void on_int64(std::int64_t, error_code&) override {}
        void on_uint64(std::uint64_t, error_code&) override {}
        void on_double(double, error_code&) override {}
        void on_bool(bool, error_code&) override {}
        void on_null(error_code&) override {}
    };

public:
    string_view
    name() const noexcept override
    {
        return "boost(null)";
    }

    void
    parse(
        string_view s,
        int repeat) const override
    {
        while(repeat--)
        {
            error_code ec;
            null_parser p;
            p.write(s.data(), s.size(), ec);
            if(! ec)
                p.finish(ec);
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

struct rapidjson_crt_impl : public any_impl
{
    string_view
    name() const noexcept override
    {
        return "rapidjson(crt)";
    }

    void
    parse(string_view s, int repeat) const override
    {
        using namespace rapidjson;
        CrtAllocator alloc;
        while(repeat--)
        {
            GenericDocument<
                UTF8<>, CrtAllocator> d(&alloc);
            d.Parse(s.data(), s.size());
        }
    }

    void
    parse_reuse(
        string_view s, int repeat) const override
    {
        using namespace rapidjson;
        CrtAllocator alloc;
        GenericDocument<
            UTF8<>, CrtAllocator> d(&alloc);
        while(repeat--)
        {
            d.Clear();
            d.Parse(s.data(), s.size());
        }
    }

    void
    serialize(string_view s, int repeat) const override
    {
        using namespace rapidjson;
        CrtAllocator alloc;
        GenericDocument<
            UTF8<>, CrtAllocator> d(&alloc);
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

struct rapidjson_pool_impl : public any_impl
{
    string_view
    name() const noexcept override
    {
        return "rapidjson(pool)";
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
    parse_reuse(
        string_view s, int repeat) const override
    {
        rapidjson::Document d;
        while(repeat--)
        {
            d.Clear();
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
    serialize(string_view s, int repeat) const override
    {
        auto jv = nlohmann::json::parse(
            s.begin(), s.end());
        while(repeat--)
            auto st = jv.dump();

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
benchParseSmall(
    std::vector<std::unique_ptr<
        any_impl const>> const& vi)
{
    string_view text =
R"xx({
    "glossary": {
        "title": "example glossary",
		"GlossDiv": {
            "title": "S",
			"GlossList": {
                "GlossEntry": {
                    "ID": "SGML",
					"SortAs": "SGML",
					"GlossTerm": "Standard Generalized Markup Language",
					"Acronym": "SGML",
					"Abbrev": "ISO 8879:1986",
					"GlossDef": {
                        "para": "A meta-markup language, used to create markup languages such as DocBook.",
						"GlossSeeAlso": ["GML", "XML"]
                    },
					"GlossSee": "markup"
                }
            }
        }
    }
})xx";
    auto const Repeat = 500000U;
    {
        dout <<
            "Parse small JSON (" <<
                text.size() << " bytes)" <<
                std::endl;
        for(unsigned j = 0; j < vi.size(); ++j)
        {
            for(unsigned k = 0; k < 10; ++k)
            {
                auto const when = clock_type::now();
                vi[j]->parse_reuse(text, 500000);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                if(k > 4)
                    dout << vi[j]->name() << ", " <<
                        std::to_string(ms) << "ms, " <<
                        (Repeat * text.size() / ms / 1024) << "Mb/s" <<
                        std::endl;
            }
        }
        dout << std::endl;
    }
}

void
benchParse(
    file_list const& vs,
    std::vector<std::unique_ptr<
        any_impl const>> const& vi)
{
    auto const Repeat = 250U;
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
                vi[j]->parse(vs[i].text, Repeat);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                if(k > 4)
                    dout << vi[j]->name() << ", " <<
                        std::to_string(ms) << "ms, " <<
                        (Repeat * vs[i].text.size() / ms / 1024) << "Mb/s" <<
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
    auto const Repeat = 1000U;
    for(unsigned i = 0; i < vs.size(); ++i)
    {
        dout <<
            "Serialize File " << std::to_string(i+1) <<
                " " << vs[i].name << " (" <<
                vs[i].text.size() << " bytes)" <<
                std::endl;
        for(unsigned j = 0; j < vi.size(); ++j)
        {
            for(unsigned k = 0; k < 10; ++k)
            {
                auto const when = clock_type::now();
                vi[j]->serialize(vs[i].text, Repeat);
                auto const ms = std::chrono::duration_cast<
                    std::chrono::milliseconds>(
                    clock_type::now() - when).count();
                if(k > 4)
                    dout << vi[j]->name() << ", " <<
                        std::to_string(ms) << "ms, " <<
                        (Repeat * vs[i].text.size() / ms / 1024) << "Mb/s" <<
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
#if 1
        vi.emplace_back(new boost_null_impl);
        vi.emplace_back(new boost_default_impl);
        vi.emplace_back(new boost_pool_impl);
        //vi.emplace_back(new boost_vec_impl);
        vi.emplace_back(new rapidjson_crt_impl);
        vi.emplace_back(new rapidjson_pool_impl);
        //vi.emplace_back(new nlohmann_impl);
#else
        vi.emplace_back(new boost_pool_impl);
#endif

        benchParseSmall(vi);
        //benchParse(vs, vi);
        //benchSerialize(vs, vi);
    }
    catch(system_error const& se)
    {
        dout << se.what() << std::endl;
    }

    return 0;
}
