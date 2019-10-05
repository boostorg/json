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

#define BEAST_THROWS( EXPR, EXCEP ) \
    try { \
        EXPR; \
        BEAST_FAIL(); \
    } \
    catch(EXCEP const&) { \
        BEAST_PASS(); \
    } \
    catch(...) { \
        BEAST_FAIL(); \
    }

class object_test : public beast::unit_test::suite
{
public:
    template<class T, class U, class = void>
    struct is_equal_comparable : std::false_type {};
    
    template<class T, class U>
    struct is_equal_comparable<T, U, boost::void_t<decltype(
        std::declval<T const&>() == std::declval<U const&>()
            )>> : std::true_type {};

    template<class T, class U, class = void>
    struct is_unequal_comparable : std::false_type {};
    
    template<class T, class U>
    struct is_unequal_comparable<T, U, boost::void_t<decltype(
        std::declval<T const&>() != std::declval<U const&>()
            )>> : std::true_type {};

    BOOST_STATIC_ASSERT(  std::is_constructible<object::iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_constructible<object::iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_constructible<object::iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(! std::is_constructible<object::local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_constructible<object::local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_constructible<object::local_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(! std::is_constructible<object::const_local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_constructible<object::const_local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_constructible<object::const_local_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_assignable<object::iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_assignable<object::iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_assignable<object::iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(! std::is_assignable<object::local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_assignable<object::local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_assignable<object::local_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(! std::is_assignable<object::const_local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(! std::is_assignable<object::const_local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(  std::is_assignable<object::const_local_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::local_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_equal_comparable<object::const_local_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::local_iterator, object::const_local_iterator>::value);

    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_local_iterator, object::iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_local_iterator, object::const_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_local_iterator, object::local_iterator>::value);
    BOOST_STATIC_ASSERT(is_unequal_comparable<object::const_local_iterator, object::const_local_iterator>::value);

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
        BEAST_EXPECT(o.at("a").as_number() == 1);
        BEAST_EXPECT(o.at("b").as_bool());
        BEAST_EXPECT(o.at("c").as_string() == "hello");

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
            {
                auto it = o.begin();
                BEAST_EXPECT(it->first == "a"); ++it;
                BEAST_EXPECT(it->first == "b"); it++;
                BEAST_EXPECT(it->first == "c"); ++it;
                BEAST_EXPECT(it == o.end());
            }
        }

        // begin() const
        {
            auto it = co.begin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == co.end());
        }

        // cbegin()
        {
            auto it = o.cbegin();
            BEAST_EXPECT(it->first == "a"); ++it;
            BEAST_EXPECT(it->first == "b"); it++;
            BEAST_EXPECT(it->first == "c"); ++it;
            BEAST_EXPECT(it == o.cend());
        }

        // end()
        {
            auto it = o.end();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == o.begin());
        }

        // end() const
        {
            auto it = co.end();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == co.begin());
        }

        // cend()
        {
            auto it = o.cend();
            --it; BEAST_EXPECT(it->first == "c");
            it--; BEAST_EXPECT(it->first == "b");
            --it; BEAST_EXPECT(it->first == "a");
            BEAST_EXPECT(it == o.cbegin());
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

    //--------------------------------------------------------------------------

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

        // insert(before, initializer_list)
        fail_loop([]
        {
            object o;
            o.emplace("c", "hello");
            o.insert(
                o.find("c"),
                {
                    { "a", 1 },
                    { "b", true }
                });
            check(o, 3);
        });

        // insert(node_type&&)
        {
            {
                object o;
                object::node_type nh;
                o.insert(std::move(nh));
            }

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
            }

            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2({
                    {"b", true}});
                auto nh = obj2.extract("b");
                auto const result =
                    obj1.insert(std::move(nh));
                // failed insertion
                BEAST_EXPECT(result.inserted == false);
                BEAST_EXPECT(result.position->first == "b");
                BEAST_EXPECT(! result.node.empty());
            }
        }

