//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/array.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

#include "test.hpp"

namespace boost {
namespace json {

class array_test : public beast::unit_test::suite
{
public:
    using init_list = std::initializer_list<value>;

    string_view const str_;
    std::size_t min_capacity_;

    array_test()
        : str_(
            "abcdefghijklmnopqrstuvwxyz")
    {
        // ensure this string does
        // not fit in the SBO area.
        BOOST_JSON_ASSERT(str_.size() >
            string().capacity());

        // calculate minimum array capacity
        array a;
        a.resize(1);
        min_capacity_ = a.capacity();
    }

    void
    check(array const& a)
    {
        BEAST_EXPECT(a.size() == 3);
        BEAST_EXPECT(a[0].is_number());
        BEAST_EXPECT(a[1].is_bool());
        BEAST_EXPECT(a[2].is_string());
    }

    void
    check(
        array const& a,
        storage_ptr const& sp)
    {
        check(a);
        check_storage(a, sp);
    }

    void
    testCtors()
    {
        // ~array()
        {
            // implied
        }

        // array()
        {
            array a;
            BEAST_EXPECT(a.empty());
            BEAST_EXPECT(a.size() == 0);
        }

        // array(storage_ptr)
        {
            array a(storage_ptr{});
            check_storage(a, storage_ptr{});
        }

        // array(size_type, value, storage)
        {
            // default storage
            {
                array a(3, true);
                BEAST_EXPECT(a.size() == 3);
                for(auto const& v : a)
                    BEAST_EXPECT(v.is_bool());
                check_storage(a, storage_ptr{});
            }

            // construct with zero `true` values
            {
                array(0, true);
            }

            // construct with three `true` values
            fail_loop([&](storage_ptr const& sp)
            {
                array a(3, true, sp);
                BEAST_EXPECT(a.size() == 3);
                check_storage(a, sp);
            });
        }

        // array(size_type, storage)
        {
            // default storage
            {
                array a(3);
                BEAST_EXPECT(a.size() == 3);
                for(auto const& v : a)
                    BEAST_EXPECT(v.is_null());
                check_storage(a, storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                array a(3, sp);
                BEAST_EXPECT(a.size() == 3);
                for(auto const& v : a)
                    BEAST_EXPECT(v.is_null());
                check_storage(a, sp);
            });
        }

        // array(InputIt, InputIt, storage)
        {
            // default storage
            {
                init_list init{ 0, 1, str_, 3, 4 };
                array a(init.begin(), init.end());
                check_storage(a, storage_ptr{});
                BEAST_EXPECT(a[0].as_int64() == 0);
                BEAST_EXPECT(a[1].as_int64() == 1);
                BEAST_EXPECT(a[2].as_string() == str_);
                BEAST_EXPECT(a[3].as_int64() == 3);
                BEAST_EXPECT(a[4].as_int64() == 4);
            }

            // random iterator
            fail_loop([&](storage_ptr const& sp)
            {
                init_list init{ 1, true, str_ };
                array a(init.begin(), init.end(), sp);
                check(a);
                check_storage(a, sp);
            });

            // input iterator
            fail_loop([&](storage_ptr const& sp)
            {
                init_list init{ 1, true, str_ };
                array a(
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end()), sp);
                check(a);
                check_storage(a, sp);
            });
        }

        // array(array const&)
        {
            {
                array a1;
                array a2(a1);
            }

            {
                array a1;
                array a2({ 1, true, str_ });
                a2 = a1;
            }

            {
                init_list init{ 1, true, str_ };
                array a1(init.begin(), init.end());
                array a2(a1);
                check(a2);
                check_storage(a2, storage_ptr{});
            }
        }

        // array(array const&, storage)
        fail_loop([&](storage_ptr const& sp)
        {
            init_list init{ 1, true, str_ };
            array a1(init.begin(), init.end());
            array a2(a1, sp);
            check(a2);
            check_storage(a2, sp);
        });

