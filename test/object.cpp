//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/object.hpp>

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/number_cast.hpp>

#include <cmath>
#include <type_traits>

#include "test.hpp"
#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

class object_test
{
public:
    test_suite::log_type log;
    string_view const str_;

    object_test()
        : str_(
            "abcdefghijklmnopqrstuvwxyz")
    {
        // ensure this string does
        // not fit in the SBO area.
        BOOST_ASSERT(str_.size() >
            string().capacity());
    }

    template<class T, class U, class = void>
    struct is_equal_comparable : std::false_type {};
    
    template<class T, class U>
    struct is_equal_comparable<T, U, detail::void_t<decltype(
        std::declval<T const&>() == std::declval<U const&>()
            )>> : std::true_type {};

    template<class T, class U, class = void>
    struct is_unequal_comparable : std::false_type {};
    
    template<class T, class U>
    struct is_unequal_comparable<T, U, detail::void_t<decltype(
        std::declval<T const&>() != std::declval<U const&>()
            )>> : std::true_type {};

    BOOST_STATIC_ASSERT(  std::is_constructible<object::iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_constructible<object::iterator, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_assignable<object::iterator&, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_assignable<object::iterator&, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_iterator&, object::iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_iterator&, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::iterator, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::const_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_constructible<object::reverse_iterator, object::reverse_iterator>::value);
    // std::reverse_iterator ctor is not SFINAEd
    //BOOST_STATIC_ASSERT(! std::is_constructible<object::reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_reverse_iterator, object::reverse_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_assignable<object::reverse_iterator&, object::reverse_iterator>::value);
    // std::reverse_iterator assignment is not SFINAEd
    //BOOST_STATIC_ASSERT(! std::is_assignable<object::reverse_iterator&, object::const_reverse_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_reverse_iterator&, object::reverse_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_reverse_iterator&, object::const_reverse_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::reverse_iterator, object::reverse_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_reverse_iterator, object::reverse_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::reverse_iterator, object::reverse_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_reverse_iterator, object::reverse_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_reverse_iterator, object::const_reverse_iterator>::value);
    
    static
    void
    check(
        object const& o,
        std::size_t capacity)
    {
        BOOST_TEST(! o.empty());
        BOOST_TEST(o.size() == 3);
        BOOST_TEST(
            o.capacity() == capacity);
        BOOST_TEST(o.at("a").as_int64() == 1);
        BOOST_TEST(o.at("b").as_bool());
        BOOST_TEST(o.at("c").as_string() == "hello");
        check_storage(o, o.storage());
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
            object o;
            BOOST_TEST(o.empty());
            BOOST_TEST(o.size() == 0);
            BOOST_TEST(o.capacity() == 0);
        }

        // object(storage_ptr)
        fail_loop([&](storage_ptr const& sp)
        {
            object o(sp);
            check_storage(o, sp);
            BOOST_TEST(o.empty());
            BOOST_TEST(o.size() == 0);
            BOOST_TEST(o.capacity() == 0);
        });

