//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/object.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/static_assert.hpp>
#include <type_traits>

#include "test_storage.hpp"

namespace boost {
namespace json {

class object_test : public beast::unit_test::suite
{
public:
    void
    check(
        object const& obj,
        std::size_t bucket_count)
    {
        BEAST_EXPECT(! obj.empty());
        BEAST_EXPECT(obj.size() == 3);
        BEAST_EXPECT(
            obj.bucket_count() == bucket_count);
        BEAST_EXPECT(obj["a"].as_number() == 1);
        BEAST_EXPECT(obj["b"].as_bool());
        BEAST_EXPECT(obj["c"].as_string() == "hello");

        // ordering, storage

        auto it = obj.begin();
        BEAST_EXPECT(it->first == "a");
        BEAST_EXPECT(
            *it->second.get_storage() ==
            *obj.get_storage());

        ++it;
        BEAST_EXPECT(it->first == "b");
        BEAST_EXPECT(
            *it->second.get_storage() ==
            *obj.get_storage());

        it++;
        BEAST_EXPECT(it->first == "c");
        BEAST_EXPECT(
            *it->second.get_storage() ==
            *obj.get_storage());
    }

    void
    testSpecial()
    {
        auto sp = make_storage<unique_storage>();
        storage_ptr sp0 =
            default_storage();
        BEAST_EXPECT(*sp != *sp0);

        // object()
        {
            object obj;
            BEAST_EXPECT(obj.empty());
            BEAST_EXPECT(obj.size() == 0);
            BEAST_EXPECT(obj.bucket_count() == 0);
            BEAST_EXPECT(*obj.get_storage() == *sp0);
        }

        // object(size_type)
        {
            object obj(50);
            BEAST_EXPECT(obj.empty());
            BEAST_EXPECT(obj.size() == 0);
            BEAST_EXPECT(obj.bucket_count() == 53);
            BEAST_EXPECT(*obj.get_storage() == *sp0);
        }

        // object(storage_ptr)
        {
            object obj(sp);
            BEAST_EXPECT(*obj.get_storage() == *sp);
        }

        // object(size_type, storage_ptr)
        {
            object obj(50, sp);
            BEAST_EXPECT(obj.empty());
            BEAST_EXPECT(obj.size() == 0);
            BEAST_EXPECT(obj.bucket_count() == 53);
            BEAST_EXPECT(*obj.get_storage() == *sp);
        }
       
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}};
            {
                // object(InputIt, InputIt)
                object obj(init.begin(), init.end());
                check(obj, 3);
            }
            {
                // object(InputIt, InputIt, size_type)
                object obj(init.begin(), init.end(), 5);
                check(obj, 7);
            }
            {
                // object(InputIt, InputIt, storage_ptr)
                object obj(init.begin(), init.end(), sp);
                BEAST_EXPECT(*obj.get_storage() == *sp);
                check(obj, 3);
            }
            {
                // object(InputIt, InputIt, size_type, storage_ptr)
                object obj(init.begin(), init.end(), 5, sp);
                BEAST_EXPECT(! obj.empty());
                BEAST_EXPECT(obj.size() == 3);
                BEAST_EXPECT(obj.bucket_count() == 7);
                BEAST_EXPECT(*obj.get_storage() == *sp);
                check(obj, 7);
            }
        }

