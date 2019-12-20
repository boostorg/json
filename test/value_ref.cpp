//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/value_ref.hpp>

#include <boost/json/value.hpp>
#include <boost/json/serializer.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

class value_ref_test
{
public:
    using init_list =
        std::initializer_list<value_ref>;

    void
    testCtors()
    {
        // scalars
        (void)value_ref((short)1);
        (void)value_ref((int)1);
        (void)value_ref((long)1);
        (void)value_ref((long long)1);
        (void)value_ref((unsigned short)1);
        (void)value_ref((unsigned int)1);
        (void)value_ref((unsigned long)1);
        (void)value_ref((unsigned long long)1);
        (void)value_ref((float)1.);
        (void)value_ref((double)1.);
        (void)value_ref((long double)1.);
        (void)value_ref(true);
        (void)value_ref(nullptr);

        // string_view, char const*
        (void)(value_ref)string_view("test");
        (void)(value_ref)("test");

        // init-list
        value_ref({1,2,3,4,5});

        // value
        (value_ref)value();
        {
            value jv;
            (value_ref)jv;
        }
        {
            value const jv;
            (value_ref)jv;
        }

        // object
        (value_ref)object();
        {
            object o;
            (value_ref)o;
        }
        {
            object const o{};
            (value_ref)o;
        }

        // array
        (value_ref)object();
        {
            array a;
            (value_ref)a;
        }
        {
            array const a{};
            (value_ref)a;
        }

        // string
        (value_ref)string();
        {
            string s;
            (value_ref)s;
        }
        {
            string const s{};
            (value_ref)s;
        }
    }
    
    void
    testInitList()
    {
        // string_view, char const*
        (void)init_list{string_view("test")};
        (void)init_list{"test"};

        // scalars
        (void)init_list{(short)1};
        (void)init_list{(int)1};
        (void)init_list{(long)1};
        (void)init_list{(long long)1};
        (void)init_list{(unsigned short)1};
        (void)init_list{(unsigned int)1};
        (void)init_list{(unsigned long)1};
        (void)init_list{(unsigned long long)1};
        (void)init_list{(float)1.};
        (void)init_list{(double)1.};
        (void)init_list{(long double)1.};
        (void)init_list{true};
        (void)init_list{nullptr};

        // bool
        {
            bool b = true;
            auto const cb = b;
            BOOST_TEST(value_ref::make_value(init_list{b}, {}).at(0).is_bool());
            BOOST_TEST(value_ref::make_value(init_list{cb}, {}).at(0).is_bool());
        }

        // value
        (void)init_list{value()};
        {
            value jv;
            (void)init_list{jv};
        }
        {
            value const jv{};
            (void)init_list{jv};
        }

        // object
        (void)init_list{object()};
        {
            object o;
            (void)init_list{o};
        }
        {
            object const o{};
            (void)init_list{o};
        }

        // array
        (void)init_list{object()};
        {
            array a;
            (void)init_list{a};
        }
        {
            array const a{};
            (void)init_list{a};
        }

        // string
        (void)init_list{string()};
        {
            string s;
            (void)init_list{s};
        }
        {
            string const s{};
            (void)init_list{s};
        }
        
        // init_list
        (void)init_list{{1,2,3,4,5}};
        (void)init_list{{{1,2},{3,4,5}}};
        (void)init_list{{1,2,{3,{4,5}}}};

        {
            init_list init = 
                { { "key", true } };
        }
    }

    void
    make_value(
        init_list init,
        string_view s)
    {
        auto const jv =
            value_ref(init).make_value({});
        auto const js = to_string(jv);
        BOOST_TEST(js == s);
    }

    void
    testMakeValue()
    {
        // scalars
        make_value({(short)-1}, "[-1]");
        make_value({(int)-2}, "[-2]");
        make_value({(long)-3}, "[-3]");
        make_value({(long long)-4}, "[-4]");
        make_value({(unsigned short)1}, "[1]");
        make_value({(unsigned int)2}, "[2]");
        make_value({(unsigned long)3}, "[3]");
        make_value({(unsigned long long)4}, "[4]");
        //make_value({(float)1.}, "[1]");
        //make_value({(double)1.}, "[1]");
        //make_value({(long double)1.}, "[1]");
        make_value({true}, "[true]");
        make_value({nullptr}, "[null]");

        // string_view
        make_value({string_view("5")}, "[\"5\"]");
        make_value({"6"}, "[\"6\"]");

        // value
        {
            make_value({value(1)}, "[1]");
            value const v(1);
            make_value({v}, "[1]");
            make_value({value(2)}, "[2]");
        }

        // object
        {
            make_value({object({{"k1",1},{"k2",2}})}, "[{\"k1\":1,\"k2\":2}]");
            object const obj({{"k1",1},{"k2",2}});
            make_value({obj}, "[{\"k1\":1,\"k2\":2}]");
        }

        // array
        {
            make_value({array({1,2,3})}, "[[1,2,3]]");
            array const arr({1,2,3});
            make_value({arr}, "[[1,2,3]]");
        }

        // string
        {
            make_value({string("test")}, "[\"test\"]");
            string const str("test");
            make_value({str}, "[\"test\"]");
        }

        // init list with size != 2
        {
            make_value({{1,2,3}}, "[[1,2,3]]");
        }

        // `string` as key
        {
            make_value({
                {string("k1"), 1},
                {string("k2"), 2}},
                "{\"k1\":1,\"k2\":2}");
        }

        // object/array conversion

        make_value(
            {1,2,{3,{4,5}}},
            "[1,2,[3,[4,5]]]" );

        make_value(
            {{"k1",1}},
            "{\"k1\":1}");

        make_value(
            {{"k1",1}, {"k2",2}},
            "{\"k1\":1,\"k2\":2}");

        make_value(
            {{"k1",1}, {"k2",{{"k3",3}, {"k4",4}}}},
            "{\"k1\":1,\"k2\":{\"k3\":3,\"k4\":4}}");

        make_value({value(1)}, "[1]");
        make_value({array({1,2,3,4})}, "[[1,2,3,4]]");
    }

    using kv_init_list =
        std::initializer_list<
            std::pair<string_view, value_ref>>;

    void
    make_object(
        kv_init_list init,
        string_view s)
    {
        auto const jv = value(object(init));
        auto const js = to_string(jv);
        BOOST_TEST(js == s);
    }

    void
    testObjects()
    {
        make_object({
            {"k1",1}
            }, "{\"k1\":1}");
        make_object({
            {"k1",1}, {"k2",2}
            }, "{\"k1\":1,\"k2\":2}");
        make_object({
            {"k1", {1,2}}, {"k2", {1,2,3}}
            }, "{\"k1\":[1,2],\"k2\":[1,2,3]}");
        make_object({
            {"k1", {{"k2",2}, {"k3",3}}}
            }, "{\"k1\":{\"k2\":2,\"k3\":3}}");
    }

    struct FT
    {
        value
        to_value(storage_ptr sp) const
        {
            return nullptr;
        }
    };

    void
    run()
    {
        testCtors();
        testInitList();
        testMakeValue();
        testObjects();
    }
};

TEST_SUITE(value_ref_test, "boost.json.value_ref");

} // json
} // boost
