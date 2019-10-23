//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#include "lib/nlohmann/single_include/nlohmann/json.hpp"

#include "lib/rapidjson/include/rapidjson/document.h"
#include "lib/rapidjson/include/rapidjson/writer.h"
#include "lib/rapidjson/include/rapidjson/stringbuffer.h"

#include <boost/json.hpp>
#include <boost/beast/_experimental/unit_test/dstream.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <chrono>
#include <iostream>
#include <random>

/*

References

https://github.com/nst/JSONTestSuite

http://seriot.ch/parsing_json.php

*/

namespace beast = boost::beast;
namespace json = boost::json;

boost::beast::unit_test::dstream dout{std::cerr};

//----------------------------------------------------------

class pool_storage : public json::storage
{
    boost::container::pmr::unsynchronized_pool_resource pr_;

public:
    virtual
    ~pool_storage() = default;

    pool_storage()
        : pr_(boost::container::pmr::new_delete_resource())
    {
    }

    void*
    do_allocate(
        std::size_t n,
        std::size_t align) override
    {
        return pr_.allocate(n, align);
    }

    void
    do_deallocate(
        void* p,
        std::size_t n,
        std::size_t align) noexcept override
    {
        pr_.deallocate(p, n, align);
    }

    bool
    do_is_equal(storage const& other) const noexcept override
    {
       return this == dynamic_cast<
            pool_storage const*>(&other);
    }
};

//----------------------------------------------------------

class any_impl
{
public:
    virtual ~any_impl() = default;

    virtual boost::string_view name() const noexcept = 0;
    virtual void parse(boost::string_view s) = 0;
};

//----------------------------------------------------------

class boost_impl : public any_impl
{
    json::parser* p_;

public:
    boost_impl()
        : p_(new json::parser)
    {
    }

    explicit
    boost_impl(json::storage_ptr sp)
        : p_(new json::parser(std::move(sp)))
    {
    }

    ~boost_impl()
    {
        delete p_;
    }

    boost::string_view
    name() const noexcept override
    {
        return "Boost.JSON";
    }

    void
    parse(
        boost::string_view s) override
    {
        boost::system::error_code ec;
        p_->write(s.data(), s.size(), ec);
    }
};

//----------------------------------------------------------

class nlohmann_impl : public any_impl
{
    nlohmann::json root_;

public:
    nlohmann_impl()
    {
    }

    boost::string_view
    name() const noexcept override
    {
        return "nlohmann";
    }

    void
    parse(
        boost::string_view s) override
    {
        auto jv = nlohmann::json::parse(s.begin(), s.end());
    }
};

//----------------------------------------------------------

class rapidjson_impl : public any_impl
{
public:
    boost::string_view
    name() const noexcept override
    {
        return "rapidjson";
    }

    void
    parse(
        boost::string_view s) override
    {
        rapidjson::Document d;
        d.Parse(s.data(), s.size());
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

    boost::string_view
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

    boost::string_view
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
    boost::string_view doc,
    any_impl& impl)
{
    using clock_type = std::chrono::steady_clock;
    auto const when = clock_type::now();
    dout << impl.name();
    impl.parse(doc);
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

int
main(int, char**)
{
    for(int i = 5; i < 7; ++i)
    {
        factory f;
        f.max_depth(i);
        auto const doc = f.make_document();
        for(int j = 0; j < 3; ++j)
        {
            rapidjson_impl impl;
            benchParse(doc, impl);
        }
        for(int j = 0; j < 3; ++j)
        {
            nlohmann_impl impl;
            benchParse(doc, impl);
        }
        for(int j = 0; j < 3; ++j)
        {
            //boost_impl impl(json::make_storage<pool_storage>());
            boost_impl impl;
            benchParse(doc, impl);
        }
    }

    return 0;
}
