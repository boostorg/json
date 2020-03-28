//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#include <boost/json/config.hpp>

#if defined(BOOST_JSON_USE_SSE2)
#  define RAPIDJSON_SSE2
#  define SSE2_ARCH_SUFFIX "/sse2"
#else
#  define SSE2_ARCH_SUFFIX ""
#endif

#include "lib/nlohmann/single_include/nlohmann/json.hpp"

#include "lib/rapidjson/include/rapidjson/rapidjson.h"
#include "lib/rapidjson/include/rapidjson/document.h"
#include "lib/rapidjson/include/rapidjson/writer.h"
#include "lib/rapidjson/include/rapidjson/stringbuffer.h"

#include <boost/json.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <cstdio>
#include <vector>

#include "test_suite.hpp"

/*  References

    https://github.com/nst/JSONTestSuite

    http://seriot.ch/parsing_json.php
*/

namespace boost {
namespace json {

using clock_type = std::chrono::steady_clock;

::test_suite::debug_stream dout(std::cerr);
std::stringstream strout;

#if defined(__clang__)
string_view toolset = "clang";
#elif defined(__GNUC__)
string_view toolset = "gcc";
#elif defined(_MSC_VER)
string_view toolset = "msvc";
#else
string_view toolset = "unknown";
#endif

#if BOOST_JSON_ARCH == 32
string_view arch = "x86" SSE2_ARCH_SUFFIX;
#elif BOOST_JSON_ARCH == 64
string_view arch = "x64" SSE2_ARCH_SUFFIX;
#else
#error Unknown architecture.
#endif

//----------------------------------------------------------

struct file_item
{
    string_view name;
    std::string text;
};

using file_list = std::vector<file_item>;

class any_impl
{
public:
    virtual ~any_impl() = default;
    virtual string_view name() const noexcept = 0;
    virtual void parse(string_view s, std::size_t repeat) const = 0;
    virtual void serialize(string_view s, std::size_t repeat) const = 0;
};

using impl_list = std::vector<
    std::unique_ptr<any_impl const>>;

std::string
load_file(char const* path)
{
    FILE* f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    auto const size = ftell(f);
    std::string s;
    s.resize(size);
    fseek(f, 0, SEEK_SET);
    auto const nread =
        fread(&s[0], 1, size, f);
    s.resize(nread);
    fclose(f);
    return s;
}

struct sample
{
    std::size_t calls;
    std::size_t millis;
    std::size_t mbs;
};

// Returns the number of invocations per second
template<
    class Rep,
    class Period,
    class F>
sample
run_for(
    std::chrono::duration<
        Rep, Period> interval,
    F&& f)
{
    using clock_type =
        std::chrono::high_resolution_clock;
    auto const when = clock_type::now();
    auto elapsed = clock_type::now() - when;
    std::size_t n = 0;
    do
    {
        f();
        elapsed = clock_type::now() - when;
        ++n;
    }
    while(elapsed < interval);
    return { n, static_cast<std::size_t>(
        std::chrono::duration_cast<
            std::chrono::milliseconds>(
                elapsed).count()), 0 };
}

void
bench(
    string_view verb,
    file_list const& vf,
    impl_list const& vi)
{
    std::size_t Trials = 6;

    std::vector<sample> trial;
    for(unsigned i = 0; i < vf.size(); ++i)
    {
        for(unsigned j = 0; j < vi.size(); ++j)
        {
            trial.clear();
            std::size_t repeat = 1000;
            for(unsigned k = 0; k < Trials; ++k)
            {
                auto result = run_for(
                    std::chrono::seconds(5),
                    [&]
                    {
                        if(verb == "Parse")
                            vi[j]->parse(
                                vf[i].text,
                                repeat);
                        else if(verb == "Serialize")
                            vi[j]->serialize(
                                vf[i].text,
                                repeat);
                    });
                result.calls *= repeat;
                result.mbs = static_cast<
                    std::size_t>(( 0.5 + 1000.0 *
                        result.calls *
                        vf[i].text.size() /
                        result.millis / 1024 / 1024));
                dout <<
                    verb << " " << vf[i].name << "," <<
                    toolset << " " << arch << "," <<
                    vi[j]->name() << "," <<
                    result.calls << "," <<
                    result.millis << "," <<
                    result.mbs <<
                    "\n";
                trial.push_back(result);
                // adjust repeat to avoid overlong tests
                repeat = 250 * result.calls / result.millis;
            }

            // clean up the samples
            std::sort(
                trial.begin(),
                trial.end(),
                []( sample const& lhs,
                    sample const& rhs)
                {
                    return lhs.mbs < rhs.mbs;
                });
            if(Trials >= 6)
            {
                // discard worst 2
                trial.erase(
                    trial.begin(),
                    trial.begin() + 2);
                // discard best 1
                trial.resize(3);
            }
            else if(Trials > 3)
            {
                trial.erase(
                    trial.begin(),
                    trial.begin() + Trials - 3);
            }
            // average
            auto const calls =
                std::accumulate(
                trial.begin(), trial.end(),
                std::size_t{},
                []( std::size_t lhs,
                    sample const& rhs)
                {
                    return lhs + rhs.calls;
                });
            auto const millis =
                std::accumulate(
                trial.begin(), trial.end(),
                std::size_t{},
                []( std::size_t lhs,
                    sample const& rhs)
                {
                    return lhs + rhs.millis;
                });
            auto const mbs = static_cast<
                std::size_t>(( 0.5 + 1000.0 *
                calls * vf[i].text.size() /
                    millis / 1024 / 1024));
            strout <<
                verb << " " << vf[i].name << "," <<
                toolset << " " << arch << "," <<
                vi[j]->name() << "," <<
                mbs <<
                "\n";
        }
    }
}

//----------------------------------------------------------

class boost_default_impl : public any_impl
{
public:
    string_view
    name() const noexcept override
    {
        return "boost";
    }