        // object(object&&)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object obj2(std::move(obj1));
            BEAST_EXPECT(
                *obj1.get_storage() ==
                *obj2.get_storage());
            BEAST_EXPECT(obj1.empty());
            BEAST_EXPECT(obj1.size() == 0);
            check(obj2, 3);
        }

        // object(object&&, storage_ptr)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object obj2(std::move(obj1), sp);
            BEAST_EXPECT(! obj1.empty());
            BEAST_EXPECT(
                *obj1.get_storage() !=
                *obj2.get_storage());
            check(obj2, 3);
        }

        // object(object const&)
        {

            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object obj2(obj1);
            BEAST_EXPECT(! obj1.empty());
            BEAST_EXPECT(
                *obj1.get_storage() ==
                *obj2.get_storage());
            check(obj2, 3);
        }

        // object(object const&, storage_ptr)
        {

            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object obj2(obj1, sp);
            BEAST_EXPECT(! obj1.empty());
            BEAST_EXPECT(
                *obj1.get_storage() !=
                *obj2.get_storage());
            check(obj2, 3);
        }

        // object(init_list)
        {
            object obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            check(obj, 3);
        }

        // object(init_list, size_type)
        {
            object obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                },
                5);
            check(obj, 7);
        }

        // object(init_list, storage_ptr)
        {
            object obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                },
                sp);
            BEAST_EXPECT(
                *obj.get_storage() == *sp);
            check(obj, 3);
        }

        // object(init_list, size_type, storage_ptr)
        {
            object obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                },
                5, sp);
            BEAST_EXPECT(
                *obj.get_storage() == *sp);
            check(obj, 7);
        }

        // operator=(object&&)
        {
            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2;
                obj2 = std::move(obj1);
                check(obj2, 3);
                BEAST_EXPECT(obj1.empty());
                BEAST_EXPECT(
                    *obj1.get_storage() == *sp0);
                BEAST_EXPECT(
                    *obj1.get_storage() ==
                    *obj2.get_storage());
            }
            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2(sp);
                obj2 = std::move(obj1);
                check(obj2, 3);
                check(obj1, 3);
                BEAST_EXPECT(
                    *obj1.get_storage() !=
                    *obj2.get_storage());
            }
        }

        // operator=(object const&)
        {
            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2;
                obj2 = obj1;
                check(obj1, 3);
                check(obj2, 3);
                BEAST_EXPECT(
                    *obj1.get_storage() ==
                    *obj2.get_storage());
            }
            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2(sp);
                obj2 = obj1;
                check(obj1, 3);
                check(obj2, 3);
                BEAST_EXPECT(
                    *obj1.get_storage() !=
                    *obj2.get_storage());
            }
        }

        // operator=(init_list)
        {
            {
                object obj;
                obj = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"} },
                BEAST_EXPECT(
                    *obj.get_storage() == *sp0);
                check(obj, 3);
            }
            {
                object obj(sp);
                obj = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"} },
                BEAST_EXPECT(
                    *obj.get_storage() == *sp);
                check(obj, 3);
            }
        }
    }

    void
    testIterators()
    {
        object obj({
            {"a", 1},
            {"b", true},
            {"c", "hello"}});
        {
            auto it = obj.begin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == obj.end());
        }
        {
            auto it = obj.cbegin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == obj.cend());
        }
        {
            auto it = static_cast<
                object const&>(obj).begin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == static_cast<
                object const&>(obj).end());
        }
        {
            auto it = obj.end();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == obj.begin());
        }
        {
            auto it = obj.cend();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == obj.cbegin());
        }
        {
            auto it = static_cast<
                object const&>(obj).end();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == static_cast<
                object const&>(obj).begin());
        }
    }

    void
    testModifiers()
    {
        // clear
        {
            object obj;
            obj.emplace("x", 1);
            BEAST_EXPECT(! obj.empty());
            obj.clear();
            BEAST_EXPECT(obj.empty());
        }

        // insert(value_type&&)
        {
            object obj;
            auto v = object::value_type("a", 1);
            auto result = obj.insert(std::move(v));
            BEAST_EXPECT(v.second.is_null());
            BEAST_EXPECT(result.second);
            BEAST_EXPECT(result.first->first == "a");
            auto v2 = object::value_type("a", 2);
            BEAST_EXPECT(
                obj.insert(std::move(v2)).first == result.first);
            BEAST_EXPECT(
                ! obj.insert(std::move(v2)).second);
        }

        // insert(value_type const&)
        {
            object obj;
            auto v = object::value_type("a", 1);
            auto result = obj.insert(v);
            BEAST_EXPECT(! v.second.is_null());
            BEAST_EXPECT(result.second);
            BEAST_EXPECT(result.first->first == "a");
            auto v2 = object::value_type("a", 2);
            BEAST_EXPECT(
                obj.insert(v2).first == result.first);
            BEAST_EXPECT(! obj.insert(v2).second);
        }

        // insert(P&&)
        {
            {
                object obj;
                auto result = obj.insert(
                    std::make_pair("x", 1));
                BEAST_EXPECT(result.second);
                BEAST_EXPECT(result.first->first == "x");
                BEAST_EXPECT(result.first->second.as_number() == 1);
            }
            {
                object obj;
                auto const p = std::make_pair("x", 1);
                auto result = obj.insert(p);
                BEAST_EXPECT(result.second);
                BEAST_EXPECT(result.first->first == "x");
                BEAST_EXPECT(result.first->second.as_number() == 1);
            }
        }

        // insert(before, value_type const&)
        {
            object obj;
            obj.emplace("a", 1);
            obj.emplace("c", "hello");
            object::value_type const p("b", true);
            obj.insert(obj.find("c"), p);
            check(obj, 3);
        }

        // insert(before, value_type&&)
        {
            object obj;
            obj.emplace("a", 1);
            obj.emplace("c", "hello");
            obj.insert(obj.find("c"), { "b", true });
            check(obj, 3);
        }

        // insert(before, P&&)
        {
            {
                object obj;
                obj.emplace("a", 1);
                obj.emplace("c", "hello");
                obj.insert(obj.find("c"),
                    std::make_pair("b", true));
                check(obj, 3);
            }
            {
                object obj;
                obj.emplace("a", 1);
                obj.emplace("c", "hello");
                auto const p =
                    std::make_pair("b", true);
                obj.insert(obj.find("c"), p);
                check(obj, 3);
            }
        }

        // insert(InputIt, InputIt)
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}};
            object obj;
            obj.insert(init.begin(), init.end());
            check(obj, 3);
        }

        // insert(init_list)
        {
            object obj;
            obj.emplace("a", 1);
            obj.insert({
                { "b", true },
                { "c", "hello" }});
            check(obj, 3);
        }

        // insert(node_type&&)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            object obj2;
            obj2.insert(obj1.extract(obj1.begin()));
            obj2.insert(obj1.extract("b"));
            auto result =
                obj2.insert(obj1.extract(obj1.find("c")));
            check(obj2, 3);
            BEAST_EXPECT(obj1.empty());
            BEAST_EXPECT(result.inserted == true);
            BEAST_EXPECT(result.node.empty());
            BEAST_EXPECT(result.position->first == "c");

            // failed insertion
            result = obj2.insert(obj1.extract(
                obj1.insert({"a", 1}).first));
            BEAST_EXPECT(result.inserted == false);
            BEAST_EXPECT(! result.node.empty());
            BEAST_EXPECT(result.position->first == "a");

        }

        // insert(before, node_type&&)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            object obj2;
            obj2.insert(obj1.extract(obj1.begin()));
            obj2.insert(obj1.extract(obj1.find("c")));
            auto result =
                obj2.insert(obj2.find("c"), obj1.extract("b"));
            check(obj2, 3);
            BEAST_EXPECT(obj1.empty());
            BEAST_EXPECT(result.inserted == true);
            BEAST_EXPECT(result.node.empty());
            BEAST_EXPECT(result.position->first == "b");

            // failed insertion
            result = obj2.insert(obj2.find("c"),
                obj1.extract(obj1.insert({"b", 1}).first));
            BEAST_EXPECT(result.inserted == false);
            BEAST_EXPECT(! result.node.empty());
            BEAST_EXPECT(result.position->first == "b");
        }

        // insert_or_assign(key, obj);
        {
            {
                object obj({{"a", 1}});
                obj.insert_or_assign("b", true);
                obj.insert_or_assign("c", "hello");
                check(obj, 3);
            }
            {
                object obj({{"a", 1}});
                BEAST_EXPECT(
                    ! obj.insert_or_assign("a", 2).second);
                BEAST_EXPECT(obj["a"].as_number() == 2);
            }
        }

        // insert_or_assign(before, key, obj);
        {
            {
                object obj({{"a", 1}});
                obj.insert_or_assign("c", "hello");
                obj.insert_or_assign(obj.find("c"), "b", true);
                check(obj, 3);
            }
            {
                object obj({{"a", 1}});
                obj.insert_or_assign("b", true);
                obj.insert_or_assign("c", "hello");
                BEAST_EXPECT(! obj.insert_or_assign(
                    obj.find("b"), "a", 2).second);
                BEAST_EXPECT(obj["a"].as_number() == 2);
            }
        }

        // emplace(key, arg)
        {
            object obj;
            obj.emplace("a", 1);
            obj.emplace("b", true);
            obj.emplace("c", "hello");
            check(obj, 3);
        }

        // emplace(before, key, arg)
        {
            object obj;
            obj.emplace("a", 1);
            obj.emplace("c", "hello");
            obj.emplace(obj.find("c"), "b", true);
            check(obj, 3);
        }

        // erase(pos)
        {
            object obj({
                {"d", nullptr },
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            auto it = obj.erase(obj.begin());
            BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it->second.as_number() == 1);
            check(obj, 7);
        }

        // erase(first, last)
        {
            object obj({
                {"a", 1},
                {"b", true},
                {"b2", 2},
                {"b3", 3},
                {"b4", 4},
                {"c", "hello"}});
            auto first = obj.find("b2");
            auto last = std::next(first, 3);
            auto it = obj.erase(first, last);
            BEAST_EXPECT(it->first == "c");
            BEAST_EXPECT(
                it->second.as_string() == "hello");
            check(obj, 7);
        }

        // erase(key)
        {
            object obj({
                {"a", 1},
                {"b", true},
                {"b2", 2},
                {"c", "hello"}});
            BEAST_EXPECT(obj.erase("b2") == 1);
            check(obj, 7);
        }
    }

    void
    testLookup()
    {
        // at(key)
        {
            object obj;
            try
            {
                obj.at("a");
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // at(key) const
        {
            object const obj;
            try
            {
                obj.at("a");
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // operator[](key)
        {
            object obj;
            obj["a"].emplace_bool() = true;
            BEAST_EXPECT(obj.find("a") != obj.end());
        }

        // operator[](key) const
        {
            object const obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(obj["a"].is_number());
        }

        // count(key)
        {
            object const obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(obj.count("b") == 1);
            BEAST_EXPECT(obj.count("d") == 0);
        }

        // count(key, hash)
        {
            object const obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            auto const hash =
                obj.hash_function()("b");
            BEAST_EXPECT(obj.count("b", hash) == 1);
        }

        // find(key)
        {
            object const obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(
                obj.find("b")->second.is_bool());
        }

        // find(key, hash)
        {
            object const obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            auto const hash =
                obj.hash_function()("c");
            BEAST_EXPECT(obj.find(
                "c", hash)->second.is_string());
        }

        // contains(key)
        {
            object const obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(obj.contains("a"));
            BEAST_EXPECT(! obj.contains("d"));
        }

        // find(key, hash)
        {
            object const obj({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(obj.contains(
                "b", obj.hash_function()("b")));
            BEAST_EXPECT(! obj.contains(
                "d", obj.hash_function()("d")));
        }


    }

    void
    testBuckets()
    {
    }

    void
    testHashPolicy()
    {
        object obj;
        for(std::size_t i = 0; i < 1000; ++i)
            obj.emplace(
                std::to_string(i),
                i);
    }

    void
    testObservers()
    {
        // hash_function
        {
            object obj;
            object::hasher h = obj.hash_function();
            boost::ignore_unused(h);
        }

        // key_eq
        {
            object obj;
            object::key_equal eq = obj.key_eq();
            BEAST_EXPECT(eq("a", "a"));
        }
    }

    void
    testNodeType()
    {
        BOOST_STATIC_ASSERT(
            std::is_same<
                object::node_type::key_type,
                object::key_type>::value);

        BOOST_STATIC_ASSERT(
            std::is_same<
                object::node_type::mapped_type,
                object::mapped_type>::value);

        // node_type()
        {
            object::node_type nh;
            BEAST_EXPECT(nh.empty());
            BEAST_EXPECT(! nh);
            BEAST_EXPECT(
                nh.get_storage() == nullptr);
        }

        // node_type(node_type&&)
        {
            {
                object::node_type nh1;
                object::node_type nh2(std::move(nh1));
                BEAST_EXPECT(nh1.empty());
                BEAST_EXPECT(nh2.empty());
                BEAST_EXPECT(
                    nh1.get_storage() == nullptr);
                BEAST_EXPECT(
                    nh2.get_storage() == nullptr);
            }
            {
                object obj({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object::node_type nh;
                BEAST_EXPECT(nh.empty());
                nh = obj.extract("b");
                BEAST_EXPECT(! nh.empty());
                BEAST_EXPECT(nh.key() == "b");
                BEAST_EXPECT(
                    nh.get_storage() ==
                    obj.get_storage());
                auto nh2 = std::move(nh);
                BEAST_EXPECT(nh.empty());
                BEAST_EXPECT(
                    nh.get_storage() == nullptr);
                BEAST_EXPECT(! nh2.empty());
                BEAST_EXPECT(nh2.key() == "b");
                BEAST_EXPECT(
                    nh2.get_storage() ==
                    obj.get_storage());
            }
        }
    }

    void
    testExceptions()
    {
        // operator=(object const&)
        {
            object obj0({
                { "a", 1 },
                { "b", true },
                { "c", "hello" }});
            auto sp = make_storage<fail_storage>();
            object obj1;
            while(sp->fail < 200)
            {
                try
                {
                    object obj(sp);
                    obj.emplace("a", 2);
                    obj = obj0;
                    obj1 = obj;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            check(obj1, 3);
        }

        // operator=(object&&)
        {
            auto sp = make_storage<fail_storage>();
            object obj1;
            while(sp->fail < 200)
            {
                try
                {
                    object obj0({
                        { "a", 1 },
                        { "b", true },
                        { "c", "hello" }});
                    object obj(sp);
                    obj.emplace("a", 2);
                    obj = std::move(obj0);
                    obj1 = obj;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            check(obj1, 3);
        }

        // operator=(init_list)
        {
            auto sp = make_storage<fail_storage>();
            object obj1;
            while(sp->fail < 200)
            {
                try
                {
                    object obj(sp);
                    obj.emplace("a", 2);
                    obj = {
                        { "a", 1 },
                        { "b", true },
                        { "c", "hello" }};
                    obj1 = obj;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            check(obj1, 3);
        }
    }

    void
    run() override
    {
        testSpecial();
        testIterators();
        testModifiers();
        testLookup();
        testBuckets();
        testHashPolicy();
        testObservers();
        testNodeType();
        testExceptions();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,object);

} // json
} // boost