        // array(pilfered<array>)
        {
            init_list init{ 1, true, str_ };
            array a1(init.begin(), init.end());
            array a2(pilfer(a1));
            BEAST_EXPECT(a1.empty());
            check(a2);
            check_storage(a2, storage_ptr{});
        }

        // array(array&&)
        {
            init_list init{ 1, true, str_ };
            array a1(init.begin(), init.end());
            array a2 = std::move(a1);
            BEAST_EXPECT(a1.empty());
            check(a2);
            check_storage(a2, storage_ptr{});
        }

        // array(array&&, storage)
        {
            {
                init_list init{ 1, true, str_ };
                array a1(init.begin(), init.end());
                array a2(
                    std::move(a1), storage_ptr{});
                BEAST_EXPECT(a1.empty());
                check(a2);
                check_storage(a1, storage_ptr{});
                check_storage(a2, storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                init_list init{ 1, true, str_ };
                array a1(init.begin(), init.end());
                array a2(std::move(a1), sp);
                BEAST_EXPECT(! a1.empty());
                check(a2);
                check_storage(a1, storage_ptr{});
                check_storage(a2, sp);
            });
        }

        // array(init_list, storage)
        {
            // default storage
            {
                array a({1, true, str_});
                check(a);
                check_storage(a, storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                array a({1, true, str_}, sp);
                check(a, sp);
                check_storage(a, sp);
            });
        }
    }

    void
    testAssignment()
    {
        // operator=(array const&)
        {
            {
                array a1({1, true, str_});
                array a2({nullptr, object{}, 1.f});
                a2 = a1;
                check(a1);
                check(a2);
                check_storage(a1, storage_ptr{});
                check_storage(a2, storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                array a1({1, true, str_});
                array a2({nullptr, object{}, 1.f}, sp);
                a2 = a1;
                check(a1);
                check(a2);
                check_storage(a1, storage_ptr{});
                check_storage(a2, sp);
            });
        }

        // operator=(array&&)
        {
            {
                array a1({1, true, str_});
                array a2({nullptr, object{}, 1.f});
                a2 = std::move(a1);
                BEAST_EXPECT(a1.empty());
                check(a2);
            }

            // empty
            {
                array a1;
                array a2;
                a2 = std::move(a1);
                BEAST_EXPECT(a1.empty());
                BEAST_EXPECT(a2.empty());
            }

            fail_loop([&](storage_ptr const& sp)
            {
                array a1({1, true, str_});
                array a2({nullptr, object{}, 1.f}, sp);
                a2 = std::move(a1);
                check(a1);
                check(a2);
                check_storage(a1, storage_ptr{});
                check_storage(a2, sp);
            });
        }

        // operator=(init_list)
        {
            {
                array a;
                a = {};
            }

            {
                array a({ 1, true, str_ });
                a = {};
            }

            {
                init_list init{ 1, true, str_ };
                array a({nullptr, object{}, 1.f});
                a = init;
                check(a);
                check_storage(a, storage_ptr{});
            }

            fail_loop([&](storage_ptr const& sp)
            {
                init_list init{ 1, true, str_ };
                array a({nullptr, object{}, 1.f}, sp);
                a = init;
                check(a);
                check_storage(a, sp);
            });
        }
    }

    void
    testGetStorage()
    {
        // get_storage()
        {
            // implied
        }
    }