    void
    parse(
        string_view s,
        std::size_t repeat) const override
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
        std::size_t repeat) const override
    {
        auto jv = json::parse(s);
        serializer sr;
        string out;
        out.reserve(512);
        while(repeat--)
        {
            sr.reset(jv);
            out.clear();
            for(;;)
            {
                out.grow(sr.read(
                    out.end(),
                    out.capacity() -
                        out.size()));
                if(sr.is_done())
                    break;
                out.reserve(
                    out.capacity() + 1);
            }
        }
    }
};

//----------------------------------------------------------

class boost_pool_impl : public any_impl
{
public:
    string_view
    name() const noexcept override
    {
        return "boost (pool)";
    }

    void
    parse(
        string_view s,
        std::size_t repeat) const override
    {
        parser p;
        while(repeat--)
        {
            scoped_storage<pool> ss;
            p.start(ss);
            error_code ec;
            p.finish(s.data(), s.size(), ec);
            auto jv = p.release();
        }
    }

    void
    serialize(
        string_view s,
        std::size_t repeat) const override
    {
        scoped_storage<pool> sp;
        auto jv = json::parse(s, sp);
        serializer sr;
        string out;
        out.reserve(512);
        while(repeat--)
        {
            sr.reset(jv);
            out.clear();
            for(;;)
            {
                out.grow(sr.read(
                    out.end(),
                    out.capacity() -
                        out.size()));
                if(sr.is_done())
                    break;
                out.reserve(
                    out.capacity() + 1);
            }
        }
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
        void on_object_end(std::size_t, error_code&) override {}
        void on_array_begin(error_code&) override {}
        void on_array_end(std::size_t, error_code&) override {}
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
        return "boost.vec";
    }