        // insert(before, node_type&&)
        {
            {
                object o;
                object::node_type nh;
                o.insert(o.end(), std::move(nh));
            }

            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2;
                obj2.insert(obj1.extract(obj1.begin()));
                obj2.insert(obj1.extract(obj1.find("c")));
                scoped_fail_storage fs;
                auto result =
                    obj2.insert(obj2.find("c"), obj1.extract("b"));
                check(obj2, 3);
                BEAST_EXPECT(obj1.empty());
                BEAST_EXPECT(result.inserted == true);
                BEAST_EXPECT(result.position->first == "b");
                BEAST_EXPECT(result.node.empty());
            }

            {
                object obj1({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                object obj2({
                    {"b", true}});
                auto nh = obj2.extract("b");
                auto const result = obj1.insert(
                    obj2.find("c"), std::move(nh));
                // failed insertion
                BEAST_EXPECT(result.inserted == false);
                BEAST_EXPECT(result.position->first == "b");
                BEAST_EXPECT(! result.node.empty());
            }
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
        {
            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"c", "hello"}});
                scoped_fail_storage fs;
                BEAST_EXPECT(o.erase("b2") == 0);
                check(o, 3);
            }

            {
                object o({
                    {"a", 1},
                    {"b", true},
                    {"b2", 2},
                    {"c", "hello"}});
                scoped_fail_storage fs;
                BEAST_EXPECT(o.erase("b2") == 1);
                check(o, 7);
            }
        }

        // swap(object&)
        {
            {
                object o1 = {{"a",1}, {"b",true}, {"c", "hello"}};
                object o2 = {{"d",{1,2,3}}};
                scoped_fail_storage fs;
                swap(o1, o2);
                BEAST_EXPECT(o1.size() == 1);
                BEAST_EXPECT(o2.size() == 3);
                BEAST_EXPECT(o1.count("d") == 1);
            }

            fail_loop([](storage_ptr const& sp)
            {
                object o1 = {{"a",1}, {"b",true}, {"c", "hello"}};
                object o2({{"d",{1,2,3}}}, sp);
                swap(o1, o2);
                BEAST_EXPECT(o1.size() == 1);
                BEAST_EXPECT(o2.size() == 3);
                BEAST_EXPECT(o1.count("d") == 1);
            });
        }

        // extract(const_iterator)
        {
            object o = {
                {"a",1},
                {"b",true},
                {"c", "hello"}};
            auto nh = o.extract(o.find("b"));
            BEAST_EXPECT(! nh.empty());
            BEAST_EXPECT(nh.key() == "b");
            BEAST_EXPECT(nh.mapped().is_bool());
            BEAST_EXPECT(nh.mapped().as_bool());
        }

        // extract(key)
        {
            object o = {
                {"a",1},
                {"b",true},
                {"c", "hello"}};

            {
                auto nh = o.extract("d");
                BEAST_EXPECT(nh.empty());
            }
            {
                auto nh = o.extract("b");
                BEAST_EXPECT(! nh.empty());
                BEAST_EXPECT(nh.key() == "b");
                BEAST_EXPECT(nh.mapped().is_bool());
                BEAST_EXPECT(nh.mapped().as_bool());
            }
            {
                auto nh = o.extract("b");
                BEAST_EXPECT(nh.empty());
            }
        }
    }

    //--------------------------------------------------------------------------

    void
    testLookup()
    {
        object o1({
            {"a", 1},
            {"b", true},
            {"c", "hello"}});
        auto const& co1 = o1;
        object o2;
        auto const& co2 = o2;

        // at(key)
        {
            BEAST_EXPECT(
                o1.at("a").is_number());
            BEAST_THROWS((o2.at("a")),
                std::out_of_range);
        }

        // at(key) const
        {
            BEAST_EXPECT(
                co1.at("a").is_number());
            BEAST_THROWS((co2.at("a")),
                std::out_of_range);
        }

        // operator[](key)
        {
            BEAST_EXPECT(o1.count("d") == 0);;
            BEAST_EXPECT(o1["a"].is_number());
            BEAST_EXPECT(o1["d"].is_null());
            BEAST_EXPECT(o1.count("d") == 1);
        }

        // count(key)
        {
            BEAST_EXPECT(o1.count("a") == 1);
            BEAST_EXPECT(o1.count("d") == 1);
            BEAST_EXPECT(o1.count("e") == 0);
            BEAST_EXPECT(o2.count("a") == 0);
        }

        // find(key)
        {
            BEAST_EXPECT(
                o1.find("a")->first == "a");
            BEAST_EXPECT(
                o1.find("e") == o1.end());
            BEAST_EXPECT(
                o2.find("e") == o2.end());
        }

        // contains(key)
        {
            BEAST_EXPECT(o1.contains("a"));
            BEAST_EXPECT(o1.contains("d"));
            BEAST_EXPECT(! o1.contains("e"));
            BEAST_EXPECT(! o2.contains("a"));
            BEAST_EXPECT(! o2.contains("d"));
        }
    }

    void
    testBuckets()
    {
        object o;
        auto const& co = o;
        o.max_load_factor(1000);
        for(std::size_t i = 0;; ++i)
        {
            o.emplace(std::to_string(i), i);
            if(o.bucket_size(0) >= 3)
                break;
        }

        // begin(size_type)
        // end(size_type)
        {
            auto it = o.begin(0);
            BEAST_EXPECT(it != o.end());
            BEAST_EXPECT(! (*it).first.empty());
            BEAST_EXPECT(it->second.is_number()); ++it;
            BEAST_EXPECT(it->second.is_number()); it++;
            BEAST_EXPECT(it->second.is_number()); ++it;
            BEAST_EXPECT(it != o.begin(0));
            BEAST_EXPECT(it == o.end(0));
            BEAST_EXPECT(it == o.end());
        }

        // begin(size_type) const
        // end(size_type) const
        {
            auto it = co.begin(0);
            BEAST_EXPECT(it != co.end());
            BEAST_EXPECT(! (*it).first.empty());
            BEAST_EXPECT(it->second.is_number()); ++it;
            BEAST_EXPECT(it->second.is_number()); it++;
            BEAST_EXPECT(it->second.is_number()); ++it;
            BEAST_EXPECT(it != co.begin(0));
            BEAST_EXPECT(it == co.end(0));
            BEAST_EXPECT(it == co.end());
        }

        // cbegin(size_type)
        // cend(size_type)
        {
            auto it = o.cbegin(0);
            BEAST_EXPECT(it != co.cend());
            BEAST_EXPECT(! (*it).first.empty());
            BEAST_EXPECT(it->second.is_number()); ++it;
            BEAST_EXPECT(it->second.is_number()); it++;
            BEAST_EXPECT(it->second.is_number()); ++it;
            BEAST_EXPECT(it != co.cbegin(0));
            BEAST_EXPECT(it == co.cend(0));
            BEAST_EXPECT(it == co.cend());
        }

        // bucket_count()
        // bucket_size()
        {
            std::size_t n = 0;
            for(std::size_t i = 0;
                i < o.bucket_count(); ++i)
                n += o.bucket_size(i);
            BEAST_EXPECT(n == o.size());
        }

        // max_bucket_count()
        {
            BEAST_EXPECT(
                o.max_bucket_count() >=
                o.bucket_count());
        }

        // bucket(key_type)
        {
            for(auto it = o.begin(0);
                    it != o.end(0); ++it)
                BEAST_EXPECT(
                    o.bucket(it->first) == 0);
        }

        // iterator(local_iterator)
        {
            object::iterator it(o.begin(0));
        }

        // const_local_iterator(local_iterator)
        {
            object::const_local_iterator it(o.begin(0));
        }
    }

    void
    testHashPolicy()
    {
        object o;
        for(std::size_t i = 0; i < 1000; ++i)
            o.emplace(std::to_string(i), i);

        // load_factor()
        {
            BEAST_EXPECT(
                o.load_factor() <=
                o.max_load_factor());
        }

        // max_load_factor()
        {
            BEAST_EXPECT(
                o.max_load_factor() == 1);
        }

        // max_load_factor(float)
        {
            auto const lf =
                o.load_factor();
            o.max_load_factor(lf/2);
            BEAST_EXPECT(
                o.load_factor() < lf);
        }

        // rehash(size_type)
        {
            o.clear();
            o.rehash(100);
            BEAST_EXPECT(
                o.bucket_count() >= 100);
        }

        // reserve(size_type)
        {
            o.reserve(200);
            BEAST_EXPECT(o.bucket_count() >=
                std::ceil(200 / o.max_load_factor()));
        }
    }

    void
    testObservers()
    {
        // hash_function()
        {
            object o;
            object::hasher h = o.hash_function();
            boost::ignore_unused(h);
        }

        // key_eq()
        {
            object o;
            object::key_equal eq = o.key_eq();
            BEAST_EXPECT(eq("a", "a"));
        }
    }

    //--------------------------------------------------------------------------

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

                object::node_type nh1(o.extract("b"));
                object::node_type nh2(std::move(nh1));
                BEAST_EXPECT(nh1.empty());
                BEAST_EXPECT(nh1.get_storage() == nullptr);
                BEAST_EXPECT(nh2.key() == "b");
                BEAST_EXPECT(
                    nh2.get_storage() == o.get_storage());
            }
        }

        // operator=(node_type&&)
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});

            object::node_type nh1(o.extract("b"));
            object::node_type nh2;
            nh2 = std::move(nh1);
            BEAST_EXPECT(nh1.empty());
            BEAST_EXPECT(nh1.get_storage() == nullptr);
            BEAST_EXPECT(nh2.key() == "b");
            BEAST_EXPECT(
                nh2.get_storage() == o.get_storage());
        }

        // empty
        {
            object::node_type nh;
            BEAST_EXPECT(nh.empty());
        }

        // operator bool
        {
            object::node_type nh;
            BEAST_EXPECT(! nh);
        }

        // get_storage()
        {
            object::node_type nh;
            BEAST_EXPECT(
                nh.get_storage() == nullptr);
        }

        // key()
        // mapped()
        // mapped() const
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});

            object::node_type nh(
                o.extract("b"));
            auto const& cnh = nh;
            BEAST_EXPECT(nh.key() == "b");
            BEAST_EXPECT(nh.mapped().is_bool());
            BEAST_EXPECT(cnh.mapped().is_bool());
        }

        // swap(node_type&)
        {
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            object o2({
                {"d", {1,2,3}}});
            check(o1, 3);
            auto nh1 = o1.extract("b");
            auto nh2 = o2.extract("d");
            BEAST_EXPECT(nh1.key() == "b");
            nh1.swap(nh2);
            BEAST_EXPECT(nh2.key() == "b");
            BEAST_EXPECT(nh1.key() == "d");
        }

        // ::swap(object&, object&)
        {
            object o1({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            object o2({
                {"d", {1,2,3}}});
            check(o1, 3);
            auto nh1 = o1.extract("b");
            auto nh2 = o2.extract("d");
            BEAST_EXPECT(nh1.key() == "b");
            swap(nh1, nh2);
            BEAST_EXPECT(nh2.key() == "b");
            BEAST_EXPECT(nh1.key() == "d");
        }
    }

    void
    testImplementation()
    {
        // insert duplicate keys
        {
            object o({
                {"a", 1},
                {"b", true},
                {"b", true},
                {"c", "hello"}});
            check(o, 3);
        }

        // insert, not at end, causing a rehash
        {
            object o({
                {"a", 1},
                {"b", true},
                {"c", "hello"}});
            BEAST_EXPECT(o.bucket_count() == 3);
            o.insert(o.begin(), {"d", {1,2,3}});
            BEAST_EXPECT(o.bucket_count() > 3);
        }

        // insert before first element of non-empty container
        {
            object o({
                {"b", true},
                {"c", "hello"}});
            o.insert(o.begin(), {"a", 1});
            check(o, 3);
        }

        // remove element in the middle of a bucket
        {
            object o;
            o.max_load_factor(1000);
            for(std::size_t i = 0;; ++i)
            {
                o.emplace(std::to_string(i), i);
                if(o.bucket_size(0) >= 3)
                    break;
            }

            auto bn = o.bucket_size(0);
            auto it = o.begin(0);
            ++it;
            ++it;
            o.erase(it);
            BEAST_EXPECT(
                o.bucket_size(0) < bn);
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
        testImplementation();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,object);

} // json
} // boost