    void
    testAccess()
    {
        // at(pos)
        {
            array a({1, true, str_});
            BEAST_EXPECT(a.at(0).is_number());
            BEAST_EXPECT(a.at(1).is_bool());
            BEAST_EXPECT(a.at(2).is_string());
            try
            {
                a.at(3);
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // at(pos) const
        {
            array const a({1, true, str_});
            BEAST_EXPECT(a.at(0).is_number());
            BEAST_EXPECT(a.at(1).is_bool());
            BEAST_EXPECT(a.at(2).is_string());
            try
            {
                a.at(3);
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // operator[&](size_type)
        {
            array a({1, true, str_});
            BEAST_EXPECT(a[0].is_number());
            BEAST_EXPECT(a[1].is_bool());
            BEAST_EXPECT(a[2].is_string());
        }

        // operator[&](size_type) const
        {
            array const a({1, true, str_});
            BEAST_EXPECT(a[0].is_number());
            BEAST_EXPECT(a[1].is_bool());
            BEAST_EXPECT(a[2].is_string());
        }

        // front()
        {
            array a({1, true, str_});
            BEAST_EXPECT(a.front().is_number());
        }

        // front() const
        {
            array const a({1, true, str_});
            BEAST_EXPECT(a.front().is_number());
        }

        // back()
        {
            array a({1, true, str_});
            BEAST_EXPECT(a.back().is_string());
        }

        // back() const
        {
            array const a({1, true, str_});
            BEAST_EXPECT(a.back().is_string());
        }

        // data()
        {
            {
                array a({1, true, str_});
                BEAST_EXPECT(a.data() == &a[0]);
            }
            {
                BEAST_EXPECT(array{}.data() == nullptr);
            }
        }

        // data() const
        {
            {
                array const a({1, true, str_});
                BEAST_EXPECT(a.data() == &a[0]);
            }
            {
                array const a{};
                BEAST_EXPECT(a.data() == nullptr);
            }
        }
    }

    void
    testIterators()
    {
        array a({1, true, str_});
        auto const& ac(a);

        {
            auto it = a.begin();
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it == a.end());
        }
        {
            auto it = a.cbegin();
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it == a.cend());
        }
        {
            auto it = ac.begin();
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it == ac.end());
        }
        {
            auto it = a.end();
            --it; BEAST_EXPECT(it->is_string());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_number());
            BEAST_EXPECT(it == a.begin());
        }
        {
            auto it = a.cend();
            --it; BEAST_EXPECT(it->is_string());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_number());
            BEAST_EXPECT(it == a.cbegin());
        }
        {
            auto it = ac.end();
            --it; BEAST_EXPECT(it->is_string());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_number());
            BEAST_EXPECT(it == ac.begin());
        }

