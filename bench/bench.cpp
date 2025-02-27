//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/detail/config.hpp>

#if defined(BOOST_JSON_USE_SSE2)
#  define RAPIDJSON_SSE2
#  define SSE2_ARCH_SUFFIX "/sse2"
#else
#  define SSE2_ARCH_SUFFIX ""
#endif

#ifdef BOOST_JSON_HAS_NLOHMANN_JSON
# include "lib/nlohmann/single_include/nlohmann/json.hpp"
#endif // BOOST_JSON_HAS_NLOHMANN_JSON

#ifdef BOOST_JSON_HAS_RAPIDJSON
# include "lib/rapidjson/include/rapidjson/rapidjson.h"
# include "lib/rapidjson/include/rapidjson/document.h"
# include "lib/rapidjson/include/rapidjson/writer.h"
# include "lib/rapidjson/include/rapidjson/stringbuffer.h"
#endif // BOOST_JSON_HAS_RAPIDJSON

#include <boost/json.hpp>
#include <boost/json/basic_parser_impl.hpp>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include "test_suite.hpp"
#include "apache_builds.hpp"
#include "canada.hpp"
#include "citm_catalog.hpp"

/*  References

    https://github.com/nst/JSONTestSuite

    http://seriot.ch/parsing_json.php
*/

std::string s_tests = "ps";
std::string s_impls = "busodrn";
std::size_t s_trials = 6;
std::string s_branch = "";
std::string s_alloc = "p";
std::string s_num_mode = "i";
std::string s_file_io = "n";
std::string s_conversion = "n";

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

#if defined(BOOST_DESCRIBE_CXX14)
using supported_files = mp11::mp_list<
    apache_builds_support,
    canada_support,
    citm_catalog_support >;
#else
struct mock_type_support
{
    using type = int;
    static constexpr char const* const name = "";
};
using supported_files = mp11::mp_list<mock_type_support>;
#endif // defined(BOOST_DESCRIBE_CXX14)

using supported_file_count = mp11::mp_size<supported_files>;

template< class I >
using supported_file_at = mp11::mp_at<supported_files, I>;

struct file_matcher
{
    file_item const& fi;
    int& result;

    template< class I >
    void operator()(I)
    {
        using supported_file = supported_file_at<I>;
        std::size_t const name_len = std::strlen(supported_file::name);

        std::size_t pos = fi.name.rfind(
            supported_file::name, string_view::npos, name_len);
        if(pos == string_view::npos)
            return;
        if( pos + name_len != fi.name.size() )
            return;
        if( pos != 0 && fi.name[pos - 1] != '/'
            && fi.name[pos - 1] != '\\' )
            return;

        result = static_cast<int>(I::value);
    }
};

int
find_supported_file(file_item const& fi)
{
    int result = -1;
    mp11::mp_for_each< mp11::mp_iota<supported_file_count> >(
        file_matcher{fi, result} );
    return result;
}

struct convert_from_value
{
    value const& jv;

    template< class I >
    void operator()(I) const
    {
        using data_type = typename supported_file_at<I>::type;
        auto data = value_to<data_type>(jv);
        (void)data;
    }
};

struct delete_data
{
    struct deleter
    {
        void* data;

        template< class I >
        void operator()(I) const
        {
            using data_type = typename supported_file_at<I>::type;
            delete reinterpret_cast<data_type*>(data);
        }
    };

    int index;

    void operator()(void* data)
    {
        if( index >= 0 )
            mp11::mp_with_index<supported_file_count>( index, deleter{data} );
    }
};
using data_holder = std::unique_ptr<void, delete_data>;

struct data_constructor
{
    data_holder& data;
    value const& jv;

    template< class I >
    void operator()(I) const
    {
        using data_type = typename supported_file_at<I>::type;
        auto const ptr = new data_type{};
        data.reset(ptr);
        *ptr = value_to<data_type>(jv);
    }
};

data_holder
construct_data(int file_index, value const& jv)
{
    data_holder result( nullptr, {file_index} );
    if( file_index >= 0 )
        mp11::mp_with_index<supported_file_count>(
            file_index, data_constructor{result, jv} );
    return result;
}

struct convert_to_value
{
    data_holder& data;
    value& jv;

    template< class I >
    BOOST_NOINLINE
    void operator()(I) const
    {
        using data_type = typename supported_file_at<I>::type;
        value_from(
            *reinterpret_cast<data_type*>( data.get() ), jv);
    }
};

