//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/value.hpp>

#include <memory>
#include <vector>

#include "test.hpp"
#include "test_suite.hpp"

namespace boost {
namespace json {

namespace value_test_ns {

struct T1
{
    void
    to_json(value&) const
    {
    }
};

struct T2
{
};

void
to_json(T2 const&, value&)
{
}

struct T3
{
};

} // value_test_ns

template<>
struct value_exchange<value_test_ns::T3>
{
    static
    void
    to_json(value_test_ns::T3, value&)
    {
    }
};

//----------------------------------------------------------

class value_test
{
public:
    //BOOST_STATIC_ASSERT(has_to_json<short>::value);

    string_view const str_;

    value_test()
        : str_(
            "abcdefghijklmnopqrstuvwxyz")
    {
        // ensure this string does
        // not fit in the SBO area.
        BOOST_ASSERT(str_.size() >
            string().capacity());
    }

    //------------------------------------------------------

    void
    testSpecial()
    {
        auto dsp = storage_ptr{};
        auto sp = make_storage<unique_storage>();

        // ~value()
        {
            value(object{});
            value(array{});
            value(string{});
            value(std::int64_t(0));
            value(std::uint64_t(0));
            value(double(0));
            value(true);
            value(nullptr);
        }

        // value()
        {
            value jv;
            BOOST_TEST(*jv.storage() == *dsp);
        }

        // value(storage_ptr)
        {
            value jv(sp);
            BOOST_TEST(*jv.storage() == *sp);
        }

        // value(pilfered<value>)
        {
            {
                value jv1(object{});
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(pilfer(jv1));
                BOOST_TEST(jv2.is_null());
            }
        }

        // value(value const&)
        {
            {
                value jv1(object{});
                value jv2(jv1);
                BOOST_TEST(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(jv1);
                BOOST_TEST(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(jv1);
                BOOST_TEST(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(jv1);
                BOOST_TEST(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(jv1);
                BOOST_TEST(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(jv1);
                BOOST_TEST(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(jv1);
                BOOST_TEST(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(jv1);
                BOOST_TEST(jv2.is_null());
            }
        }

        // value(value const&, storage_ptr)
        {
            {
                value jv1(object{});
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_object());
                BOOST_TEST(*jv2.storage() == *sp);
            }
            {
                value jv1(array{});
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_array());
                BOOST_TEST(*jv2.storage() == *sp);
            }
            {
                value jv1(string{});
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_string());
                BOOST_TEST(*jv2.storage() == *sp);
            }
            {
                value jv1(std::int64_t{});
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_int64());
                BOOST_TEST(*jv2.storage() == *sp);
            }
            {
                value jv1(std::uint64_t{});
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_uint64());
                BOOST_TEST(*jv2.storage() == *sp);
            }
            {
                value jv1(double{});
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_double());
                BOOST_TEST(*jv2.storage() == *sp);
            }
            {
                value jv1(true);
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_bool());
                BOOST_TEST(*jv2.storage() == *sp);
            }
            {
                value jv1(nullptr);
                value jv2(jv1, sp);
                BOOST_TEST(jv2.is_null());
                BOOST_TEST(*jv2.storage() == *sp);
            }
        }

        // value(value&&)
        {
            {
                value jv1(object{});
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(std::move(jv1));
                BOOST_TEST(jv2.is_null());
            }
        }

        // value(value&&, storage_ptr)
        {
            {
                value jv1(object{});
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(std::move(jv1), sp);
                BOOST_TEST(jv2.is_null());
            }
        }

        // operator=(value&&)
        {
            {
                value jv;
                jv = value(object{});
                BOOST_TEST(jv.is_object());
            }
            {
                value jv;
                jv = value(array{});
                BOOST_TEST(jv.is_array());
            }
            {
                value jv;
                jv = value(string{});
                BOOST_TEST(jv.is_string());
            }
            {
                value jv;
                jv = value(std::int64_t{});
                BOOST_TEST(jv.is_int64());
            }
            {
                value jv;
                jv = value(std::uint64_t{});
                BOOST_TEST(jv.is_uint64());
            }
            {
                value jv;
                jv = value(double{});
                BOOST_TEST(jv.is_double());
            }
            {
                value jv;
                jv = value(true);
                BOOST_TEST(jv.is_bool());
            }
            {
                value jv;
                jv = value(nullptr);
                BOOST_TEST(jv.is_null());
            }

            fail_loop([&](storage_ptr const sp)
            {
                value jv(sp);
                jv = value({
                    { {"a",1}, {"b",2u} },
                    { 1, 2 },
                    "hello",
                    1,
                    2u,
                    3.,
                    true,
                    nullptr
                    });
                BOOST_TEST(jv.as_array().size() == 8);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv(sp);
                jv = value({
                    { "aa", { {"a",1}, {"b",2u} } },
                    { "bb", { 1, 2 } },
                    { "cc", "hello" },
                    { "dd", 1 },
                    { "ee", 2u },
                    { "ff", 3. },
                    { "gg", true },
                    { "hh", nullptr },
                    });
                BOOST_TEST(jv.as_object().size() == 8);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv(sp);
                jv = value(str_);
                BOOST_TEST(jv.as_string() == str_);
            });
        }

        // operator=(value const&)
        {
            {
                value jv1(object{});
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2;
                jv2 = jv1;
                BOOST_TEST(jv2.is_null());
            }

            fail_loop([&](storage_ptr const sp)
            {
                value jv1({1, 2, 3, 4, 5});
                value jv2(sp);
                jv2 = jv1;
                BOOST_TEST(jv2.as_array().size() == 5);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv1({
                    {"a",1},{"b",2},{"c",3}});
                value jv2(sp);
                jv2 = jv1;
                BOOST_TEST(jv2.as_object().size() == 3);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv1(str_);
                value jv2(sp);
                jv2 = jv1;
                BOOST_TEST(jv2.as_string() == str_);
            });

            // self-assign
            {
                value jv(object{});
                jv = static_cast<value const&>(jv);
            }
        }
    }

