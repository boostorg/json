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

#include "test_storage.hpp"

namespace boost {
namespace json {

class array_test : public beast::unit_test::suite
{
public:
    void
    check(array const& arr)
    {
        BEAST_EXPECT(arr.size() == 3);
        BEAST_EXPECT(arr[0].is_number());
        BEAST_EXPECT(arr[1].is_bool());
        BEAST_EXPECT(arr[2].is_string());
    }

    void
    check(
        array const& arr,
        storage_ptr const& sp)
    {
        check(arr);
        BEAST_EXPECT(arr.get_storage() == sp);
        BEAST_EXPECT(arr[0].get_storage() == sp);
        BEAST_EXPECT(arr[1].get_storage() == sp);
        BEAST_EXPECT(arr[2].get_storage() == sp);
    }

    void
    testSpecial()
    {
        unique_storage us;
        storage_ptr sp(&us);
        storage_ptr sp0 =
            default_storage();
        BEAST_EXPECT(*sp != *sp0);

        // array()
        {
            array arr;
            BEAST_EXPECT(arr.empty());
            BEAST_EXPECT(arr.size() == 0);
        }

        // array(storage)
        {
            array arr(sp);
            BEAST_EXPECT(arr.get_storage() == sp);
            BEAST_EXPECT(arr.get_storage() != sp0);
        }

        // array(size_type)
        {
            array arr(3);
            BEAST_EXPECT(arr.size() == 3);
            for(auto const& v : arr)
                BEAST_EXPECT(v.is_null());
        }

        // array(size_type, storage)
        {
            array arr(3, sp);
            BEAST_EXPECT(arr.size() == 3);
            for(auto const& v : arr)
                BEAST_EXPECT(v.get_storage() == sp);
        }

        // array(size_type, value)
        {
            array arr(3, kind::boolean);
            BEAST_EXPECT(arr.size() == 3);
            for(auto const& v : arr)
                BEAST_EXPECT(v.is_bool());
        }

        // array(size_type, value, storage)
        {
            array arr(3, kind::boolean, sp);
            BEAST_EXPECT(arr.size() == 3);
            for(auto const& v : arr)
                BEAST_EXPECT(v.get_storage() == sp);
        }

        // array(InputIt, InputIt)
        {
            std::initializer_list<value> list =
                { 1, true, "hello" };
            array arr(list.begin(), list.end());
            check(arr, sp0);
        }

        // array(InputIt, InputIt, storage)
        {
            std::initializer_list<value> list =
                { 1, true, "hello" };
            array arr(list.begin(), list.end(), sp);
            check(arr, sp);
        }

        // array(array const&)
        {
            std::initializer_list<value> list =
                { 1, true, "hello" };
            array arr1(list.begin(), list.end(), sp);
            array arr2 = arr1;
            check(arr2, sp);
        }

        // array(array const&, storage)
        {
            std::initializer_list<value> list =
                { 1, true, "hello" };
            array arr1(list.begin(), list.end());
            array arr2(arr1, sp);
            BEAST_EXPECT(arr2.size() == 3);
            BEAST_EXPECT(
                arr1[0].get_storage() !=
                arr2[0].get_storage());
            BEAST_EXPECT(
                arr1[1].get_storage() !=
                arr2[1].get_storage());
            BEAST_EXPECT(
                arr1[2].get_storage() !=
                arr2[2].get_storage());
        }

        // array(array&&)
        {
            std::initializer_list<value> list =
                { 1, true, "hello" };
            array arr1(list.begin(), list.end(), sp);
            array arr2 = std::move(arr1);
            BEAST_EXPECT(arr1.empty());
            BEAST_EXPECT(
                arr1.get_storage() ==
                arr2.get_storage());
            check(arr2, sp);
        }

        // array(array&&, storage)
        {
            std::initializer_list<value> list =
                { 1, true, "hello" };
            array arr1(list.begin(), list.end());
            array arr2(std::move(arr1), sp);
            BEAST_EXPECT(! arr1.empty());
            BEAST_EXPECT(
                arr1.get_storage() !=
                arr2.get_storage());
            check(arr2, sp);
        }

        // array(init_list)
        {
            array arr({1, true, "hello"});
            check(arr, sp0);
        }

        // array(init_list, storage)
        {
            array arr({1, true, "hello"}, sp);
            check(arr, sp);
        }

        // operator=(array&&)
        {
            {
                array arr1({1, true, "hello"});
                array arr2({nullptr, kind::object, 1.f});
                arr2 = std::move(arr1);
                BEAST_EXPECT(arr1.empty());
                BEAST_EXPECT(
                    arr1.get_storage() ==
                    arr2.get_storage());
                check(arr2, sp0);
            }
            {
                array arr1({1, true, "hello"});
                array arr2({nullptr, kind::object, 1.f}, sp);
                arr2 = std::move(arr1);
                BEAST_EXPECT(! arr1.empty());
                BEAST_EXPECT(
                    arr1.get_storage() !=
                    arr2.get_storage());
                check(arr2, sp);
            }
        }

        // operator=(array const&)
        {
            {
                array arr1({1, true, "hello"});
                array arr2({nullptr, kind::object, 1.f});
                arr2 = arr1;
                BEAST_EXPECT(! arr1.empty());
                check(arr2, sp0);
            }
            {
                array arr1({1, true, "hello"});
                array arr2({nullptr, kind::object, 1.f}, sp);
                arr2 = arr1;
                BEAST_EXPECT(! arr1.empty());
                BEAST_EXPECT(
                    arr1.get_storage() !=
                    arr2.get_storage());
                check(arr2, sp);
            }
        }

        // operator=(init_list)
        {
            {
                std::initializer_list<value> list =
                    { 1, true, "hello" };
                array arr({nullptr, kind::object, 1.f});
                arr = list;
                check(arr, sp0);
            }
            {
                std::initializer_list<value> list =
                    { 1, true, "hello" };
                array arr({nullptr, kind::object, 1.f}, sp);
                arr = list;
                check(arr, sp);
            }
        }
    }

