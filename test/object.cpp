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
#include <boost/json/detail/make_void.hpp>
#include <cmath>
#include <type_traits>

#include "test.hpp"

namespace boost {
namespace json {

class object_test : public beast::unit_test::suite
{
public:
    string_view const str_;

    object_test()
        : str_(
            "abcdefghijklmnopqrstuvwxyz")
    {
        // ensure this string does
        // not fit in the SBO area.
        BOOST_JSON_ASSERT(str_.size() >
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

    BOOST_JSON_STATIC_ASSERT(  std::is_constructible<object::iterator, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(! std::is_constructible<object::iterator, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(  std::is_assignable<object::iterator&, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(! std::is_assignable<object::iterator&, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(  std::is_assignable<object::const_iterator&, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(  std::is_assignable<object::const_iterator&, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::iterator, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::iterator, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::const_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(  std::is_constructible<object::reverse_iterator, object::reverse_iterator>::value);
    // std::reverse_iterator ctor is not SFINAEd
    //BOOST_JSON_STATIC_ASSERT(! std::is_constructible<object::reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(  std::is_constructible<object::const_reverse_iterator, object::reverse_iterator>::value);
    BOOST_JSON_STATIC_ASSERT(  std::is_constructible<object::const_reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(  std::is_assignable<object::reverse_iterator&, object::reverse_iterator>::value);
    // std::reverse_iterator assignment is not SFINAEd
    //BOOST_JSON_STATIC_ASSERT(! std::is_assignable<object::reverse_iterator&, object::const_reverse_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(  std::is_assignable<object::const_reverse_iterator&, object::reverse_iterator>::value);
    BOOST_JSON_STATIC_ASSERT(  std::is_assignable<object::const_reverse_iterator&, object::const_reverse_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::reverse_iterator, object::reverse_iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::const_reverse_iterator, object::reverse_iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_equal_comparable<object::const_reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::reverse_iterator, object::reverse_iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::reverse_iterator, object::const_reverse_iterator>::value);

    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::const_reverse_iterator, object::reverse_iterator>::value);
    BOOST_JSON_STATIC_ASSERT(is_unequal_comparable<object::const_reverse_iterator, object::const_reverse_iterator>::value);
    
    static
    void
    check(
        object const& o,
        std::size_t capacity)
    {
        BEAST_EXPECT(! o.empty());
        BEAST_EXPECT(o.size() == 3);
        BEAST_EXPECT(
            o.capacity() == capacity);
        BEAST_EXPECT(o.at("a").as_int64() == 1);
        BEAST_EXPECT(o.at("b").as_bool());
        BEAST_EXPECT(o.at("c").as_string() == "hello");
        check_storage(o, o.get_storage());
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
            BEAST_EXPECT(o.empty());
            BEAST_EXPECT(o.size() == 0);
            BEAST_EXPECT(o.capacity() == 0);
        }

        // object(storage_ptr)
        fail_loop([&](storage_ptr const& sp)
        {
            object o(sp);
            check_storage(o, sp);
            BEAST_EXPECT(o.empty());
            BEAST_EXPECT(o.size() == 0);
            BEAST_EXPECT(o.capacity() == 0);
        });

        // object(std::size_t, storage_ptr)
        {
            {
                object o(10);
                BEAST_EXPECT(o.empty());
                BEAST_EXPECT(o.size() == 0);
                BEAST_EXPECT(o.capacity() >= 10);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o(10, sp);
                check_storage(o, sp);
                BEAST_EXPECT(o.empty());
                BEAST_EXPECT(o.size() == 0);
                BEAST_EXPECT(o.capacity() >= 10);
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
                BEAST_EXPECT(! o.empty());
                BEAST_EXPECT(o.size() == 3);
                BEAST_EXPECT(o.capacity() == 7);
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
                BEAST_EXPECT(! o.empty());
                BEAST_EXPECT(o.size() == 3);
                BEAST_EXPECT(o.capacity() == 7);
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
            BEAST_EXPECT(o1.empty());
            BEAST_EXPECT(o1.size() == 0);
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
            BEAST_EXPECT(! o1.empty());
            check(o2, 3);
            check_storage(o1,
                storage_ptr{});
            check_storage(o2, sp);
        });

        // object(pilfered<object>)
        {
            auto const sp =
                make_storage<unique_storage>();
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                }, sp);
            object o2(pilfer(o1));
            BEAST_EXPECT(
                o1.get_storage() == nullptr);
            BEAST_EXPECT(
                *o2.get_storage() == *sp);
            BEAST_EXPECT(o1.empty());
            check(o2, 3);
        }

        auto const sp = make_storage<unique_storage>();
        auto const sp0 = storage_ptr{};

        // object(object const&)
        {
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}
                });
            object o2(o1);
            BEAST_EXPECT(! o1.empty());
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
            BEAST_EXPECT(! o1.empty());
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
                BEAST_EXPECT(
                    *o.get_storage() == *sp);
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
                BEAST_EXPECT(o1.empty());
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
        auto const& co = o;
        object no;
        auto const& cno = no;

        // empty container
        {
            BEAST_EXPECT(no.begin() == no.end());
            BEAST_EXPECT(cno.begin() == cno.end());
            BEAST_EXPECT(no.cbegin() == no.cend());
        }

        // begin()
        {
            auto it = o.begin();
            BEAST_EXPECT(it->key() == "a"); ++it;
            BEAST_EXPECT(it->key() == "b"); it++;
            BEAST_EXPECT(it->key() == "c"); ++it;
            BEAST_EXPECT(it == o.end());
        }

        // begin() const
        {
            auto it = co.begin();
            BEAST_EXPECT(it->key() == "a"); ++it;
            BEAST_EXPECT(it->key() == "b"); it++;
            BEAST_EXPECT(it->key() == "c"); ++it;
            BEAST_EXPECT(it == co.end());
        }

        // cbegin()
        {
            auto it = o.cbegin();
            BEAST_EXPECT(it->key() == "a"); ++it;
            BEAST_EXPECT(it->key() == "b"); it++;
            BEAST_EXPECT(it->key() == "c"); ++it;
            BEAST_EXPECT(it == o.cend());
        }

        // end()
        {
            auto it = o.end();
            --it; BEAST_EXPECT(it->key() == "c");
            it--; BEAST_EXPECT(it->key() == "b");
            --it; BEAST_EXPECT(it->key() == "a");
            BEAST_EXPECT(it == o.begin());
        }

        // end() const
        {
            auto it = co.end();
            --it; BEAST_EXPECT(it->key() == "c");
            it--; BEAST_EXPECT(it->key() == "b");
            --it; BEAST_EXPECT(it->key() == "a");
            BEAST_EXPECT(it == co.begin());
        }

        // cend()
        {
            auto it = o.cend();
            --it; BEAST_EXPECT(it->key() == "c");
            it--; BEAST_EXPECT(it->key() == "b");
            --it; BEAST_EXPECT(it->key() == "a");
            BEAST_EXPECT(it == o.cbegin());
        }

        // rbegin()
        {
            auto it = o.rbegin();
            BEAST_EXPECT(it->key() == "c"); ++it;
            BEAST_EXPECT(it->key() == "b"); it++;
            BEAST_EXPECT(it->key() == "a"); ++it;
            BEAST_EXPECT(it == o.rend());
        }

        // rbegin() const
        {
            auto it = co.rbegin();
            BEAST_EXPECT(it->key() == "c"); ++it;
            BEAST_EXPECT(it->key() == "b"); it++;
            BEAST_EXPECT(it->key() == "a"); ++it;
            BEAST_EXPECT(it == co.rend());
        }

        // crbegin()
        {
            auto it = o.crbegin();
            BEAST_EXPECT(it->key() == "c"); ++it;
            BEAST_EXPECT(it->key() == "b"); it++;
            BEAST_EXPECT(it->key() == "a"); ++it;
            BEAST_EXPECT(it == o.crend());
        }

        // rend()
        {
            auto it = o.rend();
            --it; BEAST_EXPECT(it->key() == "a");
            it--; BEAST_EXPECT(it->key() == "b");
            --it; BEAST_EXPECT(it->key() == "c");
            BEAST_EXPECT(it == o.rbegin());
        }

        // rend() const
        {
            auto it = co.rend();
            --it; BEAST_EXPECT(it->key() == "a");
            it--; BEAST_EXPECT(it->key() == "b");
            --it; BEAST_EXPECT(it->key() == "c");
            BEAST_EXPECT(it == co.rbegin());
        }

        // crend()
        {
            auto it = o.crend();
            --it; BEAST_EXPECT(it->key() == "a");
            it--; BEAST_EXPECT(it->key() == "b");
            --it; BEAST_EXPECT(it->key() == "c");
            BEAST_EXPECT(it == o.crbegin());
        }
    }

    //------------------------------------------------------

    void
    testCapacity()
    {
        BEAST_EXPECT(
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
                BEAST_EXPECT(o.empty());
            }

            {
                object o;
                o.emplace("x", 1);
                BEAST_EXPECT(! o.empty());
                o.clear();
                BEAST_EXPECT(o.empty());
            }
        }

        // insert(P&&)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                object o(sp);
                auto result = o.insert(
                    std::make_pair("x", 1));
                BEAST_EXPECT(result.second);
                BEAST_EXPECT(result.first->key() == "x");
                BEAST_EXPECT(result.first->value().as_int64() == 1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o(sp);
                auto const p = std::make_pair("x", 1);
                auto result = o.insert(p);
                BEAST_EXPECT(result.second);
                BEAST_EXPECT(result.first->key() == "x");
                BEAST_EXPECT(result.first->value().as_int64() == 1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o({
                    {"a", 1},
                    {"b", 2},
                    {"c", 3}}, sp);
                auto const result = o.insert(
                    std::make_pair("b", 4));
                BEAST_EXPECT(
                    result.first->value().as_int64() == 2);
                BEAST_EXPECT(! result.second);
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
                std::string const big(
                    string().capacity() + 1, '*');
                string_view const sv(big);
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
                BEAST_EXPECT(o["a"].is_string());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o({
                    {"a", 1},
                    {"b", 2},
                    {"c", 3}}, sp);
                o.insert_or_assign("d", str_);
                BEAST_EXPECT(o["d"].is_string());
                BEAST_EXPECT(o.size() == 4);
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
                BEAST_EXPECT(
                    ! o.insert_or_assign("a", 2).second);
                BEAST_EXPECT(o["a"].as_int64() == 2);
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
                BEAST_EXPECT(it->key() == "c");
                BEAST_EXPECT(
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
                BEAST_EXPECT(o.erase("b2") == 0);
                check(o, 3);
            }

            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"b2", 2},
                    {"c", "hello"}});
                BEAST_EXPECT(o.erase("b2") == 1);
                check(o, 7);
            }
        }

