//
// Copyright (c) 2025 Dmitry Arkhipov (grisumbras@yandex.ru)
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

#include <boost/json.hpp>
#include <boost/json/basic_parser_impl.hpp>
#include <boost/mp11/algorithm.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include "test_suite.hpp"
#include "apache_builds.hpp"
#include "canada.hpp"
#include "citm_catalog.hpp"

using namespace boost::json;

namespace {

namespace mpl = boost::mp11;

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

std::string s_tests = "ps";
std::string s_impls = "bsod";
std::size_t s_trials = 6;
std::string s_branch = "";
std::string s_alloc = "p";
std::string s_num_mode = "i";

bool
parse_option(char const* s)
{
    if(*s == 0)
        return false;

    char opt = *s++;

    if(*s++ != ':')
        return false;

    switch(opt)
    {
    case 't':
        s_tests = s;
        break;

    case 'i':
        s_impls = s;
        break;

    case 'n':
        {
            int k = std::atoi(s);

            if(k > 0)
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
    }

    return true;
}

struct file_item
{
    string_view name;
    std::string text;
    int index;
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
    auto const nread = fread(&s[0], 1, size, f);
    s.resize(nread);
    fclose(f);
    return s;
}

#if defined(BOOST_DESCRIBE_CXX14)
using supported_files = mpl::mp_list<
    apache_builds_support,
    canada_support,
    citm_catalog_support >;
#else
struct mock_type_support
{
    using type = int;
    static constexpr char const* const name = "";
};
using supported_files = mpl::mp_list<mock_type_support>;
#endif // defined(BOOST_DESCRIBE_CXX14)

using supported_file_count = mpl::mp_size<supported_files>;

template< class I >
using supported_file_at = mpl::mp_at<supported_files, I>;

struct file_matcher
{
    string_view file_name;
    int& result;

    template< class I >
    void
    operator()(I)
    {
        using supported_file = supported_file_at<I>;
        std::size_t const name_len = std::strlen(supported_file::name);

        std::size_t pos = file_name.rfind(
            supported_file::name, string_view::npos, name_len);
        if(pos == string_view::npos)
            return;
        if( pos + name_len != file_name.size() )
            return;
        if( pos != 0 && file_name[pos - 1] != '/'
            && file_name[pos - 1] != '\\' )
            return;

        result = static_cast<int>(I::value);
    }
};

int
find_supported_file(string_view file_name)
{
    int result = -1;
    mpl::mp_for_each< mpl::mp_iota<supported_file_count> >(
        file_matcher{file_name, result} );
    return result;
}

using clock_type = std::chrono::steady_clock;

class any_impl
{
    std::string name_;
    parse_options popts_;

    virtual
    clock_type::duration
    parse(file_item const& fi, std::size_t repeat) const = 0;

    virtual
    clock_type::duration
    serialize(file_item const& fi, std::size_t repeat) const = 0;

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

public:
    using operation = auto (any_impl::*)
        (file_item const& fi, std::size_t repeat) const
        -> clock_type::duration;

    any_impl(
        string_view base_name,
        string_view flavor,
        bool is_boost,
        bool is_pool,
        parse_options const& popts)
        : popts_(popts)
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
        if( fi.index < 0 )
            return skip();

        if(verb == "Parse")
            return parse(fi, repeat);

        BOOST_ASSERT(verb == "Serialize");
        return serialize(fi, repeat);
    }

    string_view
    name() const noexcept
    {
        return name_;
    }
};

using impl_ptr = std::unique_ptr<any_impl const>;
using impl_list = std::vector<impl_ptr>;

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
    return {
        n,
        static_cast<std::size_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                elapsed).count()),
        0};
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