class any_impl
{
    std::string name_;
    parse_options popts_;
    bool with_file_io_;
    bool with_conversion_;
    mutable int file_index_ = -1;

    virtual
    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const = 0;

    virtual
    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const = 0;

    virtual
    clock_type::duration
    serialize_string(file_item const& fi, std::size_t repeat) const = 0;

protected:
    clock_type::duration
    skip() const
    {
        return clock_type::duration::zero();
    }

    parse_options const&
    get_parse_options() const noexcept
    {
        return popts_;
    }

    int
    file_index() const noexcept
    {
        return file_index_;
    }

public:
    using operation = auto (any_impl::*)
        (file_item const& fi, std::size_t repeat) const
        -> clock_type::duration;

    any_impl(
        string_view base_name,
        string_view flavor,
        bool is_boost,
        bool is_pool,
        bool with_file_io,
        bool with_conversion,
        parse_options const& popts)
        : popts_(popts)
        , with_file_io_(with_file_io)
        , with_conversion_(with_conversion)
    {
        std::string extra;
        switch( popts_.numbers )
        {
        case number_precision::precise:
            extra = "precise numbers";
            break;
        case number_precision::none:
            extra = "no numbers";
            break;
        default:
            break;
        }

        if( with_conversion )
        {
            if( !extra.empty() )
                extra = '+' + extra;
            extra = "conversion" + extra;
        }

        if( with_file_io_ )
        {
            if( !extra.empty() )
                extra += '+';
            extra += "file IO";
        }

        if( is_pool )
        {
            if( !extra.empty() )
                extra = '+' + extra;
            extra = "pool" + extra;
        }

        if( !flavor.empty() )
        {
            if( !extra.empty() )
                extra = '+' + extra;
            extra.insert( extra.begin(), flavor.begin(), flavor.end() );
        }

        if( !extra.empty() )
            extra = " (" + extra + ')';

        if( is_boost && !s_branch.empty() )
            extra += ' ' + s_branch;

        name_ = base_name;
        name_ += extra;
    }

    virtual ~any_impl() = default;

    clock_type::duration
    bench(string_view verb, file_item const& fi, std::size_t repeat) const
    {
        if( with_conversion_ )
        {
            if( ( file_index_ = find_supported_file(fi) ) < 0 )
                return skip();
        }
        else
        {
            file_index_ = -1;
        }

        if(verb == "Parse")
        {
            if( with_file_io_ )
                return parse_file(fi, repeat);
            else
                return parse_string(fi, repeat);
        }
        else
        {
            BOOST_ASSERT( verb == "Serialize" );
            if( with_file_io_ )
                return skip();
            else
                return serialize_string(fi, repeat);
        }
    }

    string_view
    name() const noexcept
    {
        return name_;
    }
};

using impl_ptr = std::unique_ptr<any_impl const>;
using impl_list = std::vector<impl_ptr>;

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
run_for(std::chrono::duration<Rep, Period> interval, F&& f)
{
    clock_type::duration elapsed(0);
    std::size_t n = 0;
    do
    {
        elapsed += f();
        ++n;
    }
    while(elapsed < interval);
    return { n, static_cast<std::size_t>(
        std::chrono::duration_cast<
            std::chrono::milliseconds>(
                elapsed).count()), 0 };
}

std::size_t
megabytes_per_second(
    file_item const& file, std::size_t calls, std::size_t millis)
{
    double result = file.text.size();
    result /= 1024 * 1024; // size in megabytes
    result *= calls;
    result /= millis; // mb per ms
    result *= 1000; // mb per s
    return static_cast<std::size_t>(0.5 + result); // round up
}

std::ostream&
print_prefix(
    std::ostream& os, file_item const& file, any_impl const& impl,
    string_view verb)
{
    return os << verb << " " << file.name << "," << toolset << " " <<
        arch << "," << impl.name();
}