        // swap(object&)
        {
            {
                object o1 = {{"a",1}, {"b",true}, {"c", "hello"}};
                object o2 = {{"d",{1,2,3}}};
                swap(o1, o2);
                BEAST_EXPECT(o1.size() == 1);
                BEAST_EXPECT(o2.size() == 3);
                BEAST_EXPECT(o1.count("d") == 1);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                object o1 = {{"a",1}, {"b",true}, {"c", "hello"}};
                object o2({{"d",{1,2,3}}}, sp);
                swap(o1, o2);
                BEAST_EXPECT(o1.size() == 1);
                BEAST_EXPECT(o2.size() == 3);
                BEAST_EXPECT(o1.count("d") == 1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                object o1({{"d",{1,2,3}}}, sp);
                object o2 = {{"a",1}, {"b",true}, {"c", "hello"}};
                swap(o1, o2);
                BEAST_EXPECT(o1.size() == 3);
                BEAST_EXPECT(o2.size() == 1);
                BEAST_EXPECT(o2.count("d") == 1);
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
            BEAST_EXPECT(
                o1.at("a").is_number());
            BEAST_THROWS((o1.at("d")),
                std::out_of_range);
        }

        // at(key) const
        {
            BEAST_EXPECT(
                co1.at("a").is_number());
            BEAST_THROWS((co1.at("d")),
                std::out_of_range);
        }

        // operator[&](key)
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(o.count("d") == 0);;
            BEAST_EXPECT(o["a"].is_number());
            BEAST_EXPECT(o["d"].is_null());
            BEAST_EXPECT(o.count("d") == 1);
        }

        // count(key)
        {
            BEAST_EXPECT(o1.count("a") == 1);
            BEAST_EXPECT(o1.count("d") == 0);
            BEAST_EXPECT(o1.count("e") == 0);
        }

        // find(key)
        {
            BEAST_EXPECT(
                o1.find("a")->key() == "a");
            BEAST_EXPECT(
                o1.find("e") == o1.end());
        }

        // contains(key)
        {
            BEAST_EXPECT(o1.contains("a"));
            BEAST_EXPECT(! o1.contains("e"));
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
                BEAST_EXPECT(o.capacity() >= 15);
                o.reserve(20);
                BEAST_EXPECT(o.capacity() >= 20);
            }

            {
                object o;
                o.reserve(3);
                BEAST_EXPECT(o.capacity() == 3);
                o.reserve(7);
                BEAST_EXPECT(o.capacity() == 7);
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
            BEAST_EXPECT(o.at("a").as_int64() == 1);
            BEAST_EXPECT(o.at("b").as_bool());
            BEAST_EXPECT(o.at("c").as_string() == "hello");
        }

        // find in missing or empty tables
        {
            object o;
            BEAST_EXPECT(o.find("a") == o.end());
            o.reserve(3);
            BEAST_EXPECT(o.capacity() > 0);
            BEAST_EXPECT(o.find("a") == o.end());
        }

    }

    void
    run() override
    {
        testSpecial();
        testIterators();
        testCapacity();
        testModifiers();
        testLookup();
        testHashPolicy();
        testImplementation();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,object);

} // json
} // boost
