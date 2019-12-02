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

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <memory>
#include <vector>

#include "test.hpp"

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

class value_test : public beast::unit_test::suite
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
            BEAST_EXPECT(*jv.storage() == *dsp);
        }

        // value(storage_ptr)
        {
            value jv(sp);
            BEAST_EXPECT(*jv.storage() == *sp);
        }

        // value(pilfered<value>)
        {
            {
                value jv1(object{});
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_null());
            }
        }

        // value(value const&)
        {
            {
                value jv1(object{});
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_null());
            }
        }

        // value(value const&, storage_ptr)
        {
            {
                value jv1(object{});
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_object());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
            {
                value jv1(array{});
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_array());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
            {
                value jv1(string{});
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_string());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
            {
                value jv1(std::int64_t{});
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_int64());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
            {
                value jv1(std::uint64_t{});
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_uint64());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
            {
                value jv1(double{});
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_double());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
            {
                value jv1(true);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_bool());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
            {
                value jv1(nullptr);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_null());
                BEAST_EXPECT(*jv2.storage() == *sp);
            }
        }

        // value(value&&)
        {
            {
                value jv1(object{});
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_null());
            }
        }

        // value(value&&, storage_ptr)
        {
            {
                value jv1(object{});
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_null());
            }
        }

        // operator=(value&&)
        {
            {
                value jv;
                jv = value(object{});
                BEAST_EXPECT(jv.is_object());
            }
            {
                value jv;
                jv = value(array{});
                BEAST_EXPECT(jv.is_array());
            }
            {
                value jv;
                jv = value(string{});
                BEAST_EXPECT(jv.is_string());
            }
            {
                value jv;
                jv = value(std::int64_t{});
                BEAST_EXPECT(jv.is_int64());
            }
            {
                value jv;
                jv = value(std::uint64_t{});
                BEAST_EXPECT(jv.is_uint64());
            }
            {
                value jv;
                jv = value(double{});
                BEAST_EXPECT(jv.is_double());
            }
            {
                value jv;
                jv = value(true);
                BEAST_EXPECT(jv.is_bool());
            }
            {
                value jv;
                jv = value(nullptr);
                BEAST_EXPECT(jv.is_null());
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
                BEAST_EXPECT(jv.as_array().size() == 8);
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
                BEAST_EXPECT(jv.as_object().size() == 8);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv(sp);
                jv = value(str_);
                BEAST_EXPECT(jv.as_string() == str_);
            });
        }

        // operator=(value const&)
        {
            {
                value jv1(object{});
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(array{});
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(string{});
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(std::int64_t{});
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_int64());
            }
            {
                value jv1(std::uint64_t{});
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_uint64());
            }
            {
                value jv1(double{});
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_double());
            }
            {
                value jv1(true);
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(nullptr);
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_null());
            }

            fail_loop([&](storage_ptr const sp)
            {
                value jv1({1, 2, 3, 4, 5});
                value jv2(sp);
                jv2 = jv1;
                BEAST_EXPECT(jv2.as_array().size() == 5);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv1({
                    {"a",1},{"b",2},{"c",3}});
                value jv2(sp);
                jv2 = jv1;
                BEAST_EXPECT(jv2.as_object().size() == 3);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv1(str_);
                value jv2(sp);
                jv2 = jv1;
                BEAST_EXPECT(jv2.as_string() == str_);
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
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(object(sp));
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
            {
                auto jv = value(object(), sp);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
            {
                auto jv = value(object_kind);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(object_kind, sp);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(array)
        // value(array, storage_ptr)
        // value(object_kind, storage_ptr)
        {
            {
                auto jv = value(array());
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(array(sp));
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
            {
                auto jv = value(array(), sp);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
            {
                auto jv = value(array_kind);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(array_kind, sp);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(string)
        // value(string, storage_ptr)
        {
            {
                auto jv = value(string());
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(string(sp));
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
            {
                auto jv = value(string(), sp);
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(int64)
        // value(int64, storage_ptr)
        {
            {
                auto jv = value(std::int64_t{-65536});
                BEAST_EXPECT(jv.is_int64());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(std::int64_t{-65536}, sp);
                BEAST_EXPECT(jv.is_int64());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(uint64)
        // value(uint64, storage_ptr)
        {
            {
                auto jv = value(std::uint64_t{65536});
                BEAST_EXPECT(jv.is_uint64());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(std::uint64_t{65536}, sp);
                BEAST_EXPECT(jv.is_uint64());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(double)
        // value(double, storage_ptr)
        {
            {
                auto jv = value(double{3.141});
                BEAST_EXPECT(jv.is_double());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(double{3.141}, sp);
                BEAST_EXPECT(jv.is_double());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(bool)
        // value(bool, storage_ptr)
        {
            {
                auto jv = value(true);
                BEAST_EXPECT(jv.is_bool());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(true, sp);
                BEAST_EXPECT(jv.is_bool());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(nullptr_t)
        // value(nullptr_t, storage_ptr)
        {
            {
                auto jv = value(nullptr);
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                auto jv = value(nullptr, sp);
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // value(initializer_list)
        // value(initializer_list, storage_ptr)
        {
            {
                value jv({1, 2, 3});
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                value jv({1, 2, 3}, sp);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
            {
                value jv({{"a",1},{"b",2},{"c",3}});
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                value jv({{"a",1},{"b",2},{"c",3}}, sp);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // operator=(object)
        {
            {
                value jv;
                jv = object();
                BEAST_EXPECT(*jv.storage() == *dsp);
                BEAST_EXPECT(jv.is_object());
            }
            {
                value jv(sp);
                jv = object();
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // operator=(array)
        {
            {
                value jv;
                jv = array();
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                value jv(sp);
                jv = array();
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // operator=(string)
        {
            {
                value jv;
                jv = string();
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(*jv.storage() == *dsp);
            }
            {
                value jv(sp);
                jv = string();
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(*jv.storage() == *sp);
            }
        }

        // operator=(int64)
        {
            value jv;
            jv = std::int64_t{-65536};
            BEAST_EXPECT(jv.is_int64());
        }

        // operator=(uint64)
        {
            value jv;
            jv = std::uint64_t{65536};
            BEAST_EXPECT(jv.is_uint64());
        }

        // operator=(double)
        {
            value jv;
            jv = double{3.141};
            BEAST_EXPECT(jv.is_double());
        }
    }

    void
    testModifiers()
    {
        // emplace
        {
            BEAST_EXPECT(value().emplace_object().empty());
            BEAST_EXPECT(value().emplace_array().empty());
            BEAST_EXPECT(value().emplace_string().empty());
            BEAST_EXPECT(value().emplace_int64() == 0);
            BEAST_EXPECT(value().emplace_uint64() == 0);
            BEAST_EXPECT(value().emplace_double() == 0);
            BEAST_EXPECT(value().emplace_bool() = true);
            {
                value jv(object_kind);
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
            {
                value jv(array_kind);
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
            {
                value jv(string_kind);
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
            {
                value jv(-1);
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
            {
                value jv(1U);
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
            {
                value jv(1.0);
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
            {
                value jv(false);
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
            {
                value jv;
                jv.emplace_null();
                BEAST_EXPECT(jv.is_null());
            }
        }

        // swap
        {
            {
                value jv1(1);
                value jv2("abc");
                BEAST_EXPECT(
                    *jv1.storage() == *jv2.storage());
                swap(jv1, jv2);
                BEAST_EXPECT(jv1.as_string() == "abc");
                BEAST_EXPECT(jv2.as_int64() == 1);
            }

            // different storage
            fail_loop([&](storage_ptr const& sp)
            {
                value jv1(1);
                value jv2("abc", sp);
                BEAST_EXPECT(
                    *jv1.storage() != *jv2.storage());
                swap(jv1, jv2);
                BEAST_EXPECT(jv1.as_string() == "abc");
                BEAST_EXPECT(jv2.as_int64() == 1);
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
            BEAST_EXPECT(value("abc").is_string());
            BEAST_EXPECT(value(std::string()).is_string());
            BEAST_EXPECT(value(string_view()).is_string());

            { value jv; BEAST_EXPECT((jv = "abc").is_string()); }
            { value jv; BEAST_EXPECT((jv = std::string()).is_string()); }
            { value jv; BEAST_EXPECT((jv = string_view()).is_string()); }
        }

        // numbers
        {
            BEAST_EXPECT(value(tt<int>{}).is_int64());
            BEAST_EXPECT(value(tt<short int>{}).is_int64());
            BEAST_EXPECT(value(tt<long int>{}).is_int64());
            BEAST_EXPECT(value(tt<long long int>{}).is_int64());
            BEAST_EXPECT(value(tt<unsigned int>{}).is_uint64());
            BEAST_EXPECT(value(tt<unsigned short int>{}).is_uint64());
            BEAST_EXPECT(value(tt<unsigned long int>{}).is_uint64());
            BEAST_EXPECT(value(tt<unsigned long long int>{}).is_uint64());
            BEAST_EXPECT(value(tt<float>{}).is_double());
            BEAST_EXPECT(value(tt<double>{}).is_double());
            BEAST_EXPECT(value(tt<long double>{}).is_double());

            { value jv; BEAST_EXPECT((jv = tt<int>{}).is_int64()); }
            { value jv; BEAST_EXPECT((jv = tt<short int>{}).is_int64()); }
            { value jv; BEAST_EXPECT((jv = tt<long int>{}).is_int64()); }
            { value jv; BEAST_EXPECT((jv = tt<long long int>{}).is_int64()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned int>{}).is_uint64()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned short int>{}).is_uint64()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned long int>{}).is_uint64()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned long long int>{}).is_uint64()); }
            { value jv; BEAST_EXPECT((jv = tt<float>{}).is_double()); }
            { value jv; BEAST_EXPECT((jv = tt<double>{}).is_double()); }
            { value jv; BEAST_EXPECT((jv = tt<long double>{}).is_double()); }
        }

        // bool
        {
            BEAST_EXPECT(value(true).is_bool());
            BEAST_EXPECT(value(false).is_bool());

            { value jv; BEAST_EXPECT((jv = true).is_bool()); }
            { value jv; BEAST_EXPECT((jv = false).is_bool()); }
        }

        // nullptr
        {
            BEAST_EXPECT(value(nullptr).is_null());

            { value jv("x"); BEAST_EXPECT((jv = nullptr).is_null()); }
        }

        // VFALCO TODO store()

        // is_key_value_pair()
        {
            BEAST_EXPECT(value().is_key_value_pair() == false);
            BEAST_EXPECT(value(array{}).is_key_value_pair() == false);
            BEAST_EXPECT(value({1, 2}).is_key_value_pair() == false);
            BEAST_EXPECT(value({"x", 2}).is_key_value_pair() == true);
        }

        // maybe_object()
        {
            // empty list can be an empty object
            BEAST_EXPECT(value::maybe_object({}));

            BEAST_EXPECT(! value::maybe_object({1}));
            BEAST_EXPECT(! value::maybe_object({1, 2}));
            BEAST_EXPECT(! value::maybe_object({"x", 1}));
            BEAST_EXPECT(! value::maybe_object({{1, 2}}));
            BEAST_EXPECT(! value::maybe_object({{1, 2}, {"y", 2}}));

            BEAST_EXPECT(value::maybe_object({{"x", 1}}));
            BEAST_EXPECT(value::maybe_object({{"x", 1}, {"y", 2}}));
        }
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
            BEAST_EXPECT(obj.kind() == kind::object);
            BEAST_EXPECT(arr.kind() == kind::array);
            BEAST_EXPECT(str.kind() == kind::string);
            BEAST_EXPECT(i64.kind() == kind::int64);
            BEAST_EXPECT(u64.kind() == kind::uint64);
            BEAST_EXPECT(dub.kind() == kind::double_);
            BEAST_EXPECT(bln.kind() == kind::bool_);
            BEAST_EXPECT(nul.kind() == kind::null);
        }

        // is_object()
        {
            BEAST_EXPECT(  obj.is_object());
            BEAST_EXPECT(! arr.is_object());
            BEAST_EXPECT(! str.is_object());
            BEAST_EXPECT(! i64.is_object());
            BEAST_EXPECT(! u64.is_object());
            BEAST_EXPECT(! dub.is_object());
            BEAST_EXPECT(! bln.is_object());
            BEAST_EXPECT(! nul.is_object());
        }

        // is_array()
        {
            BEAST_EXPECT(! obj.is_array());
            BEAST_EXPECT(  arr.is_array());
            BEAST_EXPECT(! str.is_array());
            BEAST_EXPECT(! i64.is_array());
            BEAST_EXPECT(! u64.is_array());
            BEAST_EXPECT(! dub.is_array());
            BEAST_EXPECT(! bln.is_array());
            BEAST_EXPECT(! nul.is_array());
        }

        // is_string()
        {
            BEAST_EXPECT(! obj.is_string());
            BEAST_EXPECT(! arr.is_string());
            BEAST_EXPECT(  str.is_string());
            BEAST_EXPECT(! i64.is_string());
            BEAST_EXPECT(! u64.is_string());
            BEAST_EXPECT(! dub.is_string());
            BEAST_EXPECT(! bln.is_string());
            BEAST_EXPECT(! nul.is_string());
        }

        // is_int64()
        {
            BEAST_EXPECT(! obj.is_int64());
            BEAST_EXPECT(! arr.is_int64());
            BEAST_EXPECT(! str.is_int64());
            BEAST_EXPECT(  i64.is_int64());
            BEAST_EXPECT(! u64.is_int64());
            BEAST_EXPECT(! dub.is_int64());
            BEAST_EXPECT(! bln.is_int64());
            BEAST_EXPECT(! nul.is_int64());
        }

        // is_uint64()
        {
            BEAST_EXPECT(! obj.is_uint64());
            BEAST_EXPECT(! arr.is_uint64());
            BEAST_EXPECT(! str.is_uint64());
            BEAST_EXPECT(! i64.is_uint64());
            BEAST_EXPECT(  u64.is_uint64());
            BEAST_EXPECT(! dub.is_uint64());
            BEAST_EXPECT(! bln.is_uint64());
            BEAST_EXPECT(! nul.is_uint64());
        }

        // is_double()
        {
            BEAST_EXPECT(! obj.is_double());
            BEAST_EXPECT(! arr.is_double());
            BEAST_EXPECT(! str.is_double());
            BEAST_EXPECT(! i64.is_double());
            BEAST_EXPECT(! u64.is_double());
            BEAST_EXPECT(  dub.is_double());
            BEAST_EXPECT(! bln.is_double());
            BEAST_EXPECT(! nul.is_double());
        }

        // is_bool()
        {
            BEAST_EXPECT(! obj.is_bool());
            BEAST_EXPECT(! arr.is_bool());
            BEAST_EXPECT(! str.is_bool());
            BEAST_EXPECT(! i64.is_bool());
            BEAST_EXPECT(! u64.is_bool());
            BEAST_EXPECT(! dub.is_bool());
            BEAST_EXPECT(  bln.is_bool());
            BEAST_EXPECT(! nul.is_bool());
        }

        // is_null()
        {
            BEAST_EXPECT(! obj.is_null());
            BEAST_EXPECT(! arr.is_null());
            BEAST_EXPECT(! str.is_null());
            BEAST_EXPECT(! i64.is_null());
            BEAST_EXPECT(! u64.is_null());
            BEAST_EXPECT(! dub.is_null());
            BEAST_EXPECT(! bln.is_null());
            BEAST_EXPECT(  nul.is_null());
        }

        // is_number()
        {
            BEAST_EXPECT(! obj.is_number());
            BEAST_EXPECT(! arr.is_number());
            BEAST_EXPECT(! str.is_number());
            BEAST_EXPECT(  i64.is_number());
            BEAST_EXPECT(  u64.is_number());
            BEAST_EXPECT(  dub.is_number());
            BEAST_EXPECT(! bln.is_number());
            BEAST_EXPECT(! nul.is_number());
        }

        // is_primitive()
        {
            BEAST_EXPECT(! obj.is_primitive());
            BEAST_EXPECT(! arr.is_primitive());
            BEAST_EXPECT(  str.is_primitive());
            BEAST_EXPECT(  i64.is_primitive());
            BEAST_EXPECT(  u64.is_primitive());
            BEAST_EXPECT(  dub.is_primitive());
            BEAST_EXPECT(  bln.is_primitive());
            BEAST_EXPECT(  nul.is_primitive());
        }

        // is_structured()
        {
            BEAST_EXPECT(  obj.is_structured());
            BEAST_EXPECT(  arr.is_structured());
            BEAST_EXPECT(! str.is_structured());
            BEAST_EXPECT(! i64.is_structured());
            BEAST_EXPECT(! u64.is_structured());
            BEAST_EXPECT(! dub.is_structured());
            BEAST_EXPECT(! bln.is_structured());
            BEAST_EXPECT(! nul.is_structured());
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
            BEAST_EXPECT(*obj.storage() == *sp);
            BEAST_EXPECT(*arr.storage() == *sp);
            BEAST_EXPECT(*str.storage() == *sp);
            BEAST_EXPECT(*i64.storage() == *sp);
            BEAST_EXPECT(*u64.storage() == *sp);
            BEAST_EXPECT(*dub.storage() == *sp);
            BEAST_EXPECT(*bln.storage() == *sp);
            BEAST_EXPECT(*nul.storage() == *sp);
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
            BEAST_EXPECT(obj.if_object() != nullptr);
            BEAST_EXPECT(arr.if_object() == nullptr);
            BEAST_EXPECT(str.if_object() == nullptr);
            BEAST_EXPECT(i64.if_object() == nullptr);
            BEAST_EXPECT(u64.if_object() == nullptr);
            BEAST_EXPECT(dub.if_object() == nullptr);
            BEAST_EXPECT(bln.if_object() == nullptr);
            BEAST_EXPECT(nul.if_object() == nullptr);
        }

        // if_object() const
        {
            BEAST_EXPECT(cobj.if_object() != nullptr);
            BEAST_EXPECT(carr.if_object() == nullptr);
            BEAST_EXPECT(cstr.if_object() == nullptr);
            BEAST_EXPECT(ci64.if_object() == nullptr);
            BEAST_EXPECT(cu64.if_object() == nullptr);
            BEAST_EXPECT(cdub.if_object() == nullptr);
            BEAST_EXPECT(cbln.if_object() == nullptr);
            BEAST_EXPECT(cnul.if_object() == nullptr);
        }

        // if_array()
        {
            BEAST_EXPECT(obj.if_array() == nullptr);
            BEAST_EXPECT(arr.if_array() != nullptr);
            BEAST_EXPECT(str.if_array() == nullptr);
            BEAST_EXPECT(i64.if_array() == nullptr);
            BEAST_EXPECT(u64.if_array() == nullptr);
            BEAST_EXPECT(dub.if_array() == nullptr);
            BEAST_EXPECT(bln.if_array() == nullptr);
            BEAST_EXPECT(nul.if_array() == nullptr);
        }

        // if_array() const
        {
            BEAST_EXPECT(cobj.if_array() == nullptr);
            BEAST_EXPECT(carr.if_array() != nullptr);
            BEAST_EXPECT(cstr.if_array() == nullptr);
            BEAST_EXPECT(ci64.if_array() == nullptr);
            BEAST_EXPECT(cu64.if_array() == nullptr);
            BEAST_EXPECT(cdub.if_array() == nullptr);
            BEAST_EXPECT(cbln.if_array() == nullptr);
            BEAST_EXPECT(cnul.if_array() == nullptr);
        }

        // if_string()
        {
            BEAST_EXPECT(obj.if_string() == nullptr);
            BEAST_EXPECT(arr.if_string() == nullptr);
            BEAST_EXPECT(str.if_string() != nullptr);
            BEAST_EXPECT(i64.if_string() == nullptr);
            BEAST_EXPECT(u64.if_string() == nullptr);
            BEAST_EXPECT(dub.if_string() == nullptr);
            BEAST_EXPECT(bln.if_string() == nullptr);
            BEAST_EXPECT(nul.if_string() == nullptr);
        }

        // if_string() const
        {
            BEAST_EXPECT(cobj.if_string() == nullptr);
            BEAST_EXPECT(carr.if_string() == nullptr);
            BEAST_EXPECT(cstr.if_string() != nullptr);
            BEAST_EXPECT(ci64.if_string() == nullptr);
            BEAST_EXPECT(cu64.if_string() == nullptr);
            BEAST_EXPECT(cdub.if_string() == nullptr);
            BEAST_EXPECT(cbln.if_string() == nullptr);
            BEAST_EXPECT(cnul.if_string() == nullptr);
        }

        // if_int64()
        {
            BEAST_EXPECT(obj.if_int64() == nullptr);
            BEAST_EXPECT(arr.if_int64() == nullptr);
            BEAST_EXPECT(str.if_int64() == nullptr);
            BEAST_EXPECT(i64.if_int64() != nullptr);
            BEAST_EXPECT(u64.if_int64() == nullptr);
            BEAST_EXPECT(dub.if_int64() == nullptr);
            BEAST_EXPECT(bln.if_int64() == nullptr);
            BEAST_EXPECT(nul.if_int64() == nullptr);
        }

        // if_int64() const
        {
            BEAST_EXPECT(cobj.if_int64() == nullptr);
            BEAST_EXPECT(carr.if_int64() == nullptr);
            BEAST_EXPECT(cstr.if_int64() == nullptr);
            BEAST_EXPECT(ci64.if_int64() != nullptr);
            BEAST_EXPECT(cu64.if_int64() == nullptr);
            BEAST_EXPECT(cdub.if_int64() == nullptr);
            BEAST_EXPECT(cbln.if_int64() == nullptr);
            BEAST_EXPECT(cnul.if_int64() == nullptr);
        }

        // if_uint64()
        {
            BEAST_EXPECT(obj.if_uint64() == nullptr);
            BEAST_EXPECT(arr.if_uint64() == nullptr);
            BEAST_EXPECT(str.if_uint64() == nullptr);
            BEAST_EXPECT(i64.if_uint64() == nullptr);
            BEAST_EXPECT(u64.if_uint64() != nullptr);
            BEAST_EXPECT(dub.if_uint64() == nullptr);
            BEAST_EXPECT(bln.if_uint64() == nullptr);
            BEAST_EXPECT(nul.if_uint64() == nullptr);
        }

        // if_uint64() const
        {
            BEAST_EXPECT(cobj.if_uint64() == nullptr);
            BEAST_EXPECT(carr.if_uint64() == nullptr);
            BEAST_EXPECT(cstr.if_uint64() == nullptr);
            BEAST_EXPECT(ci64.if_uint64() == nullptr);
            BEAST_EXPECT(cu64.if_uint64() != nullptr);
            BEAST_EXPECT(cdub.if_uint64() == nullptr);
            BEAST_EXPECT(cbln.if_uint64() == nullptr);
            BEAST_EXPECT(cnul.if_uint64() == nullptr);
        }

        // if_double()
        {
            BEAST_EXPECT(obj.if_double() == nullptr);
            BEAST_EXPECT(arr.if_double() == nullptr);
            BEAST_EXPECT(str.if_double() == nullptr);
            BEAST_EXPECT(i64.if_double() == nullptr);
            BEAST_EXPECT(u64.if_double() == nullptr);
            BEAST_EXPECT(dub.if_double() != nullptr);
            BEAST_EXPECT(bln.if_double() == nullptr);
            BEAST_EXPECT(nul.if_double() == nullptr);
        }

        // if_double() const
        {
            BEAST_EXPECT(cobj.if_double() == nullptr);
            BEAST_EXPECT(carr.if_double() == nullptr);
            BEAST_EXPECT(cstr.if_double() == nullptr);
            BEAST_EXPECT(ci64.if_double() == nullptr);
            BEAST_EXPECT(cu64.if_double() == nullptr);
            BEAST_EXPECT(cdub.if_double() != nullptr);
            BEAST_EXPECT(cbln.if_double() == nullptr);
            BEAST_EXPECT(cnul.if_double() == nullptr);
        }

        // if_bool()
        {
            BEAST_EXPECT(obj.if_bool() == nullptr);
            BEAST_EXPECT(arr.if_bool() == nullptr);
            BEAST_EXPECT(str.if_bool() == nullptr);
            BEAST_EXPECT(i64.if_bool() == nullptr);
            BEAST_EXPECT(u64.if_bool() == nullptr);
            BEAST_EXPECT(dub.if_bool() == nullptr);
            BEAST_EXPECT(bln.if_bool() != nullptr);
            BEAST_EXPECT(nul.if_bool() == nullptr);
        }

        // if_bool() const
        {
            BEAST_EXPECT(cobj.if_bool() == nullptr);
            BEAST_EXPECT(carr.if_bool() == nullptr);
            BEAST_EXPECT(cstr.if_bool() == nullptr);
            BEAST_EXPECT(ci64.if_bool() == nullptr);
            BEAST_EXPECT(cu64.if_bool() == nullptr);
            BEAST_EXPECT(cdub.if_bool() == nullptr);
            BEAST_EXPECT(cbln.if_bool() != nullptr);
            BEAST_EXPECT(cnul.if_bool() == nullptr);
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
            BEAST_THROWS(arr.as_object(), system_error);
            BEAST_THROWS(str.as_object(), system_error);
            BEAST_THROWS(i64.as_object(), system_error);
            BEAST_THROWS(u64.as_object(), system_error);
            BEAST_THROWS(dub.as_object(), system_error);
            BEAST_THROWS(bln.as_object(), system_error);
            BEAST_THROWS(nul.as_object(), system_error);
            (void)x;
        }

        // as_object() const
        {
       object const& x = cobj.as_object();
            BEAST_THROWS(carr.as_object(), system_error);
            BEAST_THROWS(cstr.as_object(), system_error);
            BEAST_THROWS(ci64.as_object(), system_error);
            BEAST_THROWS(cu64.as_object(), system_error);
            BEAST_THROWS(cdub.as_object(), system_error);
            BEAST_THROWS(cbln.as_object(), system_error);
            BEAST_THROWS(cnul.as_object(), system_error);
            (void)x;
        }

        // as_array()
        {
            BEAST_THROWS(obj.as_array(), system_error);
              array& x = arr.as_array();
            BEAST_THROWS(str.as_array(), system_error);
            BEAST_THROWS(i64.as_array(), system_error);
            BEAST_THROWS(u64.as_array(), system_error);
            BEAST_THROWS(dub.as_array(), system_error);
            BEAST_THROWS(bln.as_array(), system_error);
            BEAST_THROWS(nul.as_array(), system_error);
            (void)x;
        }

        // as_array() const
        {
            BEAST_THROWS(cobj.as_array(), system_error);
        array const& x = carr.as_array();
            BEAST_THROWS(cstr.as_array(), system_error);
            BEAST_THROWS(ci64.as_array(), system_error);
            BEAST_THROWS(cu64.as_array(), system_error);
            BEAST_THROWS(cdub.as_array(), system_error);
            BEAST_THROWS(cbln.as_array(), system_error);
            BEAST_THROWS(cnul.as_array(), system_error);
            (void)x;
        }

        // as_string()
        {
            BEAST_THROWS(obj.as_string(), system_error);
            BEAST_THROWS(arr.as_string(), system_error);
             string& x = str.as_string();
            BEAST_THROWS(i64.as_string(), system_error);
            BEAST_THROWS(u64.as_string(), system_error);
            BEAST_THROWS(dub.as_string(), system_error);
            BEAST_THROWS(bln.as_string(), system_error);
            BEAST_THROWS(nul.as_string(), system_error);
            (void)x;
        }

        // as_string() const
        {
            BEAST_THROWS(cobj.as_string(), system_error);
            BEAST_THROWS(carr.as_string(), system_error);
       string const& x = cstr.as_string();
            BEAST_THROWS(ci64.as_string(), system_error);
            BEAST_THROWS(cu64.as_string(), system_error);
            BEAST_THROWS(cdub.as_string(), system_error);
            BEAST_THROWS(cbln.as_string(), system_error);
            BEAST_THROWS(cnul.as_string(), system_error);
            (void)x;
        }

        // as_int64()
        {
            BEAST_THROWS(obj.as_int64(), system_error);
            BEAST_THROWS(arr.as_int64(), system_error);
            BEAST_THROWS(str.as_int64(), system_error);
       std::int64_t& x = i64.as_int64();
            BEAST_THROWS(u64.as_int64(), system_error);
            BEAST_THROWS(dub.as_int64(), system_error);
            BEAST_THROWS(bln.as_int64(), system_error);
            BEAST_THROWS(nul.as_int64(), system_error);
            (void)x;
        }

        // as_int64() const
        {
            BEAST_THROWS(cobj.as_int64(), system_error);
            BEAST_THROWS(carr.as_int64(), system_error);
            BEAST_THROWS(cstr.as_int64(), system_error);
 std::int64_t const& x = ci64.as_int64();
            BEAST_THROWS(cu64.as_int64(), system_error);
            BEAST_THROWS(cdub.as_int64(), system_error);
            BEAST_THROWS(cbln.as_int64(), system_error);
            BEAST_THROWS(cnul.as_int64(), system_error);
            (void)x;
        }

        // as_uint64()
        {
            BEAST_THROWS(obj.as_uint64(), system_error);
            BEAST_THROWS(arr.as_uint64(), system_error);
            BEAST_THROWS(str.as_uint64(), system_error);
            BEAST_THROWS(i64.as_uint64(), system_error);
      std::uint64_t& x = u64.as_uint64();
            BEAST_THROWS(dub.as_uint64(), system_error);
            BEAST_THROWS(bln.as_uint64(), system_error);
            BEAST_THROWS(nul.as_uint64(), system_error);
            (void)x;
        }

        // as_uint64() const
        {
            BEAST_THROWS(cobj.as_uint64(), system_error);
            BEAST_THROWS(carr.as_uint64(), system_error);
            BEAST_THROWS(cstr.as_uint64(), system_error);
            BEAST_THROWS(ci64.as_uint64(), system_error);
std::uint64_t const& x = cu64.as_uint64();
            BEAST_THROWS(cdub.as_uint64(), system_error);
            BEAST_THROWS(cbln.as_uint64(), system_error);
            BEAST_THROWS(cnul.as_uint64(), system_error);
            (void)x;
        }

        // as_double()
        {
            BEAST_THROWS(obj.as_double(), system_error);
            BEAST_THROWS(arr.as_double(), system_error);
            BEAST_THROWS(str.as_double(), system_error);
            BEAST_THROWS(i64.as_double(), system_error);
            BEAST_THROWS(u64.as_double(), system_error);
             double& x = dub.as_double();
            BEAST_THROWS(bln.as_double(), system_error);
            BEAST_THROWS(nul.as_double(), system_error);
            (void)x;
        }

        // as_uint64() const
        {
            BEAST_THROWS(cobj.as_double(), system_error);
            BEAST_THROWS(carr.as_double(), system_error);
            BEAST_THROWS(cstr.as_double(), system_error);
            BEAST_THROWS(ci64.as_double(), system_error);
            BEAST_THROWS(cu64.as_double(), system_error);
       double const& x = cdub.as_double();
            BEAST_THROWS(cbln.as_double(), system_error);
            BEAST_THROWS(cnul.as_double(), system_error);
            (void)x;
        }

        // as_bool()
        {
            BEAST_THROWS(obj.as_bool(), system_error);
            BEAST_THROWS(arr.as_bool(), system_error);
            BEAST_THROWS(str.as_bool(), system_error);
            BEAST_THROWS(i64.as_bool(), system_error);
            BEAST_THROWS(u64.as_bool(), system_error);
            BEAST_THROWS(dub.as_bool(), system_error);
               bool& x = bln.as_bool();
            BEAST_THROWS(nul.as_bool(), system_error);
            (void)x;
        }

        // as_bool() const
        {
            BEAST_THROWS(cobj.as_bool(), system_error);
            BEAST_THROWS(carr.as_bool(), system_error);
            BEAST_THROWS(cstr.as_bool(), system_error);
            BEAST_THROWS(ci64.as_bool(), system_error);
            BEAST_THROWS(cu64.as_bool(), system_error);
            BEAST_THROWS(cdub.as_bool(), system_error);
          bool const&x = cbln.as_bool();
            BEAST_THROWS(cnul.as_bool(), system_error);
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
        BEAST_EXPECT(v2.key() == "key");
        BEAST_EXPECT(
            v2.value().get_string() == "value");
        kvp v3(std::move(v2));
        BEAST_EXPECT(v3.key() == "key");
        BEAST_EXPECT(
            v3.value().get_string() == "value");
    }

    //------------------------------------------------------

    void
    run() override
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

BEAST_DEFINE_TESTSUITE(boost,json,value);

} // json
} // boost