    void
    parse(
        string_view s,
        std::size_t repeat) const override
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
        std::size_t repeat) const override
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
        void on_object_end(std::size_t, error_code&) override {}
        void on_array_begin(error_code&) override {}
        void on_array_end(std::size_t, error_code&) override {}
        void on_key_part(string_view, error_code&) override {}
        void on_key( string_view, error_code&) override {}
        void on_string_part(string_view, error_code&) override {}
        void on_string(string_view, error_code&) override {}
        void on_int64(std::int64_t, error_code&) override {}
        void on_uint64(std::uint64_t, error_code&) override {}
        void on_double(double, error_code&) override {}
        void on_bool(bool, error_code&) override {}
        void on_null(error_code&) override {}
        void reset()
        {
            basic_parser::reset();
        }
    };

public:
    string_view
    name() const noexcept override
    {
        return "boost.null";
    }

    void
    parse(
        string_view s,
        std::size_t repeat) const override
    {
        null_parser p;
        while(repeat--)
        {
            p.reset();
            error_code ec;
            p.finish(s.data(), s.size(), ec);
        }
    }

    void
    serialize(
        string_view, std::size_t) const override
    {
    }
};

//----------------------------------------------------------

struct rapidjson_crt_impl : public any_impl
{
    string_view
    name() const noexcept override
    {
        return "rapidjson";
    }

    void
    parse(
        string_view s, std::size_t repeat) const override
    {
        using namespace rapidjson;
        while(repeat--)
        {
            CrtAllocator alloc;
            GenericDocument<
                UTF8<>, CrtAllocator> d(&alloc);
            d.Parse(s.data(), s.size());
        }
    }

    void
    serialize(string_view s, std::size_t repeat) const override
    {
        using namespace rapidjson;
        CrtAllocator alloc;
        GenericDocument<
            UTF8<>, CrtAllocator> d(&alloc);
        d.Parse(s.data(), s.size());
        rapidjson::StringBuffer st;
        while(repeat--)
        {
            st.Clear();
            rapidjson::Writer<
                rapidjson::StringBuffer> wr(st);
            d.Accept(wr);
        }
    }
};

struct rapidjson_memory_impl : public any_impl
{
    string_view
    name() const noexcept override
    {
        return "rapidjson (pool)";
    }

    void
    parse(
        string_view s, std::size_t repeat) const override
    {
        while(repeat--)
        {
            rapidjson::Document d;
            d.Parse(s.data(), s.size());
        }
    }

    void
    serialize(string_view s, std::size_t repeat) const override
    {
        rapidjson::Document d;
        d.Parse(s.data(), s.size());
        rapidjson::StringBuffer st;
        while(repeat--)
        {
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
    parse(string_view s, std::size_t repeat) const override
    {
        while(repeat--)
            nlohmann::json::parse(
                s.begin(), s.end());
    }

    void
    serialize(string_view s, std::size_t repeat) const override
    {
        auto jv = nlohmann::json::parse(
            s.begin(), s.end());
        while(repeat--)
            auto st = jv.dump();

    }
};

//----------------------------------------------------------

} // json
} // boost

int
main(
    int const argc,
    char const* const* const argv)
{
    using namespace boost::json;
    file_list vf;
    if(argc > 1)
    {
        vf.reserve(argc - 1);
        for(int i = 1; i < argc; ++i)
            vf.emplace_back(
                file_item{argv[i],
                load_file(argv[i])});
    }
    else
    {
        std::cerr <<
            "Usage: bench <file>...";
    }

    try
    {
/*
        strings.json integers-32.json integers-64.json twitter.json small.json array.json random.json citm_catalog.json canada.json
*/
        impl_list vi;
        //vi.emplace_back(new boost_null_impl);
        //vi.emplace_back(new boost_vec_impl);
        vi.emplace_back(new boost_pool_impl);
        vi.emplace_back(new boost_default_impl);
        vi.emplace_back(new rapidjson_memory_impl);
        vi.emplace_back(new rapidjson_crt_impl);
        vi.emplace_back(new nlohmann_impl);

        bench("Parse", vf, vi);
        bench("Serialize", vf, vi);

        dout << "\n" << strout.str();
    }
    catch(system_error const& se)
    {
        dout << se.what() << std::endl;
    }

    return 0;
}
