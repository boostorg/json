//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#include <boost/json.hpp>
#include <boost/beast/_experimental/unit_test/dstream.hpp>
#include <boost/beast/core/static_string.hpp>
#include <chrono>
#include <iostream>
#include <random>

namespace beast = boost::beast;
namespace json = boost::json;

boost::beast::unit_test::dstream dout{std::cerr};

//------------------------------------------------------------------------------

class any_writer
{
public:
    virtual ~any_writer() = default;

    virtual
    void
    insert(
        boost::string_view key,
        int value) = 0;
};

//------------------------------------------------------------------------------

class boost_writer : public any_writer
{
    json::value root_;
    std::vector<json::value*> v_;

public:
    boost_writer()
        : root_(json::object{})
    {
    }

    void
    insert(
        boost::string_view key,
        int value) override
    {
        root_.as_object().insert_or_assign(key, value);
    }
};

//------------------------------------------------------------------------------

class factory
{
    std::mt19937 g_;
    beast::static_string<445> lorem_;

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

    beast::static_string<20>
    key() noexcept
    {
        beast::static_string<20> s;
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

    boost::string_view
    string() noexcept
    {
        return {
            lorem_.data(),
            1 + rand(lorem_.size()) };
    }

    int
    integer() noexcept
    {
        return rand(std::numeric_limits<int>::max());
    }
};

//------------------------------------------------------------------------------

// insert a bunch of key/value into an object
void
testInsertObject(
    boost::string_view name,
    any_writer& w)
{
    using clock_type = std::chrono::steady_clock;

    //for(int j = 0; j < 5; ++j)
    {
        factory f;
        auto const when = clock_type::now();
        for(auto i = 0; i < 1000000; ++i)
            w.insert(f.key(), f.integer());

        auto const elapsed =
            std::chrono::duration_cast<
                std::chrono::milliseconds>(
                    clock_type::now() - when);

        dout << name << ": " << elapsed.count() << "ms\n";
    }
}

int
main(int argc, char** argv)
{
    boost::ignore_unused(argc);
    boost::ignore_unused(argv);

    boost_writer w;
    testInsertObject("boost::json", w);

    return 0;
}