void
bench(
    string_view verb,
    file_list const& vf,
    impl_list const& vi, std::size_t Trials)
{
    std::vector<sample> trial;
    for(unsigned i = 0; i < vf.size(); ++i)
    {
        for(unsigned j = 0; j < vi.size(); ++j)
        {
            trial.clear();
            std::size_t repeat = 1;

            auto const f = [&]{
                return vi[j].get()->bench(verb, vf[i], repeat);
            };

            // helps with the caching, which reduces noise; also, we determine
            // if this configuration should be skipped altogether
            auto const elapsed = f();
            if( elapsed == std::chrono::milliseconds::zero() )
            {
                print_prefix(dout, vf[i], *vi[j], verb)
                    << "," << "N/A"
                    << "," << "N/A"
                    << "," << "N/A"
                    << "\n";
                print_prefix(strout, vf[i], *vi[j], verb)
                    << "," << "N/A" << "\n";
                continue;
            }

            repeat = 1000;
            for(unsigned k = 0; k < Trials; ++k)
            {
                auto result = run_for(std::chrono::seconds(5), f);
                result.calls *= repeat;
                result.mbs = megabytes_per_second(
                    vf[i], result.calls, result.millis);
                print_prefix(dout, vf[i], *vi[j], verb)
                    << "," << result.calls
                    << "," << result.millis
                    << "," << result.mbs
                    << "\n";
                trial.push_back(result);
                // adjust repeat to avoid overlong tests
                repeat = 250 * result.calls / result.millis;
            }

            // clean up the samples
            std::sort(
                trial.begin(),
                trial.end(),
                [](sample const& lhs, sample const& rhs)
                {
                    return lhs.mbs < rhs.mbs;
                });
            if(Trials >= 6)
            {
                // discard worst 2
                trial.erase(trial.begin(), trial.begin() + 2);
                // discard best 1
                trial.resize(trial.size() - 1);
            }
            else if(Trials > 3)
            {
                trial.erase(trial.begin(), trial.begin() + Trials - 3);
            }
            // average
            auto const calls = std::accumulate(
                trial.begin(), trial.end(),
                std::size_t{},
                [](std::size_t lhs, sample const& rhs)
                {
                    return lhs + rhs.calls;
                });
            auto const millis = std::accumulate(
                trial.begin(), trial.end(),
                std::size_t{},
                [](std::size_t lhs, sample const& rhs)
                {
                    return lhs + rhs.millis;
                });
            auto const mbs = megabytes_per_second(vf[i], calls, millis);
            print_prefix(strout, vf[i], *vi[j], verb) << "," << mbs << "\n";
        }
    }
}

//----------------------------------------------------------

class base_boost_impl : public any_impl
{
protected:
    bool is_pool_;

    BOOST_FORCEINLINE
    void
    maybe_convert_from_value(value const& jv) const
    {
        if( file_index() >= 0 )
            mp11::mp_with_index<supported_file_count>(
                file_index(), convert_from_value{jv});
    }

    BOOST_FORCEINLINE
    void
    maybe_convert_to_value(data_holder& data, value& jv) const
    {
        if( file_index() >= 0 )
            mp11::mp_with_index<supported_file_count>(
                file_index(), convert_to_value{data, jv});
    }

public:
    base_boost_impl(
        string_view flavor,
        bool is_pool,
        bool with_file_io,
        bool with_conversion,
        parse_options const& popts)
        : any_impl(
            "boost",
            flavor,
            true,
            is_pool,
            with_file_io,
            with_conversion,
            popts)
        , is_pool_(is_pool)
    {}
};

//----------------------------------------------------------

