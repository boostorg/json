//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/value.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/static_assert.hpp>
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
    testConstruction()
    {
        auto dsp = default_storage();
        auto sp = make_storage<unique_storage>();

        // ~value()
        {
            value{kind::object};
            value{kind::array};
            value{kind::string};
            value{kind::number};
            value{kind::boolean};
            value{kind::null};
        }

        // value()
        {
            value jv;
            BEAST_EXPECT(*jv.get_storage() == *dsp);
        }

        // value(storage_ptr)
        {
            value jv(sp);
            BEAST_EXPECT(*jv.get_storage() == *sp);
        }

        // value(kind)
        {
            BEAST_EXPECT(value(kind::object).is_object());
            BEAST_EXPECT(value(kind::array).is_array());
            BEAST_EXPECT(value(kind::string).is_string());
            BEAST_EXPECT(value(kind::number).is_number());
            BEAST_EXPECT(value(kind::boolean).is_bool());
            BEAST_EXPECT(value(kind::null).is_null());
        }

        // value(kind, storage_ptr)
        {
            BEAST_EXPECT((*value(kind::object, sp).get_storage() == *sp));
            BEAST_EXPECT((*value(kind::array, sp).get_storage() == *sp));
            BEAST_EXPECT((*value(kind::string, sp).get_storage() == *sp));
            BEAST_EXPECT((*value(kind::number, sp).get_storage() == *sp));
            BEAST_EXPECT((*value(kind::boolean, sp).get_storage() == *sp));
            BEAST_EXPECT((*value(kind::null, sp).get_storage() == *sp));
        }

        // value(value const&)
        {
            {
                value jv1(kind::object);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(kind::array);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(kind::string);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(kind::number);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_number());
            }
            {
                value jv1(kind::boolean);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(kind::null);
                value jv2(jv1);
                BEAST_EXPECT(jv2.is_null());
            }
        }

        // value(value const&, storage_ptr)
        {
            {
                value jv1(kind::object);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_object());
                BEAST_EXPECT(*jv2.get_storage() == *sp);
            }
            {
                value jv1(kind::array);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_array());
                BEAST_EXPECT(*jv2.get_storage() == *sp);
            }
            {
                value jv1(kind::string);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_string());
                BEAST_EXPECT(*jv2.get_storage() == *sp);
            }
            {
                value jv1(kind::number);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_number());
                BEAST_EXPECT(*jv2.get_storage() == *sp);
            }
            {
                value jv1(kind::boolean);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_bool());
                BEAST_EXPECT(*jv2.get_storage() == *sp);
            }
            {
                value jv1(kind::null);
                value jv2(jv1, sp);
                BEAST_EXPECT(jv2.is_null());
                BEAST_EXPECT(*jv2.get_storage() == *sp);
            }
        }

        // value(pilfered<value>)
        {
            {
                value jv1(kind::object);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(kind::array);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(kind::string);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(kind::number);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_number());
            }
            {
                value jv1(kind::boolean);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(kind::null);
                value jv2(pilfer(jv1));
                BEAST_EXPECT(jv2.is_null());
            }
        }

        // value(value&&)
        {
            {
                value jv1(kind::object);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(kind::array);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(kind::string);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(kind::number);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_number());
            }
            {
                value jv1(kind::boolean);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(kind::null);
                value jv2(std::move(jv1));
                BEAST_EXPECT(jv2.is_null());
            }
        }

        // value(value&&, storage_ptr)
        {
            {
                value jv1(kind::object);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(kind::array);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(kind::string);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(kind::number);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_number());
            }
            {
                value jv1(kind::boolean);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(kind::null);
                value jv2(std::move(jv1), sp);
                BEAST_EXPECT(jv2.is_null());
            }

            // self-assign
            {
                value jv(kind::object);
                jv = *(&jv);
            }
        }

        // operator=(value&&)
        {
            {
                value jv;
                jv = value(kind::object);
                BEAST_EXPECT(jv.is_object());
            }
            {
                value jv;
                jv = value(kind::array);
                BEAST_EXPECT(jv.is_array());
            }
            {
                value jv;
                jv = value(kind::string);
                BEAST_EXPECT(jv.is_string());
            }
            {
                value jv;
                jv = value(kind::number);
                BEAST_EXPECT(jv.is_number());
            }
            {
                value jv;
                jv = value(kind::boolean);
                BEAST_EXPECT(jv.is_bool());
            }
            {
                value jv;
                jv = value(kind::null);
                BEAST_EXPECT(jv.is_null());
            }

            fail_loop([&](storage_ptr const sp)
            {
                value jv(sp);
                jv = value({1, 2, 3, 4, 5});
                BEAST_EXPECT(jv.as_array().size() == 5);
            });

            fail_loop([&](storage_ptr const sp)
            {
                value jv(sp);
                jv = value({
                    {"a",1},{"b",2},{"c",3}});
                BEAST_EXPECT(jv.as_object().size() == 3);
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
                value jv1(kind::object);
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_object());
            }
            {
                value jv1(kind::array);
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_array());
            }
            {
                value jv1(kind::string);
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_string());
            }
            {
                value jv1(kind::number);
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_number());
            }
            {
                value jv1(kind::boolean);
                value jv2;
                jv2 = jv1;
                BEAST_EXPECT(jv2.is_bool());
            }
            {
                value jv1(kind::null);
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
        }
    }

    void
    testConversion()
    {
        auto dsp = default_storage();
        auto sp = make_storage<unique_storage>();

        // value(object)
        // value(object, storage_ptr)
        {
            {
                auto jv = value(object());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                auto jv = value(object(sp));
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
            {
                auto jv = value(object(), sp);
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // value(array)
        // value(array, storage_ptr)
        {
            {
                auto jv = value(array());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                auto jv = value(array(sp));
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
            {
                auto jv = value(array(), sp);
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // value(string)
        // value(string, storage_ptr)
        {
            {
                auto jv = value(string());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                auto jv = value(string(sp));
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
            {
                auto jv = value(string(), sp);
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // value(number)
        // value(number, storage_ptr)
        {
            {
                auto jv = value(number());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                auto jv = value(number(), sp);
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // value(initializer_list)
        // value(initializer_list, storage_ptr)
        {
            {
                value jv({1, 2, 3});
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                value jv({1, 2, 3}, sp);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
            {
                value jv({{"a",1},{"b",2},{"c",3}});
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                value jv({{"a",1},{"b",2},{"c",3}}, sp);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // operator=(object)
        {
            {
                value jv;
                jv = object();
                BEAST_EXPECT(*jv.get_storage() == *dsp);
                BEAST_EXPECT(jv.is_object());
            }
            {
                value jv(sp);
                jv = object();
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // operator=(array)
        {
            {
                value jv;
                jv = array();
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                value jv(sp);
                jv = array();
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // operator=(string)
        {
            {
                value jv;
                jv = string();
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                value jv(sp);
                jv = string();
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }

        // operator=(number)
        {
            {
                value jv;
                jv = number();
                BEAST_EXPECT(jv.is_number());
                BEAST_EXPECT(*jv.get_storage() == *dsp);
            }
            {
                value jv(sp);
                jv = number();
                BEAST_EXPECT(jv.is_number());
                BEAST_EXPECT(*jv.get_storage() == *sp);
            }
        }
    }

    void
    testModifiers()
    {
        // reset
        {
            BEAST_EXPECT(value().reset(kind::object).is_object());
            BEAST_EXPECT(value().reset(kind::array).is_array());
            BEAST_EXPECT(value().reset(kind::string).is_string());
            BEAST_EXPECT(value().reset(kind::number).is_number());
            BEAST_EXPECT(value().reset(kind::boolean).is_bool());
            BEAST_EXPECT(value().reset(kind::null).is_null());
        }

        // emplace
        {
            BEAST_EXPECT(value().emplace_object().empty());
            BEAST_EXPECT(value().emplace_array().empty());
            BEAST_EXPECT(value().emplace_string().empty());
            BEAST_EXPECT(value().emplace_number().get_int64() == 0);
            BEAST_EXPECT(value().emplace_bool() = true);
            {
                value jv(kind::string);
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
                    *jv1.get_storage() == *jv2.get_storage());
                swap(jv1, jv2);
                BEAST_EXPECT(jv1.as_string() == "abc");
                BEAST_EXPECT(jv2.as_number() == 1);
            }

            // different storage
            fail_loop([&](storage_ptr const& sp)
            {
                value jv1(1);
                value jv2("abc", sp);
                BEAST_EXPECT(
                    *jv1.get_storage() != *jv2.get_storage());
                swap(jv1, jv2);
                BEAST_EXPECT(jv1.as_string() == "abc");
                BEAST_EXPECT(jv2.as_number() == 1);
            });
        }
    }

    template<typename T>
    using tt = T;

    void
    testExchange()
    {
        // kind::string
        {
            BEAST_EXPECT(value("abc").is_string());
            BEAST_EXPECT(value(std::string()).is_string());
            BEAST_EXPECT(value(string_view()).is_string());

            { value jv; BEAST_EXPECT((jv = "abc").is_string()); }
            { value jv; BEAST_EXPECT((jv = std::string()).is_string()); }
            { value jv; BEAST_EXPECT((jv = string_view()).is_string()); }
        }

        // kind::number
        {
            // VFALCO I'm not sure these should be numbers
            BEAST_EXPECT(value(tt<char>{}).is_number());
            BEAST_EXPECT(value(tt<unsigned char>{}).is_number());
            BEAST_EXPECT(value(tt<wchar_t>{}).is_number());

            { value jv; BEAST_EXPECT((jv = tt<char>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned char>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<wchar_t>{}).is_number()); }
        }

        // kind::number
        {
            BEAST_EXPECT(value(tt<int>{}).is_number());
            BEAST_EXPECT(value(tt<short int>{}).is_number());
            BEAST_EXPECT(value(tt<long int>{}).is_number());
            BEAST_EXPECT(value(tt<long long int>{}).is_number());
            BEAST_EXPECT(value(tt<unsigned int>{}).is_number());
            BEAST_EXPECT(value(tt<unsigned short int>{}).is_number());
            BEAST_EXPECT(value(tt<unsigned long int>{}).is_number());
            BEAST_EXPECT(value(tt<unsigned long long int>{}).is_number());
            BEAST_EXPECT(value(tt<float>{}).is_number());
            BEAST_EXPECT(value(tt<double>{}).is_number());
            BEAST_EXPECT(value(tt<long double>{}).is_number());

            { value jv; BEAST_EXPECT((jv = tt<int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<short int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<long int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<long long int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned short int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned long int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<unsigned long long int>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<float>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<double>{}).is_number()); }
            { value jv; BEAST_EXPECT((jv = tt<long double>{}).is_number()); }
        }

        // kind::boolean
        {
            BEAST_EXPECT(value(true).is_bool());
            BEAST_EXPECT(value(false).is_bool());

            { value jv; BEAST_EXPECT((jv = true).is_bool()); }
            { value jv; BEAST_EXPECT((jv = false).is_bool()); }
        }

        // kind::null
        {
            BEAST_EXPECT(value(nullptr).is_null());

            { value jv("x"); BEAST_EXPECT((jv = nullptr).is_null()); }
        }

        // VFALCO TODO store()

        // is_key_value_pair()
        {
            BEAST_EXPECT(value().is_key_value_pair() == false);
            BEAST_EXPECT(value(kind::array).is_key_value_pair() == false);
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
        value const obj(kind::object);
        value const arr(kind::array);
        value const str(kind::string);
        value const num(kind::number);
        value const boo(kind::boolean);
        value const nul(kind::null);

        // kind()
        {
            BEAST_EXPECT(obj.kind() == kind::object);
            BEAST_EXPECT(arr.kind() == kind::array);
            BEAST_EXPECT(str.kind() == kind::string);
            BEAST_EXPECT(num.kind() == kind::number);
            BEAST_EXPECT(boo.kind() == kind::boolean);
            BEAST_EXPECT(nul.kind() == kind::null);
        }

        // is_object()
        {
            BEAST_EXPECT(  obj.is_object());
            BEAST_EXPECT(! arr.is_object());
            BEAST_EXPECT(! str.is_object());
            BEAST_EXPECT(! num.is_object());
            BEAST_EXPECT(! boo.is_object());
            BEAST_EXPECT(! nul.is_object());
        }

        // is_array()
        {
            BEAST_EXPECT(! obj.is_array());
            BEAST_EXPECT(  arr.is_array());
            BEAST_EXPECT(! str.is_array());
            BEAST_EXPECT(! num.is_array());
            BEAST_EXPECT(! boo.is_array());
            BEAST_EXPECT(! nul.is_array());
        }

        // is_string()
        {
            BEAST_EXPECT(! obj.is_string());
            BEAST_EXPECT(! arr.is_string());
            BEAST_EXPECT(  str.is_string());
            BEAST_EXPECT(! num.is_string());
            BEAST_EXPECT(! boo.is_string());
            BEAST_EXPECT(! nul.is_string());
        }

        // is_number()
        {
            BEAST_EXPECT(! obj.is_number());
            BEAST_EXPECT(! arr.is_number());
            BEAST_EXPECT(! str.is_number());
            BEAST_EXPECT(  num.is_number());
            BEAST_EXPECT(! boo.is_number());
            BEAST_EXPECT(! nul.is_number());
        }

        // is_bool()
        {
            BEAST_EXPECT(! obj.is_bool());
            BEAST_EXPECT(! arr.is_bool());
            BEAST_EXPECT(! str.is_bool());
            BEAST_EXPECT(! num.is_bool());
            BEAST_EXPECT(  boo.is_bool());
            BEAST_EXPECT(! nul.is_bool());
        }

        // is_object()
        {
            BEAST_EXPECT(! obj.is_null());
            BEAST_EXPECT(! arr.is_null());
            BEAST_EXPECT(! str.is_null());
            BEAST_EXPECT(! num.is_null());
            BEAST_EXPECT(! boo.is_null());
            BEAST_EXPECT(  nul.is_null());
        }

        // is_primitive()
        {
            BEAST_EXPECT(! obj.is_primitive());
            BEAST_EXPECT(! arr.is_primitive());
            BEAST_EXPECT(  str.is_primitive());
            BEAST_EXPECT(  num.is_primitive());
            BEAST_EXPECT(  boo.is_primitive());
            BEAST_EXPECT(  nul.is_primitive());
        }

        // is_structured()
        {
            BEAST_EXPECT(  obj.is_structured());
            BEAST_EXPECT(  arr.is_structured());
            BEAST_EXPECT(! str.is_structured());
            BEAST_EXPECT(! num.is_structured());
            BEAST_EXPECT(! boo.is_structured());
            BEAST_EXPECT(! nul.is_structured());
        }
    }

    void
    testAccessors()
    {
        auto const sp =
            make_storage<unique_storage>();

        value obj(kind::object, sp);
        value arr(kind::array, sp);
        value str(kind::string, sp);
        value num(kind::number, sp);
        value boo(kind::boolean, sp);

        auto const& cobj(obj);
        auto const& carr(arr);
        auto const& cstr(str);
        auto const& cnum(num);
        auto const& cboo(boo);

        // get_storage()
        {
            value nul(kind::null, sp);
            BEAST_EXPECT(*obj.get_storage() == *sp);
            BEAST_EXPECT(*arr.get_storage() == *sp);
            BEAST_EXPECT(*str.get_storage() == *sp);
            BEAST_EXPECT(*num.get_storage() == *sp);
            BEAST_EXPECT(*boo.get_storage() == *sp);
            BEAST_EXPECT(*nul.get_storage() == *sp);
        }

        // if_object()
        {
            BEAST_EXPECT(obj.if_object() != nullptr);
            BEAST_EXPECT(arr.if_object() == nullptr);
            BEAST_EXPECT(str.if_object() == nullptr);
            BEAST_EXPECT(num.if_object() == nullptr);
            BEAST_EXPECT(boo.if_object() == nullptr);
        }

        // if_object() const
        {
            BEAST_EXPECT(cobj.if_object() != nullptr);
            BEAST_EXPECT(carr.if_object() == nullptr);
            BEAST_EXPECT(cstr.if_object() == nullptr);
            BEAST_EXPECT(cnum.if_object() == nullptr);
            BEAST_EXPECT(cboo.if_object() == nullptr);
        }

        // if_array()
        {
            BEAST_EXPECT(obj.if_array() == nullptr);
            BEAST_EXPECT(arr.if_array() != nullptr);
            BEAST_EXPECT(str.if_array() == nullptr);
            BEAST_EXPECT(num.if_array() == nullptr);
            BEAST_EXPECT(boo.if_array() == nullptr);
        }

        // if_array() const
        {
            BEAST_EXPECT(cobj.if_array() == nullptr);
            BEAST_EXPECT(carr.if_array() != nullptr);
            BEAST_EXPECT(cstr.if_array() == nullptr);
            BEAST_EXPECT(cnum.if_array() == nullptr);
            BEAST_EXPECT(cboo.if_array() == nullptr);
        }

        // if_string()
        {
            BEAST_EXPECT(obj.if_string() == nullptr);
            BEAST_EXPECT(arr.if_string() == nullptr);
            BEAST_EXPECT(str.if_string() != nullptr);
            BEAST_EXPECT(num.if_string() == nullptr);
            BEAST_EXPECT(boo.if_string() == nullptr);
        }

        // if_string() const
        {
            BEAST_EXPECT(cobj.if_string() == nullptr);
            BEAST_EXPECT(carr.if_string() == nullptr);
            BEAST_EXPECT(cstr.if_string() != nullptr);
            BEAST_EXPECT(cnum.if_string() == nullptr);
            BEAST_EXPECT(cboo.if_string() == nullptr);
        }

        // if_number()
        {
            BEAST_EXPECT(obj.if_number() == nullptr);
            BEAST_EXPECT(arr.if_number() == nullptr);
            BEAST_EXPECT(str.if_number() == nullptr);
            BEAST_EXPECT(num.if_number() != nullptr);
            BEAST_EXPECT(boo.if_number() == nullptr);
        }

        // if_number() const
        {
            BEAST_EXPECT(cobj.if_number() == nullptr);
            BEAST_EXPECT(carr.if_number() == nullptr);
            BEAST_EXPECT(cstr.if_number() == nullptr);
            BEAST_EXPECT(cnum.if_number() != nullptr);
            BEAST_EXPECT(cboo.if_number() == nullptr);
        }

        // if_bool()
        {
            BEAST_EXPECT(obj.if_bool() == nullptr);
            BEAST_EXPECT(arr.if_bool() == nullptr);
            BEAST_EXPECT(str.if_bool() == nullptr);
            BEAST_EXPECT(num.if_bool() == nullptr);
            BEAST_EXPECT(boo.if_bool() != nullptr);
        }

        // if_bool() const
        {
            BEAST_EXPECT(cobj.if_bool() == nullptr);
            BEAST_EXPECT(carr.if_bool() == nullptr);
            BEAST_EXPECT(cstr.if_bool() == nullptr);
            BEAST_EXPECT(cnum.if_bool() == nullptr);
            BEAST_EXPECT(cboo.if_bool() != nullptr);
        }

        // as_object()
        {
                         obj.as_object();
            BEAST_THROWS(arr.as_object(), system_error);
            BEAST_THROWS(str.as_object(), system_error);
            BEAST_THROWS(num.as_object(), system_error);
            BEAST_THROWS(boo.as_object(), system_error);
        }

        // as_object() const
        {
                         cobj.as_object();
            BEAST_THROWS(carr.as_object(), system_error);
            BEAST_THROWS(cstr.as_object(), system_error);
            BEAST_THROWS(cnum.as_object(), system_error);
            BEAST_THROWS(cboo.as_object(), system_error);
        }

        // as_array()
        {
            BEAST_THROWS(obj.as_array(), system_error);
                         arr.as_array();
            BEAST_THROWS(str.as_array(), system_error);
            BEAST_THROWS(num.as_array(), system_error);
            BEAST_THROWS(boo.as_array(), system_error);
        }

        // as_array() const
        {
            BEAST_THROWS(cobj.as_array(), system_error);
                         carr.as_array();
            BEAST_THROWS(cstr.as_array(), system_error);
            BEAST_THROWS(cnum.as_array(), system_error);
            BEAST_THROWS(cboo.as_array(), system_error);
        }

        // as_string()
        {
            BEAST_THROWS(obj.as_string(), system_error);
            BEAST_THROWS(arr.as_string(), system_error);
                         str.as_string();
            BEAST_THROWS(num.as_string(), system_error);
            BEAST_THROWS(boo.as_string(), system_error);
        }

        // as_string() const
        {
            BEAST_THROWS(cobj.as_string(), system_error);
            BEAST_THROWS(carr.as_string(), system_error);
                         cstr.as_string();
            BEAST_THROWS(cnum.as_string(), system_error);
            BEAST_THROWS(cboo.as_string(), system_error);
        }

        // as_number()
        {
            BEAST_THROWS(obj.as_number(), system_error);
            BEAST_THROWS(arr.as_number(), system_error);
            BEAST_THROWS(str.as_number(), system_error);
                         num.as_number();
            BEAST_THROWS(boo.as_number(), system_error);
        }

        // as_number() const
        {
            BEAST_THROWS(cobj.as_number(), system_error);
            BEAST_THROWS(carr.as_number(), system_error);
            BEAST_THROWS(cstr.as_number(), system_error);
                         cnum.as_number();
            BEAST_THROWS(cboo.as_number(), system_error);
        }

        // as_bool()
        {
            BEAST_THROWS(obj.as_bool(), system_error);
            BEAST_THROWS(arr.as_bool(), system_error);
            BEAST_THROWS(str.as_bool(), system_error);
            BEAST_THROWS(num.as_bool(), system_error);
                         boo.as_bool();
        }

        // as_bool() const
        {
            BEAST_THROWS(cobj.as_bool(), system_error);
            BEAST_THROWS(carr.as_bool(), system_error);
            BEAST_THROWS(cstr.as_bool(), system_error);
            BEAST_THROWS(cnum.as_bool(), system_error);
                         cboo.as_bool();
        }
    }

    void
    run() override
    {
        log <<
            "sizeof(value)  == " <<
            sizeof(value) << "\n";
        log <<
            "sizeof(object) == " <<
            sizeof(object) << "\n";
        log <<
            "sizeof(array)  == " <<
            sizeof(array) << "\n";
        log <<
            "sizeof(string) == " <<
            sizeof(string) << "\n";
        log <<
            "sizeof(number) == " <<
            sizeof(number) << "\n";
        
        testCustomTypes();

        testConstruction();
        testConversion();
        testModifiers();
        testExchange();
        testObservers();
        testAccessors();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,value);

} // json
} // boost