        {
            auto it = a.rbegin();
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it == a.rend());
        }
        {
            auto it = a.crbegin();
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it == a.crend());
        }
        {
            auto it = ac.rbegin();
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it == ac.rend());
        }
        {
            auto it = a.rend();
            --it; BEAST_EXPECT(it->is_number());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_string());
            BEAST_EXPECT(it == a.rbegin());
        }
        {
            auto it = a.crend();
            --it; BEAST_EXPECT(it->is_number());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_string());
            BEAST_EXPECT(it == a.crbegin());
        }
        {
            auto it = ac.rend();
            --it; BEAST_EXPECT(it->is_number());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_string());
            BEAST_EXPECT(it == ac.rbegin());
        }

        {
            array a2;
            array const& ca2(a2);
            BEAST_EXPECT(std::distance(
                a2.begin(), a2.end()) == 0);
            BEAST_EXPECT(std::distance(
                ca2.begin(), ca2.end()) == 0);
            BEAST_EXPECT(std::distance(
                ca2.cbegin(), ca2.cend()) == 0);
            BEAST_EXPECT(std::distance(
                a2.rbegin(), a2.rend()) == 0);
            BEAST_EXPECT(std::distance(
                ca2.rbegin(), ca2.rend()) == 0);
            BEAST_EXPECT(std::distance(
                ca2.crbegin(), ca2.crend()) == 0);
        }
    }

    void
    testCapacity()
    {
        // empty()
        {
            {
                array a;
                BEAST_EXPECT(a.empty());
                a.emplace_back(1);
                BEAST_EXPECT(! a.empty());
            }

            {
                array a({1, 2});
                BEAST_EXPECT(! a.empty());
                a.clear();
                BEAST_EXPECT(a.empty());
                BEAST_EXPECT(a.capacity() > 0);
            }
        }

        // size()
        {
            array a;
            BEAST_EXPECT(a.size() == 0);
            a.emplace_back(1);
            BEAST_EXPECT(a.size() == 1);
        }

        // max_size()
        {
            array a;
            BEAST_EXPECT(a.max_size() > 0);
        }

        // reserve()
        {
            {
                array a;
                a.reserve(0);
            }

            {
                array a(3);
                a.reserve(1);
            }

            {
                array a(3);
                a.reserve(0);
            }

            {
                array a;
                a.reserve(50);
                BEAST_EXPECT(a.capacity() >= 50);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                array a(min_capacity_, 'c', sp);
                a.reserve(a.capacity() + 1);
                auto const new_cap = a.capacity();
                BEAST_EXPECT(new_cap > min_capacity_);
                a.reserve((min_capacity_ + new_cap) / 2);
                BEAST_EXPECT(a.capacity() == new_cap);
            });
        }

        // capacity()
        {
            array a;
            BEAST_EXPECT(a.capacity() == 0);
        }

        // shrink_to_fit()
        {
            {
                array a(1);
                a.shrink_to_fit();
                BEAST_EXPECT(a.size() == 1);
                BEAST_EXPECT(a.capacity() >= 1);
            }

            {
                array a(min_capacity_, 'c');
                BEAST_EXPECT(a.capacity() >= min_capacity_);
                a.erase(a.begin(), a.begin() + 2);
                a.shrink_to_fit();
                BEAST_EXPECT(a.capacity() == min_capacity_);
            }

            fail_loop([&](storage_ptr const& sp)
            {
                array a(1, sp);
                a.resize(a.capacity());
                a.shrink_to_fit();
                BEAST_EXPECT(a.size() == a.capacity());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                array a(sp);
                a.reserve(10);
                BEAST_EXPECT(a.capacity() >= 10);
                a.shrink_to_fit();
                BEAST_EXPECT(a.capacity() == 0);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                array a(min_capacity_, sp);
                a.reserve(min_capacity_ * 2);
                BEAST_EXPECT(a.capacity() >=
                    min_capacity_ * 2);
                a.shrink_to_fit();
                if(a.capacity() > min_capacity_)
                    throw test_failure{};
            });
        }
    }

    void
    testModifiers()
    {
        // clear
        {
            {
                array a;
                BEAST_EXPECT(a.size() == 0);
                BEAST_EXPECT(a.capacity() == 0);
                a.clear();
                BEAST_EXPECT(a.size() == 0);
                BEAST_EXPECT(a.capacity() == 0);
            }
            {
                array a({1, true, str_});
                a.clear();
                BEAST_EXPECT(a.size() == 0);
                BEAST_EXPECT(a.capacity() > 0);
            }
        }

        // insert(const_iterator, value_type const&)
        fail_loop([&](storage_ptr const& sp)
        {
            array a({1, str_}, sp);
            value v(true);
            a.insert(a.begin() + 1, v);
            check(a);
            check_storage(a, sp);
        });

        // insert(const_iterator, value_type&&)
        fail_loop([&](storage_ptr const& sp)
        {
            array a({1, str_}, sp);
            value v(true);
            a.insert(
                a.begin() + 1, std::move(v));
            check(a);
            check_storage(a, sp);
        });

        // insert(const_iterator, size_type, value_type const&)
        fail_loop([&](storage_ptr const& sp)
        {
            value v({1,2,3});
            array a({1, str_}, sp);
            a.insert(a.begin() + 1, 3, v);
            BEAST_EXPECT(a[0].is_number());
            BEAST_EXPECT(a[1].as_array().size() == 3);
            BEAST_EXPECT(a[2].as_array().size() == 3);
            BEAST_EXPECT(a[3].as_array().size() == 3);
            BEAST_EXPECT(a[4].is_string());
        });

        // insert(const_iterator, InputIt, InputIt)
        {
            // random iterator
            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<
                    value> init = {1, true};
                array a({str_}, sp);
                a.insert(a.begin(),
                    init.begin(), init.end());
                check(a);
            });

            // random iterator (multiple growth)
            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<value> init = {
                     1, str_, true,  1,  2,  3,  4,  5,  6,
                     7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
                    17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
                    27, 28, 29, 30 };
                BEAST_EXPECT(init.size() > min_capacity_);
                array a(sp);
                a.insert(a.begin(),
                    init.begin(), init.end());
            });

            // input iterator (empty range)
            {
                std::initializer_list<value> init;
                array a;
                a.insert(a.begin(),
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end()));
                BEAST_EXPECT(a.empty());
            }

            // input iterator
            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<
                    value> init = {1, true};
                array a({str_}, sp);
                a.insert(a.begin(),
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end()));
                check(a);
            });

            // input iterator (multiple growth)
            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<
                    value> init = {1, true, 1, 2, 3, 4, 5, 6, 7};
                array a({str_}, sp);
                a.insert(a.begin(),
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end()));
                BEAST_EXPECT(a.size() == init.size() + 1);
            });

            // backward relocate
            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<value> init = {1, 2};
                array a({"a", "b", "c", "d", "e"}, sp);
                a.insert(
                    a.begin() + 1,
                    init.begin(), init.end());
            });
        }

        // insert(const_iterator, init_list)
        fail_loop([&](storage_ptr const& sp)
        {
            array a({0, 3, 4}, sp);
            auto it = a.insert(
                a.begin() + 1, {1, str_});
            BEAST_EXPECT(it == a.begin() + 1);
            BEAST_EXPECT(a[0].as_int64() == 0);
            BEAST_EXPECT(a[1].as_int64() == 1);
            BEAST_EXPECT(a[2].as_string() == str_);
            BEAST_EXPECT(a[3].as_int64() == 3);
            BEAST_EXPECT(a[4].as_int64() == 4);
        });

        // emplace(const_iterator, arg)
        fail_loop([&](storage_ptr const& sp)
        {
            array a({0, 2, 3, 4}, sp);
            auto it = a.emplace(
                a.begin() + 1, str_);
            BEAST_EXPECT(it == a.begin() + 1);
            BEAST_EXPECT(a[0].as_int64() == 0);
            BEAST_EXPECT(a[1].as_string() == str_);
            BEAST_EXPECT(a[2].as_int64() == 2);
            BEAST_EXPECT(a[3].as_int64() == 3);
            BEAST_EXPECT(a[4].as_int64() == 4);
        });

        // erase(pos)
        {
            array a({1, true, nullptr, str_});
            a.erase(a.begin() + 2);
            check(a);
        }

        // erase(first, last)
        {
            array a({1, true, nullptr, 1.f, str_});
            a.erase(
                a.begin() + 2,
                a.begin() + 4);
            check(a);
        }

        // push_back(value const&)
        fail_loop([&](storage_ptr const& sp)
        {
            array a({1, true}, sp);
            value v(str_);
            a.push_back(v);
            BEAST_EXPECT(
                v.as_string() == str_);
            check(a);
            check_storage(a, sp);
        });

        // push_back(value&&)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                array a({1, true}, sp);
                value v(str_);
                a.push_back(std::move(v));
                check(a);
                check_storage(a, sp);
            });
        }

        // emplace_back(arg)
        fail_loop([&](storage_ptr const& sp)
        {
            array a({1, true}, sp);
            a.emplace_back(str_);
            check(a);
            check_storage(a, sp);
        });

        // pop_back()
        fail_loop([&](storage_ptr const& sp)
        {
            array a({1, true, str_, nullptr}, sp);
            a.pop_back();
            check(a);
            check_storage(a, sp);
        });

        // resize(size_type)
        {
            value v(array{});
            v.as_array().emplace_back(1);
            v.as_array().emplace_back(true);
            v.as_array().emplace_back(str_);

            fail_loop([&](storage_ptr const& sp)
            {
                array a(5, sp);
                a.resize(3);
                BEAST_EXPECT(a.size() == 3);
                check_storage(a, sp);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                array a(sp);
                a.resize(3);
                BEAST_EXPECT(a.size() == 3);
                check_storage(a, sp);
            });
        }

        // resize(size_type, value_type const&)
        {
            value v(array{});
            v.as_array().emplace_back(1);
            v.as_array().emplace_back(true);
            v.as_array().emplace_back(str_);

            fail_loop([&](storage_ptr const& sp)
            {
                array a(5, v, sp);
                a.resize(3, v);
                BEAST_EXPECT(a.size() == 3);
                check_storage(a, sp);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                array a(3, v, sp);
                a.resize(5, v);
                BEAST_EXPECT(a.size() == 5);
                check_storage(a, sp);
            });
        }

        // swap
        {
            // same storage
            {
                array a1({1, true, str_});
                array a2 = {1.};
                a1.swap(a2);
                check(a2);
                BEAST_EXPECT(a1.size() == 1);
                BEAST_EXPECT(a1.front().is_number());
                BEAST_EXPECT(a1.front().as_double() == 1.);
            }

            // different storage
            fail_loop([&](storage_ptr const& sp)
            {
                array a1({1, true, str_}, sp);
                array a2 = {1.};
                a1.swap(a2);
                check(a2);
                BEAST_EXPECT(a1.size() == 1);
            });
            fail_loop([&](storage_ptr const& sp)
            {
                array a1 = {1.};
                array a2({1, true, str_}, sp);
                a1.swap(a2);
                check(a1);
                BEAST_EXPECT(a2.size() == 1);
            });
        }
    }

    void
    testExceptions()
    {
        // operator=(array const&)
        fail_loop([&](storage_ptr const& sp)
        {
            array a0({1, true, str_});
            array a1;
            array a(sp);
            a.emplace_back(nullptr);
            a = a0;
            a1 = a;
            check(a1);
        });

        // operator=(init_list)
        fail_loop([&](storage_ptr const& sp)
        {
            init_list init{ 1, true, str_ };
            array a1;
            array a(sp);
            a.emplace_back(nullptr);
            a = init;
            a1 = a;
            check(a1);
        });

        // insert(const_iterator, count, value_type const&)
        fail_loop([&](storage_ptr const& sp)
        {
            array a1;
            array a({1, true}, sp);
            a.insert(a.begin() + 1,
                3, nullptr);
            a1 = a;
            BEAST_EXPECT(a1.size() == 5);
            BEAST_EXPECT(a1[0].is_number());
            BEAST_EXPECT(a1[1].is_null());
            BEAST_EXPECT(a1[2].is_null());
            BEAST_EXPECT(a1[3].is_null());
            BEAST_EXPECT(a1[4].is_bool());
        });

        // insert(const_iterator, InputIt, InputIt)
        fail_loop([&](storage_ptr const& sp)
        {
            init_list init{ 1, true, str_ };
            array a1;
            array a(sp);
            a.insert(a.end(),
                init.begin(), init.end());
            a1 = a;
            check(a1);
        });

        // emplace(const_iterator, arg)
        fail_loop([&](storage_ptr const& sp)
        {
            array a1;
            array a({1, nullptr}, sp);
            a.emplace(a.begin() + 1, true);
            a1 = a;
            BEAST_EXPECT(a1.size() == 3);
            BEAST_EXPECT(a1[0].is_number());
            BEAST_EXPECT(a1[1].is_bool());
            BEAST_EXPECT(a1[2].is_null());
        });

        // emplace(const_iterator, arg)
        fail_loop([&](storage_ptr const& sp)
        {
            array a1;
            array a({1, str_}, sp);
            a.emplace(a.begin() + 1, true);
            a1 = a;
            check(a1);
            BEAST_EXPECT(a1.size() == 3);
            BEAST_EXPECT(a1[0].is_number());
            BEAST_EXPECT(a1[1].is_bool());
            BEAST_EXPECT(a1[2].is_string());
        });
    }

    void
    run() override
    {
        testCtors();
        testAssignment();
        testGetStorage();
        testAccess();
        testIterators();
        testCapacity();
        testModifiers();
        testExceptions();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,array);

} // json
} // boost