class boost_impl : public base_boost_impl
{
public:
    boost_impl(
        bool is_pool,
        bool with_file_io,
        bool with_conversion,
        parse_options const& popts)
        : base_boost_impl("", is_pool, with_file_io, with_conversion, popts)
    {}

    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const override
    {
        auto const start = clock_type::now();
        parser p( {}, get_parse_options() );
        while(repeat--)
        {
            monotonic_resource mr;
            storage_ptr sp;
            if( is_pool_ )
                sp = &mr;
            p.reset( std::move(sp) );

            p.write( fi.text.data(), fi.text.size() );
            maybe_convert_from_value( p.release() );
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const override
    {
        auto const start = clock_type::now();
        stream_parser p( {}, get_parse_options() );
        char s[ BOOST_JSON_STACK_BUFFER_SIZE];
        while(repeat--)
        {
            monotonic_resource mr;
            storage_ptr sp;
            if( is_pool_ )
                sp = &mr;
            p.reset( std::move(sp) );

            FILE* f = fopen(fi.name.data(), "rb");

            while( true )
            {
                std::size_t const sz = fread(s, 1, sizeof(s), f);
                if( ferror(f) )
                    break;

                p.write(s, sz);

                if( feof(f) )
                    break;
            }

            p.finish();
            auto jv = p.release();

            fclose(f);

            maybe_convert_from_value( jv );
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize_string(file_item const& fi, std::size_t repeat) const override
    {
        monotonic_resource mr;
        storage_ptr sp;
        if( is_pool_ )
            sp = &mr;
        value jv = json::parse( fi.text, std::move(sp) );
        data_holder data = construct_data(file_index(), jv);

        auto const start = clock_type::now();
        serializer sr;
        string out;
        out.reserve(512);
        while(repeat--)
        {
            maybe_convert_to_value(data, jv);
            sr.reset(&jv);
            out.clear();
            for(;;)
            {
                out.grow(sr.read(
                    out.end(),
                    out.capacity() -
                        out.size()).size());
                if(sr.done())
                    break;
                out.reserve(
                    out.capacity() + 1);
            }
        }
        return clock_type::now() - start;
    }
};

//----------------------------------------------------------

class boost_null_impl : public base_boost_impl
{
    struct null_parser
    {
        struct handler
        {
            constexpr static std::size_t max_object_size = std::size_t(-1);
            constexpr static std::size_t max_array_size = std::size_t(-1);
            constexpr static std::size_t max_key_size = std::size_t(-1);
            constexpr static std::size_t max_string_size = std::size_t(-1);

            bool on_document_begin(system::error_code&) { return true; }
            bool on_document_end(system::error_code&) { return true; }
            bool on_object_begin(system::error_code&) { return true; }
            bool on_object_end(std::size_t, system::error_code&) { return true; }
            bool on_array_begin(system::error_code&) { return true; }
            bool on_array_end(std::size_t, system::error_code&) { return true; }
            bool on_key_part(string_view, std::size_t, system::error_code&) { return true; }
            bool on_key( string_view, std::size_t, system::error_code&) { return true; }
            bool on_string_part(string_view, std::size_t, system::error_code&) { return true; }
            bool on_string(string_view, std::size_t, system::error_code&) { return true; }
            bool on_number_part(string_view, system::error_code&) { return true; }
            bool on_int64(std::int64_t, string_view, system::error_code&) { return true; }
            bool on_uint64(std::uint64_t, string_view, system::error_code&) { return true; }
            bool on_double(double, string_view, system::error_code&) { return true; }
            bool on_bool(bool, system::error_code&) { return true; }
            bool on_null(system::error_code&) { return true; }
            bool on_comment_part(string_view, system::error_code&) { return true; }
            bool on_comment(string_view, system::error_code&) { return true; }
        };

        basic_parser<handler> p_;

        null_parser(parse_options const& popts)
            : p_(popts)
        {
        }

        void
        reset()
        {
            p_.reset();
        }

        std::size_t
        write(
            char const* data,
            std::size_t size,
            system::error_code& ec)
        {
            auto const n = p_.write_some(
                false, data, size, ec);
            if(! ec && n < size)
                ec = error::extra_data;
            return n;
        }

        std::size_t
        write_some(
            char const* data,
            std::size_t size,
            system::error_code& ec)
        {
            return p_.write_some(
                true, data, size, ec);
        }

        void
        finish(system::error_code& ec)
        {
            p_.write_some(false, nullptr, 0, ec);
        }
    };

public:
    boost_null_impl(
        bool with_file_io, bool with_conversion, parse_options const& popts)
        : base_boost_impl("null", false, with_file_io, with_conversion, popts)
    {}

    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        auto const start = clock_type::now();
        null_parser p( get_parse_options() );
        while(repeat--)
        {
            p.reset();
            system::error_code ec;
            p.write(fi.text.data(), fi.text.size(), ec);
            if( ec.failed() )
                throw system::system_error( ec );
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        auto const start = clock_type::now();
        null_parser p( get_parse_options() );
        char s[ BOOST_JSON_STACK_BUFFER_SIZE];
        while(repeat--)
        {
            p.reset();

            FILE* f = fopen(fi.name.data(), "rb");

            system::error_code ec;
            while( true )
            {
                std::size_t const sz = fread(s, 1, sizeof(s), f);
                if( ferror(f) )
                {
                    ec = std::io_errc::stream;
                    break;
                }

                p.write_some( s, sz, ec );
                if( ec.failed() )
                    break;

                if( feof(f) )
                    break;
            }

            fclose(f);

            if( ec.failed() )
                throw system::system_error( ec );
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize_string(file_item const&, std::size_t) const override
    {
        return skip();
    }
};

//----------------------------------------------------------

class boost_simple_impl : public base_boost_impl
{
public:
    boost_simple_impl(
        bool is_pool,
        bool with_file_io,
        bool with_conversion,
        parse_options const& popts)
        : base_boost_impl(
            "convenient", is_pool, with_file_io, with_conversion, popts)
    {}

    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const override
    {
        auto const start = clock_type::now();
        while(repeat--)
        {
            monotonic_resource mr;
            storage_ptr sp;
            if( is_pool_ )
                sp = &mr;

            auto jv = json::parse(
                fi.text, std::move(sp), get_parse_options() );
            maybe_convert_from_value(jv);
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const override
    {
        auto const start = clock_type::now();
        while(repeat--)
        {
            std::ifstream is( fi.name, std::ios::in | std::ios::binary );

            monotonic_resource mr;
            storage_ptr sp;
            if( is_pool_ )
                sp = &mr;

            auto jv = json::parse( is, std::move(sp), get_parse_options() );
            maybe_convert_from_value(jv);
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize_string(file_item const& fi, std::size_t repeat) const override
    {
        monotonic_resource mr;
        storage_ptr sp;
        if( is_pool_ )
            sp = &mr;
        auto jv = json::parse( fi.text, std::move(sp) );
        auto data = construct_data(file_index(), jv);

        auto const start = clock_type::now();
        std::string out;
        while(repeat--)
        {
            maybe_convert_to_value(data, jv);
            out = json::serialize(jv);
        }
        return clock_type::now() - start;
    }
};

//----------------------------------------------------------

class boost_operator_impl : public base_boost_impl
{
public:
    boost_operator_impl(
        bool is_pool,
        bool with_file_io,
        bool with_conversion,
        parse_options const& popts)
        : base_boost_impl(
            "operators", is_pool, with_file_io, with_conversion, popts)
    {}

    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const override
    {
        std::istringstream is(fi.text);
        is.exceptions(std::ios::failbit);

        auto const start = clock_type::now();
        while(repeat--)
        {
            monotonic_resource mr;
            storage_ptr sp;
            if( is_pool_ )
                sp = &mr;

            value jv( std::move(sp) );
            is.seekg(0);
            is >> get_parse_options() >> jv;
            maybe_convert_from_value(jv);
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const override
    {
        auto const start = clock_type::now();
        while(repeat--)
        {
            monotonic_resource mr;
            storage_ptr sp;
            if( is_pool_ )
                sp = &mr;

            std::ifstream is( fi.name, std::ios::in | std::ios::binary );
            is.exceptions(std::ios::failbit);

            value jv( std::move(sp) );
            is >> get_parse_options() >> jv;

            maybe_convert_from_value(jv);
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize_string(file_item const& fi, std::size_t repeat) const override
    {
        monotonic_resource mr;
        storage_ptr sp;
        if( is_pool_ )
            sp = &mr;

        auto jv = json::parse( fi.text, std::move(sp) );
        auto data = construct_data(file_index(), jv);

        auto const start = clock_type::now();
        std::string out;
        while(repeat--)
        {
            maybe_convert_to_value(data, jv);
            std::ostringstream os;
            os.exceptions(std::ios::failbit);
            os << jv;
            out = os.str();
        }
        return clock_type::now() - start;
    }
};

//----------------------------------------------------------

class boost_direct_impl : public base_boost_impl
{
    struct string_parser
    {
        file_item const& fi;
        parse_options const& opts;
        std::size_t& repeat;

        template< class I >
        clock_type::duration operator()(I) const
        {
            auto const start = clock_type::now();
            while(repeat--)
            {
                using data_type = typename supported_file_at<I>::type;
                data_type v;
                system::error_code ec;
                parser_for<data_type> p(opts, &v);

                auto const n = p.write_some(
                    false, fi.text.data(), fi.text.size(), ec );
                if( !ec.failed() && n < fi.text.size() )
                    ec = error::extra_data;
                if( ec.failed() )
                    throw system::system_error( ec );
            }
            return clock_type::now() - start;
        }
    };

    struct file_parser
    {
        file_item const& fi;
        parse_options const& opts;
        std::size_t& repeat;

        template< class I >
        clock_type::duration operator()(I) const
        {
            auto const start = clock_type::now();
            char s[ BOOST_JSON_STACK_BUFFER_SIZE];
            while(repeat--)
            {
                using data_type = typename supported_file_at<I>::type;
                data_type v;
                system::error_code ec;
                parser_for<data_type> p(opts, &v);

                FILE* f = fopen(fi.name.data(), "rb");

                while( true )
                {
                    std::size_t const sz = fread(s, 1, sizeof(s), f);
                    if( ferror(f) )
                        break;

                    p.write_some(true, s, sz, ec);
                    if( ec.failed() )
                        throw system::system_error(ec);

                    if( feof(f) )
                        break;
                }

                if( !p.done() )
                {
                    p.write_some(false, nullptr, 0, ec);
                    if( ec.failed() )
                        throw system::system_error(ec);
                }

                fclose(f);
            }
            return clock_type::now() - start;
        }
    };

    struct string_serializer
    {
        file_item const& fi;
        parse_options const& opts;
        std::size_t& repeat;

        template< class I >
        clock_type::duration operator()(I) const
        {
            typename supported_file_at<I>::type v;
            json::parse_into(v, fi.text);

            auto const start = clock_type::now();
            serializer sr;
            string out;
            out.reserve(512);
            while(repeat--)
            {
                sr.reset(&v);
                out.clear();
                for(;;)
                {
                    auto const sv = sr.read(
                        out.end(), out.capacity() - out.size() );
                    out.grow( sv.size() );
                    if( sr.done() )
                        break;
                    out.reserve( out.capacity() + 1 );
                }
            }
            return clock_type::now() - start;
        }
    };

public:
    boost_direct_impl(
        bool with_file_io, bool with_conversion, parse_options const& popts)
        : base_boost_impl(
            "direct", false, with_file_io, with_conversion, popts)
    {}

    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() < 0 )
            return skip();
        return mp11::mp_with_index<supported_file_count>(
            file_index(), string_parser{fi, get_parse_options(), repeat} );
    }

    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() < 0 )
            return skip();
        return mp11::mp_with_index<supported_file_count>(
            file_index(), file_parser{fi, get_parse_options(), repeat} );
    }

    clock_type::duration
    serialize_string(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() < 0 )
            return skip();
        return mp11::mp_with_index<supported_file_count>(
            file_index(), string_serializer{fi, get_parse_options(), repeat} );
    }
};

//----------------------------------------------------------

#ifdef BOOST_JSON_HAS_RAPIDJSON
template<class Allocator, bool FullPrecision>
struct rapidjson_impl : public any_impl
{
    static constexpr unsigned parse_flags
        = rapidjson::kParseDefaultFlags
        | (FullPrecision
                ? rapidjson::kParseFullPrecisionFlag
                : rapidjson::kParseNoFlags);

    static
    parse_options
    make_parse_options() noexcept
    {
        parse_options opts;
        opts.numbers = FullPrecision
            ? number_precision::precise : number_precision::imprecise;
        return opts;
    }

    rapidjson_impl(bool with_file_io, bool with_conversion)
        : any_impl(
            "rapidjson",
            "",
            false,
            std::is_same<Allocator, RAPIDJSON_DEFAULT_ALLOCATOR>::value,
            with_file_io,
            with_conversion,
            make_parse_options() )
    {}

    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        using namespace rapidjson;
        auto const start = clock_type::now();
        while(repeat--)
        {
            Allocator alloc;
            GenericDocument<UTF8<>, Allocator> d(&alloc);
            d.template Parse<rapidjson_impl::parse_flags>(
                fi.text.data(), fi.text.size() );
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        using namespace rapidjson;

        auto const start = clock_type::now();
        char* s = new char[ fi.text.size() ];
        std::unique_ptr<char[]> holder(s);

        while(repeat--)
        {
            FILE* f = fopen(fi.name.data(), "rb");
            std::size_t const sz = fread(s, 1, fi.text.size(), f);

            Allocator alloc;
            GenericDocument<UTF8<>, Allocator> d(&alloc);
            d.template Parse<rapidjson_impl::parse_flags>(s, sz);

            fclose(f);
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize_string(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        using namespace rapidjson;
        Allocator alloc;
        GenericDocument<UTF8<>, Allocator> d(&alloc);
        d.template Parse<rapidjson_impl::parse_flags>( fi.text.data(), fi.text.size() );

        auto const start = clock_type::now();
        rapidjson::StringBuffer st;
        while(repeat--)
        {
            st.Clear();
            rapidjson::Writer<rapidjson::StringBuffer> wr(st);
            d.Accept(wr);
        }
        return clock_type::now() - start;
    }
};
#endif // BOOST_JSON_HAS_RAPIDJSON

//----------------------------------------------------------

#ifdef BOOST_JSON_HAS_NLOHMANN_JSON
struct nlohmann_impl : public any_impl
{
    nlohmann_impl(bool with_file_io, bool with_conversion)
        : any_impl(
            "nlohmann",
            "",
            false,
            false,
            with_file_io,
            with_conversion,
            parse_options() )
    {}

    clock_type::duration
    parse_string(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        auto const start = clock_type::now();
        while(repeat--)
        {
            auto jv = nlohmann::json::parse( fi.text.begin(), fi.text.end() );
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    parse_file(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        auto const start = clock_type::now();
        char* s = new char[ fi.text.size() ];
        std::unique_ptr<char[]> holder(s);

        while(repeat--)
        {
            FILE* f = fopen(fi.name.data(), "rb");
            std::size_t const sz = fread(s, 1, fi.text.size(), f);

            auto jv = nlohmann::json::parse(s, s + sz);

            fclose(f);
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize_string(file_item const& fi, std::size_t repeat) const override
    {
        if( file_index() >= 0 )
            return skip();

        auto jv = nlohmann::json::parse( fi.text.begin(), fi.text.end() );

        auto const start = clock_type::now();
        while(repeat--)
            auto st = jv.dump();
        return clock_type::now() - start;
    }
};
#endif // BOOST_JSON_HAS_NLOHMANN_JSON

} // json
} // boost

//

using namespace boost::json;

static bool parse_option( char const* s )
{
    if( *s == 0 )
        return false;

    char opt = *s++;

    if( *s++ != ':' )
        return false;

    switch( opt )
    {
    case 't':
        s_tests = s;
        break;

    case 'i':
        s_impls = s;
        break;

    case 'n':
        {
            int k = std::atoi( s );

            if( k > 0 )
                s_trials = k;
            else
                return false;
        }
        break;

    case 'b':
        s_branch = s;
        break;

    case 'a':
        s_alloc = s;
        break;

    case 'm':
        s_num_mode = s;
        break;

    case 'f':
        s_file_io = s;
        break;

    case 'c':
        s_conversion = s;
        break;
    }

    return true;
}

bool add_impl(
    impl_list & vi, char kind, char alloc, char io, char num, char conv)
{
    parse_options popts;
    switch(num)
    {
    case 'i':
        popts.numbers = number_precision::imprecise;
        break;
    case 'p':
        popts.numbers = number_precision::precise;
        break;
    case 'n':
        popts.numbers = number_precision::none;
        break;
    default:
        return false;
    }
    bool const with_file_io = io == 'y';
    bool const is_pool = alloc == 'p';
    bool const with_conversion = conv == 'y';

    impl_ptr impl;
    switch( kind )
    {
    case 'b':
        impl.reset(
            new boost_impl(is_pool, with_file_io, with_conversion, popts) );
        break;

    case 'u':
        impl.reset(
            new boost_null_impl(with_file_io, with_conversion, popts) );
        break;

    case 's':
        impl.reset(
            new boost_simple_impl(
                is_pool, with_file_io, with_conversion, popts) );
        break;

    case 'o':
        impl.reset(
            new boost_operator_impl(
                is_pool, with_file_io, with_conversion, popts) );
        break;

    case 'd':
        impl.reset(
            new boost_direct_impl(with_file_io, with_conversion, popts) );
        break;

#ifdef BOOST_JSON_HAS_RAPIDJSON
    case 'r':
        if(is_pool)
        {
            using Allocator = RAPIDJSON_DEFAULT_ALLOCATOR;
            if(popts.numbers == number_precision::precise)
                impl.reset(
                    new rapidjson_impl<Allocator, true>(
                        with_file_io, with_conversion) );
            else
                impl.reset(
                    new rapidjson_impl<Allocator, false>(
                        with_file_io, with_conversion) );
        }
        else
        {
            using Allocator = rapidjson::CrtAllocator;
            if(popts.numbers == number_precision::precise)
                impl.reset(
                    new rapidjson_impl<Allocator, true>(
                        with_file_io, with_conversion) );
            else
                impl.reset(
                    new rapidjson_impl<Allocator, false>(
                        with_file_io, with_conversion) );
        }
        break;
#endif // BOOST_JSON_HAS_RAPIDJSON

#ifdef BOOST_JSON_HAS_NLOHMANN_JSON
    case 'n':
        impl.reset( new nlohmann_impl(with_file_io, with_conversion) );
        break;
#endif // BOOST_JSON_HAS_NLOHMANN_JSON

    default:
        std::cerr << "Unknown implementation: '" << kind << "'\n";
        return false;
    }

    vi.emplace_back( std::move(impl) );
    return true;
}

static bool do_test( file_list const & vf, impl_list const & vi, char test )
{
    switch( test )
    {
    case 'p':
        bench("Parse", vf, vi, s_trials);
        break;

    case 's':
        bench("Serialize", vf, vi, s_trials);
        break;

    default:
        std::cerr << "Unknown test type: '" << test << "'\n";
        return false;
    }

    return true;
}

int
main(
    int const argc,
    char const* const* const argv)
{
    if( argc < 2 )
    {
        std::cerr <<
            "Usage: bench [options...] <file>...\n"
            "\n"
            "Options: -t:[p][s]                Test parsing, serialization or both\n"
            "            (default both)\n"
            "         -i:[b][u][s][o][d][r][n] Test the specified implementations\n"
            "            (b: Boost.JSON)\n"
            "            (u: Boost.JSON, null parser)\n"
            "            (s: Boost.JSON, convenient functions)\n"
            "            (o: Boost.JSON, stream operators)\n"
            "            (d: Boost.JSON, direct conversion)\n"
#ifdef BOOST_JSON_HAS_RAPIDJSON
            "            (r: RapidJSON)\n"
#endif // BOOST_JSON_HAS_RAPIDJSON
#ifdef BOOST_JSON_HAS_NLOHMANN_JSON
            "            (n: nlohmann/json)\n"
#endif // BOOST_JSON_HAS_NLOHMANN_JSON
            "            (default all)\n"
            "         -a:[p][d]                Memory allocation strategy\n"
            "            (p: memory pool)\n"
            "            (d: default strategy)\n"
            "            (default memory pool)\n"
            "         -n:<number>              Number of trials (default 6)\n"
            "         -b:<branch>              Branch label for boost implementations\n"
            "         -m:[i][p][n]             Number parsing mode\n"
            "            (i: imprecise)\n"
            "            (p: precise)\n"
            "            (n: none)\n"
            "            (default imprecise)\n"
            "         -f:[y][n]                Include file IO into consideration when testing parsers\n"
            "            (y: yes)\n"
            "            (n: no)\n"
            "            (default no)\n"
            "         -c:[y][n]                Convert to user-defined type\n"
            "            (y: yes)\n"
            "            (n: no)\n"
            "            (default no)\n"
        ;

        return 4;
    }

    file_list vf;

    for( int i = 1; i < argc; ++i )
    {
        char const* s = argv[ i ];

        if( *s == '-' )
        {
            if( !parse_option( s+1 ) )
                std::cerr << "Unrecognized or incorrect option: '" << s << "'\n";
        }
        else
        {
            vf.emplace_back( file_item{ argv[i], load_file( s ) } );
        }
    }

    try
    {
        impl_list vi;

        for( char impl: s_impls )
            for( char alloc: s_alloc )
                for( char num: s_num_mode )
                    for( char io: s_file_io )
                        for( char conv: s_conversion )
                            add_impl( vi, impl, alloc, io, num, conv );

        // remove duplicate implementations
        std::sort(
            vi.begin(),
            vi.end(),
            [](impl_ptr const& l, impl_ptr const& r)
            {
                return l->name() < r->name();
            });
        auto const it = std::unique(
            vi.begin(),
            vi.end(),
            [](impl_ptr const& l, impl_ptr const& r)
            {
                return l->name() == r->name();
            });
        vi.erase( it, vi.end() );

        for( char ch: s_tests )
            do_test( vf, vi, ch );

        dout << "\n" << strout.str();
    }
    catch(boost::system::system_error const& se)
    {
        dout << se.what() << std::endl;
    }

    return 0;
}