bool
do_test(file_list const & vf, impl_list const & vi, char test)
{
    string_view verb;
    switch(test)
    {
    case 'p':
        verb = "Parse";
        break;
    case 's':
        verb = "Serialize";
        break;
    default:
        std::cerr << "Unknown test type: '" << test << "'\n";
        return false;
    }

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
            for(unsigned k = 0; k < s_trials; ++k)
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
            if(s_trials >= 6)
            {
                // discard worst 2
                trial.erase(trial.begin(), trial.begin() + 2);
                // discard best 1
                trial.resize(trial.size() - 1);
            }
            else if(s_trials > 3)
            {
                trial.erase(trial.begin(), trial.begin() + s_trials - 3);
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
    return true;
}

class base_boost_impl : public any_impl
{
protected:
    bool is_pool_;

    struct delete_data
    {
        std::size_t index;

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

        void operator()(void* data)
        {
            mpl::mp_with_index<supported_file_count>( index, deleter{data} );
        }
    };

    using data_holder = std::unique_ptr<void, delete_data>;

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

    struct convert_to_value
    {
        data_holder& data;
        value& jv;

        template< class I >
        BOOST_NOINLINE
        void operator()(I) const
        {
            using data_type = typename supported_file_at<I>::type;
            value_from( *reinterpret_cast<data_type*>( data.get() ), jv );
        }
    };

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

    static
    data_holder
    construct_data(std::size_t file_index, value const& jv)
    {
        data_holder result;
        mpl::mp_with_index<supported_file_count>(
            file_index, data_constructor{result, jv} );
        return result;
    }

public:
    base_boost_impl(
        string_view flavor, bool is_pool, parse_options const& popts)
        : any_impl("boost", flavor, true, is_pool, popts)
        , is_pool_(is_pool)
    {}
};

class boost_impl : public base_boost_impl
{
public:
    boost_impl(bool is_pool, parse_options const& popts)
        : base_boost_impl("", is_pool, popts)
    {}

    clock_type::duration
    parse(file_item const& fi, std::size_t repeat) const override
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
            mpl::mp_with_index<supported_file_count>(
                fi.index, convert_from_value{p.release()});
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize(file_item const& fi, std::size_t repeat) const override
    {
        monotonic_resource mr;
        storage_ptr sp;
        if(is_pool_)
            sp = &mr;
        value jv = boost::json::parse( fi.text, std::move(sp) );
        data_holder data = construct_data(fi.index, jv);

        auto const start = clock_type::now();
        serializer sr;
        string out;
        out.reserve(512);
        while(repeat--)
        {
            mpl::mp_with_index<supported_file_count>(
                fi.index, convert_to_value{data, jv});
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

class boost_simple_impl : public base_boost_impl
{
public:
    boost_simple_impl(bool is_pool, parse_options const& popts)
        : base_boost_impl("convenient", is_pool, popts)
    {}

    clock_type::duration
    parse(file_item const& fi, std::size_t repeat) const override
    {
        auto const start = clock_type::now();
        while(repeat--)
        {
            monotonic_resource mr;
            storage_ptr sp;
            if( is_pool_ )
                sp = &mr;

            auto jv = boost::json::parse(
                fi.text, std::move(sp), get_parse_options() );
            mpl::mp_with_index<supported_file_count>(
                fi.index, convert_from_value{jv});
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize(file_item const& fi, std::size_t repeat) const override
    {
        monotonic_resource mr;
        storage_ptr sp;
        if(is_pool_)
            sp = &mr;
        auto jv = boost::json::parse( fi.text, std::move(sp) );
        auto data = construct_data(fi.index, jv);

        auto const start = clock_type::now();
        std::string out;
        while(repeat--)
        {
            mpl::mp_with_index<supported_file_count>(
                fi.index, convert_to_value{data, jv});
            out = boost::json::serialize(jv);
        }
        return clock_type::now() - start;
    }
};

class boost_operator_impl : public base_boost_impl
{
public:
    boost_operator_impl(bool is_pool, parse_options const& popts)
        : base_boost_impl("operators", is_pool, popts)
    {}

    clock_type::duration
    parse(file_item const& fi, std::size_t repeat) const override
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
            mpl::mp_with_index<supported_file_count>(
                fi.index, convert_from_value{jv});
        }
        return clock_type::now() - start;
    }

    clock_type::duration
    serialize(file_item const& fi, std::size_t repeat) const override
    {
        monotonic_resource mr;
        storage_ptr sp;
        if(is_pool_)
            sp = &mr;

        auto jv = boost::json::parse( fi.text, std::move(sp) );
        auto data = construct_data(fi.index, jv);

        auto const start = clock_type::now();
        std::string out;
        while(repeat--)
        {
            mpl::mp_with_index<supported_file_count>(
                fi.index, convert_to_value{data, jv});
            std::ostringstream os;
            os.exceptions(std::ios::failbit);
            os << jv;
            out = os.str();
        }
        return clock_type::now() - start;
    }
};

class boost_direct_impl : public base_boost_impl
{
    struct parser
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
                boost::system::error_code ec;
                parser_for<data_type> p(opts, &v);

                auto const n = p.write_some(
                    false, fi.text.data(), fi.text.size(), ec );
                if( !ec.failed() && n < fi.text.size() )
                    ec = error::extra_data;
                if( ec.failed() )
                    throw boost::system::system_error(ec);
            }
            return clock_type::now() - start;
        }
    };

    struct serializer
    {
        file_item const& fi;
        parse_options const& opts;
        std::size_t& repeat;

        template< class I >
        clock_type::duration operator()(I) const
        {
            typename supported_file_at<I>::type v;
            boost::json::parse_into(v, fi.text);

            auto const start = clock_type::now();
            boost::json::serializer sr;
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
    boost_direct_impl(parse_options const& popts)
        : base_boost_impl("direct", false, popts)
    {}

    clock_type::duration
    parse(file_item const& fi, std::size_t repeat) const override
    {
        return mpl::mp_with_index<supported_file_count>(
            fi.index, parser{fi, get_parse_options(), repeat} );
    }

    clock_type::duration
    serialize(file_item const& fi, std::size_t repeat) const override
    {
        return mpl::mp_with_index<supported_file_count>(
            fi.index, serializer{fi, get_parse_options(), repeat} );
    }
};

bool
add_impl(impl_list & vi, char kind, char alloc, char num)
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
    bool const is_pool = alloc == 'p';

    impl_ptr impl;
    switch(kind)
    {
    case 'b':
        impl.reset( new boost_impl(is_pool, popts) );
        break;

    case 's':
        impl.reset( new boost_simple_impl(is_pool, popts) );
        break;

    case 'o':
        impl.reset( new boost_operator_impl(is_pool, popts) );
        break;

    case 'd':
        impl.reset( new boost_direct_impl(popts) );
        break;

    default:
        std::cerr << "Unknown implementation: '" << kind << "'\n";
        return false;
    }

    vi.emplace_back( std::move(impl) );
    return true;
}

} // namespace

int
main(int const argc, char const* const* const argv)
{
    if(argc < 2)
    {
        std::cerr <<
            "Usage: bench-direct [options...] <file>...\n"
            "\n"
            "Options: -t:[p][s]                Test parsing, serialization or both\n"
            "            (default both)\n"
            "         -i:[b][u][s][o][d][r][n] Test the specified implementations\n"
            "            (b: Boost.JSON)\n"
            "            (s: Boost.JSON, convenient functions)\n"
            "            (o: Boost.JSON, stream operators)\n"
            "            (d: Boost.JSON, direct conversion)\n"
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
        ;

        return 4;
    }

    file_list vf;

    for(int i = 1; i < argc; ++i)
    {
        char const* s = argv[i];

        if(*s == '-')
        {
            if( !parse_option(s + 1) )
                std::cerr
                    << "Unrecognized or incorrect option: '" << s << "'\n";
        }
        else
        {
            string_view file_name = argv[i];
            vf.emplace_back(
                file_item{
                    file_name, load_file(s), find_supported_file(file_name)});
        }
    }

    try
    {
        impl_list vi;
        for( char impl: s_impls )
            for( char alloc: s_alloc )
                for( char num: s_num_mode )
                    add_impl(vi, impl, alloc, num);

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

        for(char ch: s_tests)
            do_test(vf, vi, ch);

        dout << "\n" << strout.str();
    }
    catch(boost::system::system_error const& se)
    {
        dout << se.what() << std::endl;
    }

    return 0;
}