    void
    testElements()
    {
        // at(pos)
        {
            array arr({1, true, "hello"});
            BEAST_EXPECT(arr.at(0).is_number());
            BEAST_EXPECT(arr.at(1).is_bool());
            BEAST_EXPECT(arr.at(2).is_string());
            try
            {
                arr.at(3);
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // at(pos) const
        {
            array const arr({1, true, "hello"});
            BEAST_EXPECT(arr.at(0).is_number());
            BEAST_EXPECT(arr.at(1).is_bool());
            BEAST_EXPECT(arr.at(2).is_string());
            try
            {
                arr.at(3);
                BEAST_FAIL();
            }
            catch(std::out_of_range const&)
            {
                BEAST_PASS();
            }
        }

        // operator[](size_type)
        {
            array arr({1, true, "hello"});
            BEAST_EXPECT(arr[0].is_number());
            BEAST_EXPECT(arr[1].is_bool());
            BEAST_EXPECT(arr[2].is_string());
        }

        // operator[](size_type) const
        {
            array const arr({1, true, "hello"});
            BEAST_EXPECT(arr[0].is_number());
            BEAST_EXPECT(arr[1].is_bool());
            BEAST_EXPECT(arr[2].is_string());
        }

        // front()
        {
            array arr({1, true, "hello"});
            BEAST_EXPECT(arr.front().is_number());
        }

        // front() const
        {
            array const arr({1, true, "hello"});
            BEAST_EXPECT(arr.front().is_number());
        }

        // back()
        {
            array arr({1, true, "hello"});
            BEAST_EXPECT(arr.back().is_string());
        }

        // back() const
        {
            array const arr({1, true, "hello"});
            BEAST_EXPECT(arr.back().is_string());
        }

        // data()
        {
            array arr({1, true, "hello"});
            BEAST_EXPECT(arr.data() == &arr[0]);
        }

        // data() const
        {
            array const arr({1, true, "hello"});
            BEAST_EXPECT(arr.data() == &arr[0]);
        }
    }

    void
    testIterators()
    {
        array arr({1, true, "hello"});
        auto const& ac(arr);
        {
            auto it = arr.begin();
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it == arr.end());
        }
        {
            auto it = arr.cbegin();
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it == arr.cend());
        }
        {
            auto it = ac.begin();
            BEAST_EXPECT(it->is_number()); ++it;
            BEAST_EXPECT(it->is_bool());   it++;
            BEAST_EXPECT(it->is_string()); ++it;
            BEAST_EXPECT(it == ac.end());
        }
        {
            auto it = arr.end();
            --it; BEAST_EXPECT(it->is_string());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_number());
            BEAST_EXPECT(it == arr.begin());
        }
        {
            auto it = arr.cend();
            --it; BEAST_EXPECT(it->is_string());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_number());
            BEAST_EXPECT(it == arr.cbegin());
        }
        {
            auto it = ac.end();
            --it; BEAST_EXPECT(it->is_string());
            it--; BEAST_EXPECT(it->is_bool());
            --it; BEAST_EXPECT(it->is_number());
            BEAST_EXPECT(it == ac.begin());
        }
    }