        // object(std::size_t, storage_ptr)
        {
            {
                object o(10);
                BOOST_TEST(o.empty());
                BOOST_TEST(o.size() == 0);
                BOOST_TEST(o.capacity() >= 10);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o(10, sp);
                check_storage(o, sp);
                BOOST_TEST(o.empty());
                BOOST_TEST(o.size() == 0);
                BOOST_TEST(o.capacity() >= 10);
            });
        }

        // object(InputIt, InputIt, size_type, storage_ptr)
        {
            {
                std::initializer_list<std::pair<
                    string_view, value>> init = {
                        {"a", 1},
                        {"b", true},
                        {"c", "hello"}};
                object o(init.begin(), init.end());
                check(o, 3);
            }

            {
                std::initializer_list<std::pair<
                    string_view, value>> init = {
                        {"a", 1},
                        {"b", true},
                        {"c", "hello"}};
                object o(init.begin(), init.end(), 5);
                check(o, 7);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<std::pair<
                    string_view, value>> init = {
                        {"a", 1},
                        {"b", true},
                        {"c", "hello"}};
                object o(init.begin(), init.end(), 5, sp);
                BOOST_TEST(! o.empty());
                BOOST_TEST(o.size() == 3);
                BOOST_TEST(o.capacity() == 7);
                check(o, 7);
                check_storage(o, sp);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<std::pair<
                    string_view, value>> init = {
                        {"a", 1},
                        {"b", true},
                        {"c", "hello"}};
                object o(
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end()),
                    5, sp);
                BOOST_TEST(! o.empty());
                BOOST_TEST(o.size() == 3);
                BOOST_TEST(o.capacity() == 7);
                check(o, 7);
                check_storage(o, sp);
            });
        }

        // object(object&&)
        {
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            check(o1, 3);
            auto const sp =
                storage_ptr{};
            object o2(std::move(o1));
            BOOST_TEST(o1.empty());
            BOOST_TEST(o1.size() == 0);
            check(o2, 3);
            check_storage(o1, sp);
            check_storage(o2, sp);
        }

        // object(object&&, storage_ptr)
        fail_loop([&](storage_ptr const& sp)
        {
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object o2(std::move(o1), sp);
            BOOST_TEST(! o1.empty());
            check(o2, 3);
            check_storage(o1,
                storage_ptr{});
            check_storage(o2, sp);
        });

        // object(pilfered<object>)
        {
            auto const sp =
                make_counted_resource<unique_resource>();
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                }, sp);
            object o2(pilfer(o1));
            BOOST_TEST(
                o1.storage() == storage_ptr());
            BOOST_TEST(
                *o2.storage() == *sp);
            BOOST_TEST(o1.empty());
            check(o2, 3);
        }

        auto const sp = make_counted_resource<unique_resource>();
        auto const sp0 = storage_ptr{};

        // object(object const&)
        {
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object o2(o1);
            BOOST_TEST(! o1.empty());
            check(o2, 3);
        }

        // object(object const&, storage_ptr)
        fail_loop([&](storage_ptr const& sp)
        {
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object o2(o1, sp);
            BOOST_TEST(! o1.empty());
            check(o2, 3);
            check_storage(o2, sp);
        });

        // object(initializer_list, storage_ptr)
        {
            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                check(o, 3);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}},
                    sp);
                check(o, 3);
                check_storage(o, sp);
            });
        }

        // object(initializer_list, std::size_t, storage_ptr)
        {
            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}},
                    5);
                check(o, 7);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}
                    },
                    5, sp);
                BOOST_TEST(
                    *o.storage() == *sp);
                check(o, 7);
            });
        }

        // operator=(object&&)
        {
            {
                object o1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object o2;
                o2 = std::move(o1);
                check(o2, 3);
                BOOST_TEST(o1.empty());
                check_storage(o1,
                    storage_ptr{});
                check_storage(o2,
                    storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object o2(sp);
                o2 = std::move(o1);
                check(o1, 3);
                check(o2, 3);
                check_storage(o1,
                    storage_ptr{});
                check_storage(o2, sp);
            });
        }

        // operator=(object const&)
        {
            {
                object o1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object o2;
                o2 = o1;
                check(o1, 3);
                check(o2, 3);
                check_storage(o1,
                    storage_ptr{});
                check_storage(o2,
                    storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object o2(sp);
                o2 = o1;
                check(o1, 3);
                check(o2, 3);
                check_storage(o1,
                    storage_ptr{});
                check_storage(o2, sp);
            });

            // self-assign
            {
                object o1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object const& o2(o1);
                o1 = o2;
                check(o1, 3);
            }
        }

        // operator=(initializer_list)
        {
            {
                object o;
                o = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"} },
                check(o, 3);
                check_storage(o,
                    storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o(sp);
                o = {
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"} },
                BOOST_TEST(
                    *o.storage() == *sp);
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
        auto const& co = o;
        object no;
        auto const& cno = no;

        // empty container
        {
            BOOST_TEST(no.begin() == no.end());
            BOOST_TEST(cno.begin() == cno.end());
            BOOST_TEST(no.cbegin() == no.cend());
        }

        // begin()
        {
            auto it = o.begin();
            BOOST_TEST(it->key() == "a"); ++it;
            BOOST_TEST(it->key() == "b"); it++;
            BOOST_TEST(it->key() == "c"); ++it;
            BOOST_TEST(it == o.end());
        }

        // begin() const
        {
            auto it = co.begin();
            BOOST_TEST(it->key() == "a"); ++it;
            BOOST_TEST(it->key() == "b"); it++;
            BOOST_TEST(it->key() == "c"); ++it;
            BOOST_TEST(it == co.end());
        }

        // cbegin()
        {
            auto it = o.cbegin();
            BOOST_TEST(it->key() == "a"); ++it;
            BOOST_TEST(it->key() == "b"); it++;
            BOOST_TEST(it->key() == "c"); ++it;
            BOOST_TEST(it == o.cend());
        }

        // end()
        {
            auto it = o.end();
            --it; BOOST_TEST(it->key() == "c");
            it--; BOOST_TEST(it->key() == "b");
            --it; BOOST_TEST(it->key() == "a");
            BOOST_TEST(it == o.begin());
        }

        // end() const
        {
            auto it = co.end();
            --it; BOOST_TEST(it->key() == "c");
            it--; BOOST_TEST(it->key() == "b");
            --it; BOOST_TEST(it->key() == "a");
            BOOST_TEST(it == co.begin());
        }

        // cend()
        {
            auto it = o.cend();
            --it; BOOST_TEST(it->key() == "c");
            it--; BOOST_TEST(it->key() == "b");
            --it; BOOST_TEST(it->key() == "a");
            BOOST_TEST(it == o.cbegin());
        }

        // rbegin()
        {
            auto it = o.rbegin();
            BOOST_TEST(it->key() == "c"); ++it;
            BOOST_TEST(it->key() == "b"); it++;
            BOOST_TEST(it->key() == "a"); ++it;
            BOOST_TEST(it == o.rend());
        }

        // rbegin() const
        {
            auto it = co.rbegin();
            BOOST_TEST(it->key() == "c"); ++it;
            BOOST_TEST(it->key() == "b"); it++;
            BOOST_TEST(it->key() == "a"); ++it;
            BOOST_TEST(it == co.rend());
        }

        // crbegin()
        {
            auto it = o.crbegin();
            BOOST_TEST(it->key() == "c"); ++it;
            BOOST_TEST(it->key() == "b"); it++;
            BOOST_TEST(it->key() == "a"); ++it;
            BOOST_TEST(it == o.crend());
        }

        // rend()
        {
            auto it = o.rend();
            --it; BOOST_TEST(it->key() == "a");
            it--; BOOST_TEST(it->key() == "b");
            --it; BOOST_TEST(it->key() == "c");
            BOOST_TEST(it == o.rbegin());
        }

        // rend() const
        {
            auto it = co.rend();
            --it; BOOST_TEST(it->key() == "a");
            it--; BOOST_TEST(it->key() == "b");
            --it; BOOST_TEST(it->key() == "c");
            BOOST_TEST(it == co.rbegin());
        }

        // crend()
        {
            auto it = o.crend();
            --it; BOOST_TEST(it->key() == "a");
            it--; BOOST_TEST(it->key() == "b");
            --it; BOOST_TEST(it->key() == "c");
            BOOST_TEST(it == o.crbegin());
        }
    }

    //------------------------------------------------------

    void
    testCapacity()
    {
        BOOST_TEST(
            object{}.size() < object{}.max_size());
    }

    //------------------------------------------------------

    void
    testModifiers()
    {
        // clear
        {
            {
                object o;
                o.clear();
                BOOST_TEST(o.empty());
            }

            {
                object o;
                o.emplace("x", 1);
                BOOST_TEST(! o.empty());
                o.clear();
                BOOST_TEST(o.empty());
            }
        }

        // insert(P&&)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                object o(sp);
                auto result = o.insert(
                    std::make_pair("x", 1));
                BOOST_TEST(result.second);
                BOOST_TEST(result.first->key() == "x");
                BOOST_TEST(result.first->value().as_int64() == 1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o(sp);
                auto const p = std::make_pair("x", 1);
                auto result = o.insert(p);
                BOOST_TEST(result.second);
                BOOST_TEST(result.first->key() == "x");
                BOOST_TEST(result.first->value().as_int64() == 1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o({
                    {"a", 1},
                    {"b", 2},
                    {"c", 3}}, sp);
                auto const result = o.insert(
                    std::make_pair("b", 4));
                BOOST_TEST(
                    result.first->value().as_int64() == 2);
                BOOST_TEST(! result.second);
            });
        }

        // insert(InputIt, InputIt)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<std::pair<
                    string_view, value>> init = {
                        {"a", 1},
                        {"b", true},
                        {"c", "hello"}};
                object o(sp);
                o.insert(init.begin(), init.end());
                check(o, 3);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<std::pair<
                    string_view, value>> init = {
                        {"a", 1},
                        {"b", true},
                        {"c", "hello"}};
                object o(sp);
                o.insert(
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end()));
                check(o, 3);
            });
        }

        // insert(initializer_list)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                object o(sp);
                o.emplace("a", 1);
                o.insert({
                    { "b", true },
                    { "c", "hello" }});
                check(o, 3);
            });

            // do rollback in ~undo_insert
            fail_loop([&](storage_ptr const& sp)
            {
                object o(sp);
                o.insert({
                    { "a", { 1, 2, 3, 4 } } });
            });
        }

        // insert_or_assign(key, o);
        {
            fail_loop([&](storage_ptr const& sp)
            {
                object o({{"a", 1}}, sp);
                o.insert_or_assign("a", str_);
                BOOST_TEST(o["a"].is_string());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o({
                    {"a", 1},
                    {"b", 2},
                    {"c", 3}}, sp);
                o.insert_or_assign("d", str_);
                BOOST_TEST(o["d"].is_string());
                BOOST_TEST(o.size() == 4);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o({{"a", 1}}, sp);
                o.insert_or_assign("b", true);
                o.insert_or_assign("c", "hello");
                check(o, 3);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o({{"a", 1}}, sp);
                BOOST_TEST(
                    ! o.insert_or_assign("a", 2).second);
                BOOST_TEST(o["a"].as_int64() == 2);
            });
        }

        // emplace(key, arg)
        fail_loop([&](storage_ptr const& sp)
        {
            object o(sp);
            o.emplace("a", 1);
            o.emplace("b", true);
            o.emplace("c", "hello");
            check(o, 3);
        });

        // erase(pos)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                object o({
                    {"d", nullptr },
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}}, sp);
                auto it = o.erase(o.begin());
                BOOST_TEST(it->key() == "c");
                BOOST_TEST(
                    it->value().as_string() == "hello");
                check(o, 7);
            });
        }

        // erase(key)
        {
            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                BOOST_TEST(o.erase("b2") == 0);
                check(o, 3);
            }

            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"b2", 2},
                    {"c", "hello"}});
                BOOST_TEST(o.erase("b2") == 1);
                check(o, 7);
            }
        }

        // swap(object&)
        {
            {
                object o1 = {{"a",1}, {"b",true}, {"c", "hello"}};
                object o2 = {{"d",{1,2,3}}};
                swap(o1, o2);
                BOOST_TEST(o1.size() == 1);
                BOOST_TEST(o2.size() == 3);
                BOOST_TEST(o1.count("d") == 1);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o1 = {{"a",1}, {"b",true}, {"c", "hello"}};
                object o2({{"d",{1,2,3}}}, sp);
                swap(o1, o2);
                BOOST_TEST(o1.size() == 1);
                BOOST_TEST(o2.size() == 3);
                BOOST_TEST(o1.count("d") == 1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o1({{"d",{1,2,3}}}, sp);
                object o2 = {{"a",1}, {"b",true}, {"c", "hello"}};
                swap(o1, o2);
                BOOST_TEST(o1.size() == 3);
                BOOST_TEST(o2.size() == 1);
                BOOST_TEST(o2.count("d") == 1);
            });
        }
    }

    //------------------------------------------------------

    void
    testLookup()
    {
        object o1({
            {"a", 1},
            {"b", true},
            {"c", "hello"}});
        auto const& co1 = o1;

        // at(key)
        {
            BOOST_TEST(
                o1.at("a").is_number());
            BOOST_TEST_THROWS((o1.at("d")),
                std::out_of_range);
        }

        // at(key) const
        {
            BOOST_TEST(
                co1.at("a").is_number());
            BOOST_TEST_THROWS((co1.at("d")),
                std::out_of_range);
        }

        // operator[&](key)
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BOOST_TEST(o.count("d") == 0);;
            BOOST_TEST(o["a"].is_number());
            BOOST_TEST(o["d"].is_null());
            BOOST_TEST(o.count("d") == 1);
        }

        // count(key)
        {
            BOOST_TEST(o1.count("a") == 1);
            BOOST_TEST(o1.count("d") == 0);
            BOOST_TEST(o1.count("e") == 0);
        }

        // find(key)
        {
            BOOST_TEST(
                o1.find("a")->key() == "a");
            BOOST_TEST(
                o1.find("e") == o1.end());
        }

        // contains(key)
        {
            BOOST_TEST(o1.contains("a"));
            BOOST_TEST(! o1.contains("e"));

            *o1.contains("a") = 2;
            BOOST_TEST(co1.contains("a")->as_int64() == 2);
        }
    }

    void
    testHashPolicy()
    {
        // reserve(size_type)
        {
            {
                object o;
                for(std::size_t i = 0; i < 10; ++i)
                    o.emplace(std::to_string(i), i);
                o.reserve(15);
                BOOST_TEST(o.capacity() >= 15);
                o.reserve(20);
                BOOST_TEST(o.capacity() >= 20);
            }

            {
                object o;
                o.reserve(3);
                BOOST_TEST(o.capacity() == 3);
                o.reserve(7);
                BOOST_TEST(o.capacity() == 7);
            }
        }
    }

    //------------------------------------------------------

    void
    testImplementation()
    {
        // insert duplicate keys
        {
            object o({
                {"a", 1},
                {"b", true},
                {"b", {1,2,3}},
                {"c", "hello"}});
            BOOST_TEST(o.at("a").as_int64() == 1);
            BOOST_TEST(o.at("b").as_bool());
            BOOST_TEST(o.at("c").as_string() == "hello");
        }

        // find in missing or empty tables
        {
            object o;
            BOOST_TEST(o.find("a") == o.end());
            o.reserve(3);
            BOOST_TEST(o.capacity() > 0);
            BOOST_TEST(o.find("a") == o.end());
        }

        // destroy key_value_pair array with need_free=false
        {
            monotonic_resource mr;
            object o({
                {"a", 1},
                {"b", true},
                {"b", {1,2,3}},
                {"c", "hello"}}, &mr);
        }
    }

    static
    string_view
    make_key(
        std::size_t i,
        char* buf)
    {
        int constexpr base = 62;
        char const* const alphabet =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        char* dest = buf;
        do
        {
            *dest++ = alphabet[i%base];
            i /= base;
        }
        while(i);
        return { buf, static_cast<
            std::size_t>(dest-buf) };
    }

    void
    testCollisions()
    {
        int constexpr buckets = 3;
#if 0
        {
            // VFALCO This code finds the collisions
            object o(this); // must create 3 buckets
            char buf[26];
            auto const match =
                o.impl_.digest("0") % buckets;
            std::size_t i = 8;
            for(;;)
            {
                if((o.impl_.digest(
                    make_key(i, buf)) % buckets) == match)
                {
                    log <<
                        "match: \"" <<
                        make_key(i, buf) <<
                        "\"\n";
                    break;
                }
                ++i;
            }
            }
#endif
        // Create an object with 3 keys that
        // hash to the same bucket.

    #if BOOST_JSON_ARCH == 32
        string_view k1 = "0";
        string_view k2 = "7";
        string_view k3 = "A";
    #else
        string_view k1 = "0";
        string_view k2 = "7";
        string_view k3 = "C";
    #endif

        // ensure collisions are distinguishable
        {
            object o(this);
            BOOST_TEST(
                (o.impl_.digest(k1) % buckets) ==
                (o.impl_.digest(k2) % buckets));
            BOOST_TEST(
                (o.impl_.digest(k2) % buckets) ==
                (o.impl_.digest(k3) % buckets));
            o.emplace(k1, 1);
            o.emplace(k2, 2);
            o.emplace(k3, 3);
            BOOST_TEST(number_cast<int>(o.at(k1)) == 1);
            BOOST_TEST(number_cast<int>(o.at(k2)) == 2);
            BOOST_TEST(number_cast<int>(o.at(k3)) == 3);
        }

        // erase k1
        {
            object o(this);
            o.emplace(k1, 1);
            o.emplace(k2, 2);
            o.emplace(k3, 3);
            o.erase(k1);
            BOOST_TEST(number_cast<int>(o.at(k2)) == 2);
            BOOST_TEST(number_cast<int>(o.at(k3)) == 3);
        }

        // erase k2
        {
            object o(this);
            o.emplace(k1, 1);
            o.emplace(k2, 2);
            o.emplace(k3, 3);
            o.erase(k2);
            BOOST_TEST(number_cast<int>(o.at(k1)) == 1);
            BOOST_TEST(number_cast<int>(o.at(k3)) == 3);
        }

        // erase k3
        {
            object o(this);
            o.emplace(k1, 1);
            o.emplace(k2, 2);
            o.emplace(k3, 3);
            o.erase(k3);
            BOOST_TEST(number_cast<int>(o.at(k1)) == 1);
            BOOST_TEST(number_cast<int>(o.at(k2)) == 2);
        }
    }

    void
    testEquality()
    {
        BOOST_TEST(object({}) == object({}));
        BOOST_TEST(object({}) != object({{"1",1},{"2",2}}));
        BOOST_TEST(object({{"1",1},{"2",2},{"3",3}}) == object({{"1",1},{"2",2},{"3",3}}));
        BOOST_TEST(object({{"1",1},{"2",2},{"3",3}}) != object({{"1",1},{"2",2}}));
        BOOST_TEST(object({{"1",1},{"2",2},{"3",3}}) == object({{"3",3},{"2",2},{"1",1}}));
    }

    void
    run()
    {
        testSpecial();
        testIterators();
        testCapacity();
        testModifiers();
        testLookup();
        testHashPolicy();
        testImplementation();
        testCollisions();
        testEquality();
    }
};

TEST_SUITE(object_test, "boost.json.object");

BOOST_JSON_NS_END
