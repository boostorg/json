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
    static
    void
    check(
        object const& o,
        std::size_t bucket_count)
    {
        BEAST_EXPECT(! o.empty());
        BEAST_EXPECT(o.size() == 3);
        BEAST_EXPECT(
            o.bucket_count() == bucket_count);
        BEAST_EXPECT(o["a"].as_number() == 1);
        BEAST_EXPECT(o["b"].as_bool());
        BEAST_EXPECT(o["c"].as_string() == "hello");

        // ordering, storage

        auto it = o.begin();
        BEAST_EXPECT(it->first == "a");
        BEAST_EXPECT(
            *it->second.get_storage() ==
            *o.get_storage());

        ++it;
        BEAST_EXPECT(it->first == "b");
        BEAST_EXPECT(
            *it->second.get_storage() ==
            *o.get_storage());

        it++;
        BEAST_EXPECT(it->first == "c");
        BEAST_EXPECT(
            *it->second.get_storage() ==
            *o.get_storage());
    }

    void
    testSpecial()
    {
        // ~object()
        {
            // implied
        }

        // object()
        {
            scoped_fail_storage fs;
            object o;
            BEAST_EXPECT(o.empty());
            BEAST_EXPECT(o.size() == 0);
            BEAST_EXPECT(o.bucket_count() == 0);
        }

        // object(storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            object o(sp);
            check_storage(o, sp);
        });

        // object(size_type)
        fail_loop([]
        {
            object o(50);
            BEAST_EXPECT(o.empty());
            BEAST_EXPECT(o.size() == 0);
            BEAST_EXPECT(o.bucket_count() == 53);
        });

        // object(size_type, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            object o(50, sp);
            BEAST_EXPECT(o.empty());
            BEAST_EXPECT(o.size() == 0);
            BEAST_EXPECT(o.bucket_count() == 53);
            check_storage(o, sp);
        });

        // object(InputIt, InputIt)
        //fail_loop([]
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}};
            object o(init.begin(), init.end());
            check(o, 3);
        }//);

        // object(InputIt, InputIt, size_type)
        fail_loop([]
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}};
            object o(init.begin(), init.end(), 5);
            check(o, 7);
        });

        // object(InputIt, InputIt, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}};
            object o(init.begin(), init.end(), sp);
            check(o, 3);
            check_storage(o, sp);
        });

        // object(InputIt, InputIt, size_type, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}};
            object o(init.begin(), init.end(), 5, sp);
            BEAST_EXPECT(! o.empty());
            BEAST_EXPECT(o.size() == 3);
            BEAST_EXPECT(o.bucket_count() == 7);
            check(o, 7);
            check_storage(o, sp);
        });

        // object(object&&)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            check(obj1, 3);
            auto const sp =
                default_storage();
            scoped_fail_storage fs;
            object obj2(std::move(obj1));
            BEAST_EXPECT(obj1.empty());
            BEAST_EXPECT(obj1.size() == 0);
            check(obj2, 3);
            check_storage(obj1, sp);
            check_storage(obj2, sp);
        }

        // object(pilfered<object>)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            scoped_fail_storage fs;
            object obj2(pilfer(obj1));
            BEAST_EXPECT(
                obj1.get_storage() == nullptr);
            BEAST_EXPECT(obj1.empty());
            check(obj2, 3);
        }

        auto const sp = make_storage<unique_storage>();
        auto const sp0 = default_storage();

        // object(object&&, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object obj2(std::move(obj1), sp);
            BEAST_EXPECT(! obj1.empty());
            check(obj2, 3);
            check_storage(obj1,
                default_storage());
            check_storage(obj2, sp);
        });

        // object(object const&)
        fail_loop([]
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object obj2(obj1);
            BEAST_EXPECT(! obj1.empty());
            check(obj2, 3);
        });

        // object(object const&, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            object obj1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object obj2(obj1, sp);
            BEAST_EXPECT(! obj1.empty());
            check(obj2, 3);
            check_storage(obj2, sp);
        });

        // object(initializer_list)
        fail_loop([]
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            check(o, 3);
        });

        // object(initializer_list, size_type)
        fail_loop([]
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                },
                5);
            check(o, 7);
        });

        // object(initializer_list, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                },
                sp);
            check(o, 3);
            check_storage(o, sp);
        });

        // object(initializer_list, size_type, storage_ptr)
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                },
                5, sp);
            BEAST_EXPECT(
                *o.get_storage() == *sp);
            check(o, 7);
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
                check_storage(obj1,
                    default_storage());
                check_storage(obj2,
                    default_storage());
            }

            fail_loop([](storage_ptr const& sp)
            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2(sp);
                obj2 = std::move(obj1);
                check(obj1, 3);
                check(obj2, 3);
                check_storage(obj1,
                    default_storage());
                check_storage(obj2, sp);
            });
        }

        // operator=(object const&)
        {
            fail_loop([]
            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2;
                obj2 = obj1;
                check(obj1, 3);
                check(obj2, 3);
                check_storage(obj1,
                    default_storage());
                check_storage(obj2,
                    default_storage());
            });

            fail_loop([](storage_ptr const& sp)
            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2(sp);
                obj2 = obj1;
                check(obj1, 3);
                check(obj2, 3);
                check_storage(obj1,
                    default_storage());
                check_storage(obj2, sp);
            });
        }

        // operator=(initializer_list)
        {
            fail_loop([]
            {
                object o;
                o = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"} },
                check(o, 3);
                check_storage(o,
                    default_storage());
            });

            fail_loop([](storage_ptr const& sp)
            {
                object o(sp);
                o = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"} },
                BEAST_EXPECT(
                    *o.get_storage() == *sp);
                check(o, 3);
                check_storage(o, sp);
            });
        }
    }

    void
    testIterators()
    {
        object o({
            {"a", 1},
            {"b", true},
            {"c", "hello"}});

        {
            auto it = o.begin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == o.end());
        }
        {
            auto it = o.cbegin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == o.cend());
        }
        {
            auto it = static_cast<
                object const&>(o).begin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == static_cast<
                object const&>(o).end());
        }
        {
            auto it = o.end();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == o.begin());
        }
        {
            auto it = o.cend();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == o.cbegin());
        }
        {
            auto it = static_cast<
                object const&>(o).end();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == static_cast<
                object const&>(o).begin());
        }