    void
    testCapacity()
    {
        // empty()
        {
            array arr;
            BEAST_EXPECT(arr.empty());
            arr.emplace_back(1);
            BEAST_EXPECT(! arr.empty());
        }

        // size()
        {
            array arr;
            BEAST_EXPECT(arr.size() == 0);
            arr.emplace_back(1);
            BEAST_EXPECT(arr.size() == 1);
        }

        // max_size()
        {
            array arr;
            BEAST_EXPECT(arr.max_size() > 0);
        }

        // reserve()
        {
            array arr;
            arr.reserve(50);
            BEAST_EXPECT(arr.capacity() >= 50);
        }

        // capacity()
        {
            array arr;
            BEAST_EXPECT(arr.capacity() == 0);
        }

        // shrink_to_fit()
        {
            array arr;
            arr.reserve(50);
            BEAST_EXPECT(arr.capacity() >= 50);
            arr.shrink_to_fit();
            BEAST_EXPECT(arr.capacity() == 0);
        }
    }

    void
    testModifiers()
    {
        // clear
        {
            array arr({1, true, "hello"});
            arr.clear();
            BEAST_EXPECT(arr.capacity() > 0);
        }

        // insert(before, value_type const&)
        {
            array arr({1, "hello"});
            value v(true);
            arr.insert(arr.begin() + 1, v);
            check(arr);
        }

        // insert(before, value_type const&)
        {
            array arr({1, "hello"});
            arr.insert(arr.begin() + 1, true);
            check(arr);
        }
        // insert(before, size_type, value_type const&)
        {
            array arr({1, "hello"});
            arr.insert(arr.begin() + 1, 3, true);
            BEAST_EXPECT(arr[0].is_number());
            BEAST_EXPECT(arr[1].is_bool());
            BEAST_EXPECT(arr[2].is_bool());
            BEAST_EXPECT(arr[3].is_bool());
            BEAST_EXPECT(arr[4].is_string());
        }

        // insert(before, InputIt, InputIt)
        {
            std::initializer_list<
                value> list = {1, true};
            array arr({"hello"});
            arr.insert(arr.begin(),
                list.begin(), list.end());
            check(arr);
        }

        // insert(before, init_list)
        {
            array arr({"hello"});
            arr.insert(arr.begin(), {1, true});
            check(arr);
        }

        // emplace(before, arg)
        {
            array arr({1, "hello"});
            auto it = arr.emplace(
                arr.begin() + 1, true);
            BEAST_EXPECT(it == arr.begin() + 1);
            check(arr);
        }

        // erase(pos)
        {
            array arr({1, true, nullptr, "hello"});
            arr.erase(arr.begin() + 2);
            check(arr);
        }

        // push_back(value const&)
        {
            array arr({1, true});
            value v("hello");
            arr.push_back(v);
            BEAST_EXPECT(
                v.as_string() == "hello");
            check(arr);
        }

        // push_back(value&&)
        {
            array arr({1, true});
            value v("hello");
            arr.push_back(std::move(v));
            BEAST_EXPECT(v.is_null());
            check(arr);
        }

        // emplace_back(arg)
        {
            array arr({1, true});
            arr.emplace_back("hello");
            check(arr);
        }

        // pop_back()
        {
            array arr({1, true, "hello", nullptr});
            arr.pop_back();
            check(arr);
        }

        // resize(size_type)
        {
            array arr;
            arr.resize(3);
            BEAST_EXPECT(arr.size() == 3);
            BEAST_EXPECT(arr[0].is_null());
            BEAST_EXPECT(arr[1].is_null());
            BEAST_EXPECT(arr[2].is_null());
        }

        // resize(size_type, value_type const&)
        {
            array arr;
            value v(kind::boolean);
            arr.resize(3, v);
            BEAST_EXPECT(arr.size() == 3);
            BEAST_EXPECT(arr[0].is_bool());
            BEAST_EXPECT(arr[1].is_bool());
            BEAST_EXPECT(arr[2].is_bool());
        }

        // swap
        {
            array arr1({1, true, "hello"});
            array arr2;
            arr1.swap(arr2);
            check(arr2);
            BEAST_EXPECT(arr1.empty());
        }
    }