    void
    testConversion()
    {
        auto dsp = storage_ptr{};
        auto sp = make_storage<unique_storage>();

        // value(object)
        // value(object, storage_ptr)
        // value(object_kind, storage_ptr)
        {
            {
                auto jv = value(object());
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(object(sp));
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *sp);
            }
            {
                auto jv = value(object(), sp);
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *sp);
            }
            {
                auto jv = value(object_kind);
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(object_kind, sp);
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(array)
        // value(array, storage_ptr)
        // value(object_kind, storage_ptr)
        {
            {
                auto jv = value(array());
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(array(sp));
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *sp);
            }
            {
                auto jv = value(array(), sp);
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *sp);
            }
            {
                auto jv = value(array_kind);
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(array_kind, sp);
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(string)
        // value(string, storage_ptr)
        {
            {
                auto jv = value(string());
                BOOST_TEST(jv.is_string());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(string(sp));
                BOOST_TEST(jv.is_string());
                BOOST_TEST(*jv.storage() == *sp);
            }
            {
                auto jv = value(string(), sp);
                BOOST_TEST(jv.is_string());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(int64)
        // value(int64, storage_ptr)
        {
            {
                auto jv = value(std::int64_t{-65536});
                BOOST_TEST(jv.is_int64());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(std::int64_t{-65536}, sp);
                BOOST_TEST(jv.is_int64());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(uint64)
        // value(uint64, storage_ptr)
        {
            {
                auto jv = value(std::uint64_t{65536});
                BOOST_TEST(jv.is_uint64());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(std::uint64_t{65536}, sp);
                BOOST_TEST(jv.is_uint64());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(double)
        // value(double, storage_ptr)
        {
            {
                auto jv = value(double{3.141});
                BOOST_TEST(jv.is_double());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(double{3.141}, sp);
                BOOST_TEST(jv.is_double());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(bool)
        // value(bool, storage_ptr)
        {
            {
                auto jv = value(true);
                BOOST_TEST(jv.is_bool());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(true, sp);
                BOOST_TEST(jv.is_bool());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(nullptr_t)
        // value(nullptr_t, storage_ptr)
        {
            {
                auto jv = value(nullptr);
                BOOST_TEST(jv.is_null());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                auto jv = value(nullptr, sp);
                BOOST_TEST(jv.is_null());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // value(initializer_list)
        // value(initializer_list, storage_ptr)
        {
            {
                value jv({1, 2, 3});
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                value jv({1, 2, 3}, sp);
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *sp);
            }
            {
                value jv({{"a",1},{"b",2},{"c",3}});
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                value jv({{"a",1},{"b",2},{"c",3}}, sp);
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // operator=(object)
        {
            {
                value jv;
                jv = object();
                BOOST_TEST(*jv.storage() == *dsp);
                BOOST_TEST(jv.is_object());
            }
            {
                value jv(sp);
                jv = object();
                BOOST_TEST(jv.is_object());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // operator=(array)
        {
            {
                value jv;
                jv = array();
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                value jv(sp);
                jv = array();
                BOOST_TEST(jv.is_array());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // operator=(string)
        {
            {
                value jv;
                jv = string();
                BOOST_TEST(jv.is_string());
                BOOST_TEST(*jv.storage() == *dsp);
            }
            {
                value jv(sp);
                jv = string();
                BOOST_TEST(jv.is_string());
                BOOST_TEST(*jv.storage() == *sp);
            }
        }

        // operator=(int64)
        {
            value jv;
            jv = std::int64_t{-65536};
            BOOST_TEST(jv.is_int64());
        }

        // operator=(uint64)
        {
            value jv;
            jv = std::uint64_t{65536};
            BOOST_TEST(jv.is_uint64());
        }

        // operator=(double)
        {
            value jv;
            jv = double{3.141};
            BOOST_TEST(jv.is_double());
        }
    }

    void
    testModifiers()
    {
        // emplace
        {
            BOOST_TEST(value().emplace_object().empty());
            BOOST_TEST(value().emplace_array().empty());
            BOOST_TEST(value().emplace_string().empty());
            BOOST_TEST(value().emplace_int64() == 0);
            BOOST_TEST(value().emplace_uint64() == 0);
            BOOST_TEST(value().emplace_double() == 0);
            BOOST_TEST(value().emplace_bool() = true);
            {
                value jv(object_kind);
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
            {
                value jv(array_kind);
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
            {
                value jv(string_kind);
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
            {
                value jv(-1);
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
            {
                value jv(1U);
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
            {
                value jv(1.0);
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
            {
                value jv(false);
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
            {
                value jv;
                jv.emplace_null();
                BOOST_TEST(jv.is_null());
            }
        }

        // swap
        {
            {
                value jv1(1);
                value jv2("abc");
                BOOST_TEST(
                    *jv1.storage() == *jv2.storage());
                swap(jv1, jv2);
                BOOST_TEST(jv1.as_string() == "abc");
                BOOST_TEST(jv2.as_int64() == 1);
            }

            // different storage
            fail_loop([&](storage_ptr const& sp)
            {
                value jv1(1);
                value jv2("abc", sp);
                BOOST_TEST(
                    *jv1.storage() != *jv2.storage());
                swap(jv1, jv2);
                BOOST_TEST(jv1.as_string() == "abc");
                BOOST_TEST(jv2.as_int64() == 1);
            });
        }
    }

    template<typename T>
    using tt = T;

    void
    testExchange()
    {
        // string{}
        {
            BOOST_TEST(value("abc").is_string());
            BOOST_TEST(value(std::string()).is_string());
            BOOST_TEST(value(string_view()).is_string());

            { value jv; BOOST_TEST((jv = "abc").is_string()); }
            { value jv; BOOST_TEST((jv = std::string()).is_string()); }
            { value jv; BOOST_TEST((jv = string_view()).is_string()); }
        }

        // numbers
        {
            BOOST_TEST(value(tt<int>{}).is_int64());
            BOOST_TEST(value(tt<short int>{}).is_int64());
            BOOST_TEST(value(tt<long int>{}).is_int64());
            BOOST_TEST(value(tt<long long int>{}).is_int64());
            BOOST_TEST(value(tt<unsigned int>{}).is_uint64());
            BOOST_TEST(value(tt<unsigned short int>{}).is_uint64());
            BOOST_TEST(value(tt<unsigned long int>{}).is_uint64());
            BOOST_TEST(value(tt<unsigned long long int>{}).is_uint64());
            BOOST_TEST(value(tt<float>{}).is_double());
            BOOST_TEST(value(tt<double>{}).is_double());
            BOOST_TEST(value(tt<long double>{}).is_double());

            { value jv; BOOST_TEST((jv = tt<int>{}).is_int64()); }
            { value jv; BOOST_TEST((jv = tt<short int>{}).is_int64()); }
            { value jv; BOOST_TEST((jv = tt<long int>{}).is_int64()); }
            { value jv; BOOST_TEST((jv = tt<long long int>{}).is_int64()); }
            { value jv; BOOST_TEST((jv = tt<unsigned int>{}).is_uint64()); }
            { value jv; BOOST_TEST((jv = tt<unsigned short int>{}).is_uint64()); }
            { value jv; BOOST_TEST((jv = tt<unsigned long int>{}).is_uint64()); }
            { value jv; BOOST_TEST((jv = tt<unsigned long long int>{}).is_uint64()); }
            { value jv; BOOST_TEST((jv = tt<float>{}).is_double()); }
            { value jv; BOOST_TEST((jv = tt<double>{}).is_double()); }
            { value jv; BOOST_TEST((jv = tt<long double>{}).is_double()); }
        }

        // bool
        {
            BOOST_TEST(value(true).is_bool());
            BOOST_TEST(value(false).is_bool());

            { value jv; BOOST_TEST((jv = true).is_bool()); }
            { value jv; BOOST_TEST((jv = false).is_bool()); }
        }

        // nullptr
        {
            BOOST_TEST(value(nullptr).is_null());

            { value jv("x"); BOOST_TEST((jv = nullptr).is_null()); }
        }

        // VFALCO TODO store()
    }

    void
    testObservers()
    {
        value const obj(object{});
        value const arr(array{});
        value const str(string{});
        value const i64(std::int64_t{});
        value const u64(std::uint64_t{});
        value const dub(double{});
        value const bln(true);
        value const nul(nullptr);

        // kind()
        {
            BOOST_TEST(obj.kind() == kind::object);
            BOOST_TEST(arr.kind() == kind::array);
            BOOST_TEST(str.kind() == kind::string);
            BOOST_TEST(i64.kind() == kind::int64);
            BOOST_TEST(u64.kind() == kind::uint64);
            BOOST_TEST(dub.kind() == kind::double_);
            BOOST_TEST(bln.kind() == kind::bool_);
            BOOST_TEST(nul.kind() == kind::null);
        }

        // is_object()
        {
            BOOST_TEST(  obj.is_object());
            BOOST_TEST(! arr.is_object());
            BOOST_TEST(! str.is_object());
            BOOST_TEST(! i64.is_object());
            BOOST_TEST(! u64.is_object());
            BOOST_TEST(! dub.is_object());
            BOOST_TEST(! bln.is_object());
            BOOST_TEST(! nul.is_object());
        }

        // is_array()
        {
            BOOST_TEST(! obj.is_array());
            BOOST_TEST(  arr.is_array());
            BOOST_TEST(! str.is_array());
            BOOST_TEST(! i64.is_array());
            BOOST_TEST(! u64.is_array());
            BOOST_TEST(! dub.is_array());
            BOOST_TEST(! bln.is_array());
            BOOST_TEST(! nul.is_array());
        }

        // is_string()
        {
            BOOST_TEST(! obj.is_string());
            BOOST_TEST(! arr.is_string());
            BOOST_TEST(  str.is_string());
            BOOST_TEST(! i64.is_string());
            BOOST_TEST(! u64.is_string());
            BOOST_TEST(! dub.is_string());
            BOOST_TEST(! bln.is_string());
            BOOST_TEST(! nul.is_string());
        }

        // is_int64()
        {
            BOOST_TEST(! obj.is_int64());
            BOOST_TEST(! arr.is_int64());
            BOOST_TEST(! str.is_int64());
            BOOST_TEST(  i64.is_int64());
            BOOST_TEST(! u64.is_int64());
            BOOST_TEST(! dub.is_int64());
            BOOST_TEST(! bln.is_int64());
            BOOST_TEST(! nul.is_int64());
        }

        // is_uint64()
        {
            BOOST_TEST(! obj.is_uint64());
            BOOST_TEST(! arr.is_uint64());
            BOOST_TEST(! str.is_uint64());
            BOOST_TEST(! i64.is_uint64());
            BOOST_TEST(  u64.is_uint64());
            BOOST_TEST(! dub.is_uint64());
            BOOST_TEST(! bln.is_uint64());
            BOOST_TEST(! nul.is_uint64());
        }

        // is_double()
        {
            BOOST_TEST(! obj.is_double());
            BOOST_TEST(! arr.is_double());
            BOOST_TEST(! str.is_double());
            BOOST_TEST(! i64.is_double());
            BOOST_TEST(! u64.is_double());
            BOOST_TEST(  dub.is_double());
            BOOST_TEST(! bln.is_double());
            BOOST_TEST(! nul.is_double());
        }

        // is_bool()
        {
            BOOST_TEST(! obj.is_bool());
            BOOST_TEST(! arr.is_bool());
            BOOST_TEST(! str.is_bool());
            BOOST_TEST(! i64.is_bool());
            BOOST_TEST(! u64.is_bool());
            BOOST_TEST(! dub.is_bool());
            BOOST_TEST(  bln.is_bool());
            BOOST_TEST(! nul.is_bool());
        }

        // is_null()
        {
            BOOST_TEST(! obj.is_null());
            BOOST_TEST(! arr.is_null());
            BOOST_TEST(! str.is_null());
            BOOST_TEST(! i64.is_null());
            BOOST_TEST(! u64.is_null());
            BOOST_TEST(! dub.is_null());
            BOOST_TEST(! bln.is_null());
            BOOST_TEST(  nul.is_null());
        }

        // is_number()
        {
            BOOST_TEST(! obj.is_number());
            BOOST_TEST(! arr.is_number());
            BOOST_TEST(! str.is_number());
            BOOST_TEST(  i64.is_number());
            BOOST_TEST(  u64.is_number());
            BOOST_TEST(  dub.is_number());
            BOOST_TEST(! bln.is_number());
            BOOST_TEST(! nul.is_number());
        }

        // is_primitive()
        {
            BOOST_TEST(! obj.is_primitive());
            BOOST_TEST(! arr.is_primitive());
            BOOST_TEST(  str.is_primitive());
            BOOST_TEST(  i64.is_primitive());
            BOOST_TEST(  u64.is_primitive());
            BOOST_TEST(  dub.is_primitive());
            BOOST_TEST(  bln.is_primitive());
            BOOST_TEST(  nul.is_primitive());
        }

        // is_structured()
        {
            BOOST_TEST(  obj.is_structured());
            BOOST_TEST(  arr.is_structured());
            BOOST_TEST(! str.is_structured());
            BOOST_TEST(! i64.is_structured());
            BOOST_TEST(! u64.is_structured());
            BOOST_TEST(! dub.is_structured());
            BOOST_TEST(! bln.is_structured());
            BOOST_TEST(! nul.is_structured());
        }
    }

    //------------------------------------------------------
    //
    // Accessors
    //
    //------------------------------------------------------

    void
    testGetStorage()
    {
        auto const sp =
            make_storage<unique_storage>();

        value obj(object{}, sp);
        value arr(array{}, sp);
        value str(string{}, sp);
        value i64(std::int64_t{}, sp);
        value u64(std::uint64_t{}, sp);
        value dub(double{}, sp);
        value bln(true, sp);
        value nul(nullptr, sp);

        // storage()
        {
            BOOST_TEST(*obj.storage() == *sp);
            BOOST_TEST(*arr.storage() == *sp);
            BOOST_TEST(*str.storage() == *sp);
            BOOST_TEST(*i64.storage() == *sp);
            BOOST_TEST(*u64.storage() == *sp);
            BOOST_TEST(*dub.storage() == *sp);
            BOOST_TEST(*bln.storage() == *sp);
            BOOST_TEST(*nul.storage() == *sp);
        }
    }

    void
    testIf()
    {
        value obj(object{});
        value arr(array{});
        value str(string{});
        value i64(std::int64_t{});
        value u64(std::uint64_t{});
        value dub(double{});
        value bln(true);
        value nul(nullptr);

        auto const& cobj(obj);
        auto const& carr(arr);
        auto const& cstr(str);
        auto const& ci64(i64);
        auto const& cu64(u64);
        auto const& cdub(dub);
        auto const& cbln(bln);
        auto const& cnul(nul);

        // if_object()
        {
            BOOST_TEST(obj.if_object() != nullptr);
            BOOST_TEST(arr.if_object() == nullptr);
            BOOST_TEST(str.if_object() == nullptr);
            BOOST_TEST(i64.if_object() == nullptr);
            BOOST_TEST(u64.if_object() == nullptr);
            BOOST_TEST(dub.if_object() == nullptr);
            BOOST_TEST(bln.if_object() == nullptr);
            BOOST_TEST(nul.if_object() == nullptr);
        }

        // if_object() const
        {
            BOOST_TEST(cobj.if_object() != nullptr);
            BOOST_TEST(carr.if_object() == nullptr);
            BOOST_TEST(cstr.if_object() == nullptr);
            BOOST_TEST(ci64.if_object() == nullptr);
            BOOST_TEST(cu64.if_object() == nullptr);
            BOOST_TEST(cdub.if_object() == nullptr);
            BOOST_TEST(cbln.if_object() == nullptr);
            BOOST_TEST(cnul.if_object() == nullptr);
        }

        // if_array()
        {
            BOOST_TEST(obj.if_array() == nullptr);
            BOOST_TEST(arr.if_array() != nullptr);
            BOOST_TEST(str.if_array() == nullptr);
            BOOST_TEST(i64.if_array() == nullptr);
            BOOST_TEST(u64.if_array() == nullptr);
            BOOST_TEST(dub.if_array() == nullptr);
            BOOST_TEST(bln.if_array() == nullptr);
            BOOST_TEST(nul.if_array() == nullptr);
        }

        // if_array() const
        {
            BOOST_TEST(cobj.if_array() == nullptr);
            BOOST_TEST(carr.if_array() != nullptr);
            BOOST_TEST(cstr.if_array() == nullptr);
            BOOST_TEST(ci64.if_array() == nullptr);
            BOOST_TEST(cu64.if_array() == nullptr);
            BOOST_TEST(cdub.if_array() == nullptr);
            BOOST_TEST(cbln.if_array() == nullptr);
            BOOST_TEST(cnul.if_array() == nullptr);
        }

        // if_string()
        {
            BOOST_TEST(obj.if_string() == nullptr);
            BOOST_TEST(arr.if_string() == nullptr);
            BOOST_TEST(str.if_string() != nullptr);
            BOOST_TEST(i64.if_string() == nullptr);
            BOOST_TEST(u64.if_string() == nullptr);
            BOOST_TEST(dub.if_string() == nullptr);
            BOOST_TEST(bln.if_string() == nullptr);
            BOOST_TEST(nul.if_string() == nullptr);
        }

        // if_string() const
        {
            BOOST_TEST(cobj.if_string() == nullptr);
            BOOST_TEST(carr.if_string() == nullptr);
            BOOST_TEST(cstr.if_string() != nullptr);
            BOOST_TEST(ci64.if_string() == nullptr);
            BOOST_TEST(cu64.if_string() == nullptr);
            BOOST_TEST(cdub.if_string() == nullptr);
            BOOST_TEST(cbln.if_string() == nullptr);
            BOOST_TEST(cnul.if_string() == nullptr);
        }

        // if_int64()
        {
            BOOST_TEST(obj.if_int64() == nullptr);
            BOOST_TEST(arr.if_int64() == nullptr);
            BOOST_TEST(str.if_int64() == nullptr);
            BOOST_TEST(i64.if_int64() != nullptr);
            BOOST_TEST(u64.if_int64() == nullptr);
            BOOST_TEST(dub.if_int64() == nullptr);
            BOOST_TEST(bln.if_int64() == nullptr);
            BOOST_TEST(nul.if_int64() == nullptr);
        }

        // if_int64() const
        {
            BOOST_TEST(cobj.if_int64() == nullptr);
            BOOST_TEST(carr.if_int64() == nullptr);
            BOOST_TEST(cstr.if_int64() == nullptr);
            BOOST_TEST(ci64.if_int64() != nullptr);
            BOOST_TEST(cu64.if_int64() == nullptr);
            BOOST_TEST(cdub.if_int64() == nullptr);
            BOOST_TEST(cbln.if_int64() == nullptr);
            BOOST_TEST(cnul.if_int64() == nullptr);
        }

        // if_uint64()
        {
            BOOST_TEST(obj.if_uint64() == nullptr);
            BOOST_TEST(arr.if_uint64() == nullptr);
            BOOST_TEST(str.if_uint64() == nullptr);
            BOOST_TEST(i64.if_uint64() == nullptr);
            BOOST_TEST(u64.if_uint64() != nullptr);
            BOOST_TEST(dub.if_uint64() == nullptr);
            BOOST_TEST(bln.if_uint64() == nullptr);
            BOOST_TEST(nul.if_uint64() == nullptr);
        }

        // if_uint64() const
        {
            BOOST_TEST(cobj.if_uint64() == nullptr);
            BOOST_TEST(carr.if_uint64() == nullptr);
            BOOST_TEST(cstr.if_uint64() == nullptr);
            BOOST_TEST(ci64.if_uint64() == nullptr);
            BOOST_TEST(cu64.if_uint64() != nullptr);
            BOOST_TEST(cdub.if_uint64() == nullptr);
            BOOST_TEST(cbln.if_uint64() == nullptr);
            BOOST_TEST(cnul.if_uint64() == nullptr);
        }

        // if_double()
        {
            BOOST_TEST(obj.if_double() == nullptr);
            BOOST_TEST(arr.if_double() == nullptr);
            BOOST_TEST(str.if_double() == nullptr);
            BOOST_TEST(i64.if_double() == nullptr);
            BOOST_TEST(u64.if_double() == nullptr);
            BOOST_TEST(dub.if_double() != nullptr);
            BOOST_TEST(bln.if_double() == nullptr);
            BOOST_TEST(nul.if_double() == nullptr);
        }

        // if_double() const
        {
            BOOST_TEST(cobj.if_double() == nullptr);
            BOOST_TEST(carr.if_double() == nullptr);
            BOOST_TEST(cstr.if_double() == nullptr);
            BOOST_TEST(ci64.if_double() == nullptr);
            BOOST_TEST(cu64.if_double() == nullptr);
            BOOST_TEST(cdub.if_double() != nullptr);
            BOOST_TEST(cbln.if_double() == nullptr);
            BOOST_TEST(cnul.if_double() == nullptr);
        }

        // if_bool()
        {
            BOOST_TEST(obj.if_bool() == nullptr);
            BOOST_TEST(arr.if_bool() == nullptr);
            BOOST_TEST(str.if_bool() == nullptr);
            BOOST_TEST(i64.if_bool() == nullptr);
            BOOST_TEST(u64.if_bool() == nullptr);
            BOOST_TEST(dub.if_bool() == nullptr);
            BOOST_TEST(bln.if_bool() != nullptr);
            BOOST_TEST(nul.if_bool() == nullptr);
        }

        // if_bool() const
        {
            BOOST_TEST(cobj.if_bool() == nullptr);
            BOOST_TEST(carr.if_bool() == nullptr);
            BOOST_TEST(cstr.if_bool() == nullptr);
            BOOST_TEST(ci64.if_bool() == nullptr);
            BOOST_TEST(cu64.if_bool() == nullptr);
            BOOST_TEST(cdub.if_bool() == nullptr);
            BOOST_TEST(cbln.if_bool() != nullptr);
            BOOST_TEST(cnul.if_bool() == nullptr);
        }
    }

    void
    testAs()
    {
        value obj(object{});
        value arr(array{});
        value str(string{});
        value i64(std::int64_t{});
        value u64(std::uint64_t{});
        value dub(double{});
        value bln(true);
        value nul(nullptr);

        auto const& cobj(obj);
        auto const& carr(arr);
        auto const& cstr(str);
        auto const& ci64(i64);
        auto const& cu64(u64);
        auto const& cdub(dub);
        auto const& cbln(bln);
        auto const& cnul(nul);

        // as_object()
        {
             object& x = obj.as_object();
            BOOST_TEST_THROWS(arr.as_object(), system_error);
            BOOST_TEST_THROWS(str.as_object(), system_error);
            BOOST_TEST_THROWS(i64.as_object(), system_error);
            BOOST_TEST_THROWS(u64.as_object(), system_error);
            BOOST_TEST_THROWS(dub.as_object(), system_error);
            BOOST_TEST_THROWS(bln.as_object(), system_error);
            BOOST_TEST_THROWS(nul.as_object(), system_error);
            (void)x;
        }

        // as_object() const
        {
       object const& x = cobj.as_object();
            BOOST_TEST_THROWS(carr.as_object(), system_error);
            BOOST_TEST_THROWS(cstr.as_object(), system_error);
            BOOST_TEST_THROWS(ci64.as_object(), system_error);
            BOOST_TEST_THROWS(cu64.as_object(), system_error);
            BOOST_TEST_THROWS(cdub.as_object(), system_error);
            BOOST_TEST_THROWS(cbln.as_object(), system_error);
            BOOST_TEST_THROWS(cnul.as_object(), system_error);
            (void)x;
        }

        // as_array()
        {
            BOOST_TEST_THROWS(obj.as_array(), system_error);
              array& x = arr.as_array();
            BOOST_TEST_THROWS(str.as_array(), system_error);
            BOOST_TEST_THROWS(i64.as_array(), system_error);
            BOOST_TEST_THROWS(u64.as_array(), system_error);
            BOOST_TEST_THROWS(dub.as_array(), system_error);
            BOOST_TEST_THROWS(bln.as_array(), system_error);
            BOOST_TEST_THROWS(nul.as_array(), system_error);
            (void)x;
        }

        // as_array() const
        {
            BOOST_TEST_THROWS(cobj.as_array(), system_error);
        array const& x = carr.as_array();
            BOOST_TEST_THROWS(cstr.as_array(), system_error);
            BOOST_TEST_THROWS(ci64.as_array(), system_error);
            BOOST_TEST_THROWS(cu64.as_array(), system_error);
            BOOST_TEST_THROWS(cdub.as_array(), system_error);
            BOOST_TEST_THROWS(cbln.as_array(), system_error);
            BOOST_TEST_THROWS(cnul.as_array(), system_error);
            (void)x;
        }

        // as_string()
        {
            BOOST_TEST_THROWS(obj.as_string(), system_error);
            BOOST_TEST_THROWS(arr.as_string(), system_error);
             string& x = str.as_string();
            BOOST_TEST_THROWS(i64.as_string(), system_error);
            BOOST_TEST_THROWS(u64.as_string(), system_error);
            BOOST_TEST_THROWS(dub.as_string(), system_error);
            BOOST_TEST_THROWS(bln.as_string(), system_error);
            BOOST_TEST_THROWS(nul.as_string(), system_error);
            (void)x;
        }

        // as_string() const
        {
            BOOST_TEST_THROWS(cobj.as_string(), system_error);
            BOOST_TEST_THROWS(carr.as_string(), system_error);
       string const& x = cstr.as_string();
            BOOST_TEST_THROWS(ci64.as_string(), system_error);
            BOOST_TEST_THROWS(cu64.as_string(), system_error);
            BOOST_TEST_THROWS(cdub.as_string(), system_error);
            BOOST_TEST_THROWS(cbln.as_string(), system_error);
            BOOST_TEST_THROWS(cnul.as_string(), system_error);
            (void)x;
        }

        // as_int64()
        {
            BOOST_TEST_THROWS(obj.as_int64(), system_error);
            BOOST_TEST_THROWS(arr.as_int64(), system_error);
            BOOST_TEST_THROWS(str.as_int64(), system_error);
       std::int64_t& x = i64.as_int64();
            BOOST_TEST_THROWS(u64.as_int64(), system_error);
            BOOST_TEST_THROWS(dub.as_int64(), system_error);
            BOOST_TEST_THROWS(bln.as_int64(), system_error);
            BOOST_TEST_THROWS(nul.as_int64(), system_error);
            (void)x;
        }

        // as_int64() const
        {
            BOOST_TEST_THROWS(cobj.as_int64(), system_error);
            BOOST_TEST_THROWS(carr.as_int64(), system_error);
            BOOST_TEST_THROWS(cstr.as_int64(), system_error);
 std::int64_t const& x = ci64.as_int64();
            BOOST_TEST_THROWS(cu64.as_int64(), system_error);
            BOOST_TEST_THROWS(cdub.as_int64(), system_error);
            BOOST_TEST_THROWS(cbln.as_int64(), system_error);
            BOOST_TEST_THROWS(cnul.as_int64(), system_error);
            (void)x;
        }

        // as_uint64()
        {
            BOOST_TEST_THROWS(obj.as_uint64(), system_error);
            BOOST_TEST_THROWS(arr.as_uint64(), system_error);
            BOOST_TEST_THROWS(str.as_uint64(), system_error);
            BOOST_TEST_THROWS(i64.as_uint64(), system_error);
      std::uint64_t& x = u64.as_uint64();
            BOOST_TEST_THROWS(dub.as_uint64(), system_error);
            BOOST_TEST_THROWS(bln.as_uint64(), system_error);
            BOOST_TEST_THROWS(nul.as_uint64(), system_error);
            (void)x;
        }

        // as_uint64() const
        {
            BOOST_TEST_THROWS(cobj.as_uint64(), system_error);
            BOOST_TEST_THROWS(carr.as_uint64(), system_error);
            BOOST_TEST_THROWS(cstr.as_uint64(), system_error);
            BOOST_TEST_THROWS(ci64.as_uint64(), system_error);
std::uint64_t const& x = cu64.as_uint64();
            BOOST_TEST_THROWS(cdub.as_uint64(), system_error);
            BOOST_TEST_THROWS(cbln.as_uint64(), system_error);
            BOOST_TEST_THROWS(cnul.as_uint64(), system_error);
            (void)x;
        }

        // as_double()
        {
            BOOST_TEST_THROWS(obj.as_double(), system_error);
            BOOST_TEST_THROWS(arr.as_double(), system_error);
            BOOST_TEST_THROWS(str.as_double(), system_error);
            BOOST_TEST_THROWS(i64.as_double(), system_error);
            BOOST_TEST_THROWS(u64.as_double(), system_error);
             double& x = dub.as_double();
            BOOST_TEST_THROWS(bln.as_double(), system_error);
            BOOST_TEST_THROWS(nul.as_double(), system_error);
            (void)x;
        }

        // as_uint64() const
        {
            BOOST_TEST_THROWS(cobj.as_double(), system_error);
            BOOST_TEST_THROWS(carr.as_double(), system_error);
            BOOST_TEST_THROWS(cstr.as_double(), system_error);
            BOOST_TEST_THROWS(ci64.as_double(), system_error);
            BOOST_TEST_THROWS(cu64.as_double(), system_error);
       double const& x = cdub.as_double();
            BOOST_TEST_THROWS(cbln.as_double(), system_error);
            BOOST_TEST_THROWS(cnul.as_double(), system_error);
            (void)x;
        }

        // as_bool()
        {
            BOOST_TEST_THROWS(obj.as_bool(), system_error);
            BOOST_TEST_THROWS(arr.as_bool(), system_error);
            BOOST_TEST_THROWS(str.as_bool(), system_error);
            BOOST_TEST_THROWS(i64.as_bool(), system_error);
            BOOST_TEST_THROWS(u64.as_bool(), system_error);
            BOOST_TEST_THROWS(dub.as_bool(), system_error);
               bool& x = bln.as_bool();
            BOOST_TEST_THROWS(nul.as_bool(), system_error);
            (void)x;
        }

        // as_bool() const
        {
            BOOST_TEST_THROWS(cobj.as_bool(), system_error);
            BOOST_TEST_THROWS(carr.as_bool(), system_error);
            BOOST_TEST_THROWS(cstr.as_bool(), system_error);
            BOOST_TEST_THROWS(ci64.as_bool(), system_error);
            BOOST_TEST_THROWS(cu64.as_bool(), system_error);
            BOOST_TEST_THROWS(cdub.as_bool(), system_error);
          bool const&x = cbln.as_bool();
            BOOST_TEST_THROWS(cnul.as_bool(), system_error);
            (void)x;
        }
    }

    void
    testGet()
    {
        value obj(object{});
        value arr(array{});
        value str(string{});
        value i64(std::int64_t{});
        value u64(std::uint64_t{});
        value dub(double{});
        value bln(true);

        auto const& cobj(obj);
        auto const& carr(arr);
        auto const& cstr(str);
        auto const& ci64(i64);
        auto const& cu64(u64);
        auto const& cdub(dub);
        auto const& cbln(bln);

        // get_object()
        // get_array()
        // get_string()
        // get_int64()
        // get_uint64()
        // get_double()
        // get_bool()
        {
            object&         xobj = obj.get_object();
            array&          xarr = arr.get_array();
            string&         xstr = str.get_string();
            std::int64_t&   xi64 = i64.get_int64();
            std::uint64_t&  xu64 = u64.get_uint64();
            double&         xdub = dub.get_double();
            bool&           xbln = bln.get_bool();

            (void)(xobj);
            (void)(xarr);
            (void)(xstr);
            (void)(xi64);
            (void)(xu64);
            (void)(xdub);
            (void)(xbln);
        }

        // get_object() const
        // get_array() const
        // get_string() const
        // get_int64() const
        // get_uint64() const
        // get_double() const
        // get_bool() const
        {
            object const&   xobj = cobj.get_object();
            array const&    xarr = carr.get_array();
            string const&   xstr = cstr.get_string();
            std::int64_t
                const&      xi64 = ci64.get_int64();
            std::uint64_t
                const&      xu64 = cu64.get_uint64();
            double const&   xdub = cdub.get_double();
            bool const&     xbln = cbln.get_bool();

            (void)(xobj);
            (void)(xarr);
            (void)(xstr);
            (void)(xi64);
            (void)(xu64);
            (void)(xdub);
            (void)(xbln);
        }
    }

    void
    testCustomTypes()
    {
        using namespace value_test_ns;

        // to_json
        {
            T1 t;
            value jv(t);
        }
        {
            T2 t;
            value jv(t);
        }
        {
            T3 t;
            value jv(t);
        }
    }

    BOOST_STATIC_ASSERT(
        detail::is_range<std::vector<int>>::value);

    BOOST_STATIC_ASSERT(
        detail::is_range<std::initializer_list<int>>::value);

    //------------------------------------------------------

    void
    testKeyValuePair()
    {
        using kvp = key_value_pair;

        kvp v1("key", "value");
        kvp v2(v1);
        BOOST_TEST(v2.key() == "key");
        BOOST_TEST(
            v2.value().get_string() == "value");
        kvp v3(std::move(v2));
        BOOST_TEST(v3.key() == "key");
        BOOST_TEST(
            v3.value().get_string() == "value");
    }

    //------------------------------------------------------

    void
    run()
    {
        testSpecial();
        testConversion();
        testModifiers();
        testExchange();
        testObservers();
        testGetStorage();
        testIf();
        testAs();
        testGet();
        testCustomTypes();
        testKeyValuePair();
    }
};

TEST_SUITE(value_test, "boost.json.value");

} // json
} // boost