#if 0
        {
            auto it = o.rbegin();
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it == o.rend());
        }
        {
            auto it = o.crbegin();
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it == o.crend());
        }
        {
            auto it = static_cast<
                object const&>(o).rbegin();
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it == static_cast<
                object const&>(o).rend());
        }
        {
            auto it = o.rend();
            --it; BEAST_EXPECT(it->first == "a");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "c");
            BEAST_EXPECT(it == o.rbegin());
        }
        {
            auto it = o.crend();
            --it; BEAST_EXPECT(it->first == "a");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "c");
            BEAST_EXPECT(it == o.crbegin());
        }
        {
            auto it = static_cast<
                object const&>(o).rend();
            --it; BEAST_EXPECT(it->first == "a");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "c");
            BEAST_EXPECT(it == static_cast<
                object const&>(o).rbegin());
        }
#endif
    }

    void
    testModifiers()
    {
        // clear
        {
            object o;
            o.emplace("x", 1);
            BEAST_EXPECT(! o.empty());
            o.clear();
            BEAST_EXPECT(o.empty());
        }

        // insert(value_type&&)
        fail_loop([]
        {
            object o;
            auto v = object::value_type("a", 1);
            auto result = o.insert(std::move(v));
            BEAST_EXPECT(v.second.is_null());
            BEAST_EXPECT(result.second);
            BEAST_EXPECT(result.first->first == "a");
            auto v2 = object::value_type("a", 2);
            BEAST_EXPECT(
                o.insert(std::move(v2)).first == result.first);
            BEAST_EXPECT(
                ! o.insert(std::move(v2)).second);
        });

        // insert(value_type const&)
        fail_loop([]
        {
            object o;
            auto v = object::value_type("a", 1);
            auto result = o.insert(v);
            BEAST_EXPECT(! v.second.is_null());
            BEAST_EXPECT(result.second);
            BEAST_EXPECT(result.first->first == "a");
            auto v2 = object::value_type("a", 2);
            BEAST_EXPECT(
                o.insert(v2).first == result.first);
            BEAST_EXPECT(! o.insert(v2).second);
        });

        // insert(P&&)
        {
            fail_loop([]
            {
                object o;
                auto result = o.insert(
                    std::make_pair("x", 1));
                BEAST_EXPECT(result.second);
                BEAST_EXPECT(result.first->first == "x");
                BEAST_EXPECT(result.first->second.as_number() == 1);
            });

            fail_loop([]
            {
                object o;
                auto const p = std::make_pair("x", 1);
                auto result = o.insert(p);
                BEAST_EXPECT(result.second);
                BEAST_EXPECT(result.first->first == "x");
                BEAST_EXPECT(result.first->second.as_number() == 1);
            });
        }

        // insert(before, value_type const&)
        fail_loop([]
        {
            object o;
            o.emplace("a", 1);
            o.emplace("c", "hello");
            object::value_type const p("b", true);
            o.insert(o.find("c"), p);
            check(o, 3);
        });

        // insert(before, value_type&&)
        fail_loop([]
        {
            object o;
            o.emplace("a", 1);
            o.emplace("c", "hello");
            o.insert(o.find("c"), { "b", true });
            check(o, 3);
        });

        // insert(before, P&&)
        {
            fail_loop([]
            {
                object o;
                o.emplace("a", 1);
                o.emplace("c", "hello");
                o.insert(o.find("c"),
                    std::make_pair("b", true));
                check(o, 3);
            });

            fail_loop([]
            {
                object o;
                o.emplace("a", 1);
                o.emplace("c", "hello");
                auto const p =
                    std::make_pair("b", true);
                o.insert(o.find("c"), p);
                check(o, 3);
            });
        }

        // insert(InputIt, InputIt)
        fail_loop([]
        {
            std::initializer_list<std::pair<
                string_view, value>> init = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}};
            object o;
            o.insert(init.begin(), init.end());
            check(o, 3);
        });

        // insert(initializer_list)
        fail_loop([]
        {
            object o;
            o.emplace("a", 1);
            o.insert({
                { "b", true },
                { "c", "hello" }});
            check(o, 3);
        });

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
        #if 0
            // VFALCO This no longer compiles
            result = obj2.insert(obj1.extract(
                obj1.insert({"a", 1}).first));
            BEAST_EXPECT(result.inserted == false);
            BEAST_EXPECT(! result.node.empty());
            BEAST_EXPECT(result.position->first == "a");
        #endif

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
        #if 0
            // VFALCO This no longer compiles
            result = obj2.insert(obj2.find("c"),
                obj1.extract(obj1.insert({"b", 1}).first));
            BEAST_EXPECT(result.inserted == false);
            BEAST_EXPECT(! result.node.empty());
            BEAST_EXPECT(result.position->first == "b");
        #endif
        }

        // insert_or_assign(key, o);
        {
            fail_loop([]
            {
                object o({{"a", 1}});
                o.insert_or_assign("b", true);
                o.insert_or_assign("c", "hello");
                check(o, 3);
            });

            fail_loop([]
            {
                object o({{"a", 1}});
                BEAST_EXPECT(
                    ! o.insert_or_assign("a", 2).second);
                BEAST_EXPECT(o["a"].as_number() == 2);
            });
        }

        // insert_or_assign(before, key, o);
        {
            fail_loop([]
            {
                object o({{"a", 1}});
                o.insert_or_assign("c", "hello");
                o.insert_or_assign(o.find("c"), "b", true);
                check(o, 3);
            });

            fail_loop([]
            {
                object o({{"a", 1}});
                o.insert_or_assign("b", true);
                o.insert_or_assign("c", "hello");
                BEAST_EXPECT(! o.insert_or_assign(
                    o.find("b"), "a", 2).second);
                BEAST_EXPECT(o["a"].as_number() == 2);
            });
        }

        // emplace(key, arg)
        fail_loop([]
        {
            object o;
            o.emplace("a", 1);
            o.emplace("b", true);
            o.emplace("c", "hello");
            check(o, 3);
        });

        // emplace(before, key, arg)
        fail_loop([]
        {
            object o;
            o.emplace("a", 1);
            o.emplace("c", "hello");
            o.emplace(o.find("c"), "b", true);
            check(o, 3);
        });

        // erase(pos)
        fail_loop([]
        {
            object o({
                {"d", nullptr },
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            auto it = o.erase(o.begin());
            BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it->second.as_number() == 1);
            check(o, 7);
        });

        // erase(first, last)
        fail_loop([]
        {
            object o({
                {"a", 1},
                {"b", true},
                {"b2", 2},
                {"b3", 3},
                {"b4", 4},
                {"c", "hello"}});
            auto first = o.find("b2");
            auto last = std::next(first, 3);
            auto it = o.erase(first, last);
            BEAST_EXPECT(it->first == "c");
            BEAST_EXPECT(
                it->second.as_string() == "hello");
            check(o, 7);
        });

        // erase(key)
        fail_loop([]
        {
            object o({
                {"a", 1},
                {"b", true},
                {"b2", 2},
                {"c", "hello"}});
            BEAST_EXPECT(o.erase("b2") == 1);
            check(o, 7);
        });
    }

    void
    testLookup()
    {
        // at(key)
        {
            object o;
            try
            {
                o.at("a");
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // at(key) const
        {
            object const o;
            try
            {
                o.at("a");
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // operator[](key)
        {
            object o;
            o["a"].emplace_bool() = true;
            BEAST_EXPECT(o.find("a") != o.end());
        }

        // operator[](key) const
        {
            object const o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(o["a"].is_number());
        }

        // count(key)
        {
            object const o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(o.count("b") == 1);
            BEAST_EXPECT(o.count("d") == 0);
        }

        // find(key)
        {
            object const o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(
                o.find("b")->second.is_bool());
        }

        // contains(key)
        {
            object const o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(o.contains("a"));
            BEAST_EXPECT(! o.contains("d"));
        }
    }

    void
    testBuckets()
    {
    }

    void
    testHashPolicy()
    {
        object o;
        for(std::size_t i = 0; i < 1000; ++i)
            o.emplace(
                std::to_string(i),
                i);
    }

    void
    testObservers()
    {
        // hash_function
        {
            object o;
            object::hasher h = o.hash_function();
            boost::ignore_unused(h);
        }

        // key_eq
        {
            object o;
            object::key_equal eq = o.key_eq();
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
                object o({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object::node_type nh;
                BEAST_EXPECT(nh.empty());
                nh = o.extract("b");
                BEAST_EXPECT(! nh.empty());
                BEAST_EXPECT(nh.key() == "b");
                BEAST_EXPECT(
                    nh.get_storage() ==
                    o.get_storage());
                auto nh2 = std::move(nh);
                BEAST_EXPECT(nh.empty());
                BEAST_EXPECT(
                    nh.get_storage() == nullptr);
                BEAST_EXPECT(! nh2.empty());
                BEAST_EXPECT(nh2.key() == "b");
                BEAST_EXPECT(
                    nh2.get_storage() ==
                    o.get_storage());
            }
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
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,object);

} // json
} // boost