    void
    testExceptions()
    {
        // operator=(array const&)
        {
            array arr0({1, true, "hello"});
            fail_storage fs;
            storage_ptr sp(&fs);
            array arr1;
            while(fs.fail < 200)
            {
                try
                {
                    array arr(sp);
                    arr.emplace_back(nullptr);
                    arr = arr0;
                    arr1 = arr;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            check(arr1);
        }

        // operator=(init_list)
        {
            std::initializer_list<value> list(
                {1, true, "hello"});
            fail_storage fs;
            storage_ptr sp(&fs);
            array arr1;
            while(fs.fail < 200)
            {
                try
                {
                    array arr(sp);
                    arr.emplace_back(nullptr);
                    arr = list;
                    arr1 = arr;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            check(arr1);
        }

        // insert(before, count, value_type const&)
        {
            fail_storage fs;
            storage_ptr sp(&fs);
            array arr1;
            while(fs.fail < 200)
            {
                try
                {
                    array arr({1, true}, sp);
                    arr.insert(arr.begin() + 1,
                        3, kind::null);
                    arr1 = arr;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            BEAST_EXPECT(arr1.size() == 5);
            BEAST_EXPECT(arr1[0].is_number());
            BEAST_EXPECT(arr1[1].is_null());
            BEAST_EXPECT(arr1[2].is_null());
            BEAST_EXPECT(arr1[3].is_null());
            BEAST_EXPECT(arr1[4].is_bool());
        }

    #if _ITERATOR_DEBUG_LEVEL == 0
        // insert(before, InputIt, InputIt)
        {
            std::initializer_list<value> list(
                {1, true, "hello"});
            fail_storage fs;
            storage_ptr sp(&fs);
            array arr1;
            while(fs.fail < 200)
            {
                try
                {
                    array arr(sp);
                    arr.insert(arr.end(),
                        list.begin(), list.end());
                    arr1 = arr;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            check(arr1);
        }
    #endif

        // emplace(before, arg)
        {
            fail_storage fs;
            storage_ptr sp(&fs);
            array arr1;
            while(fs.fail < 200)
            {
                try
                {
                    array arr({1, nullptr}, sp);
                    arr.emplace(arr.begin() + 1, true);
                    arr1 = arr;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            BEAST_EXPECT(arr1.size() == 3);
            BEAST_EXPECT(arr1[0].is_number());
            BEAST_EXPECT(arr1[1].is_bool());
            BEAST_EXPECT(arr1[2].is_null());
        }

    #if _ITERATOR_DEBUG_LEVEL == 0
        // emplace(before, arg)
        {
            fail_storage fs;
            storage_ptr sp(&fs);
            array arr1;
            while(fs.fail < 200)
            {
                try
                {
                    array arr({1, "hello"}, sp);
                    arr.emplace(arr.begin() + 1, true);
                    arr1 = arr;
                    break;
                }
                catch(std::bad_alloc const&)
                {
                }
            }
            check(arr1);
            BEAST_EXPECT(arr1.size() == 3);
            BEAST_EXPECT(arr1[0].is_number());
            BEAST_EXPECT(arr1[1].is_bool());
            BEAST_EXPECT(arr1[2].is_string());
        }
    #endif
    }

    void
    run() override
    {
        testSpecial();
        testElements();
        testIterators();
        testCapacity();
        testModifiers();
        testExceptions();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,array);

} // json
} // boost
