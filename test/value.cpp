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

#include "test_storage.hpp"

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

//------------------------------------------------------------------------------

class value_test : public beast::unit_test::suite
{
public:
    void
    testSpecial()
    {
        // move ctor
        {
            value jv1;
            value jv2(std::move(jv1));
        }

        // copy ctor
        {
            value jv1;
            value jv2(jv1);
        }

        // move assign
        {
            value jv1;
            value jv2;
            jv2 = std::move(jv1);
        }

        // copy assign
        {
            value jv1;
            value jv2;
            jv2 = jv1;
        }
    }

    void
    testConstruct()
    {
        auto sp = make_storage<fail_storage>();
        storage_ptr sp0 =
            default_storage();

        // default ctor
        {
            value jv;
            BEAST_EXPECT(jv.is_null());
            BEAST_EXPECT(jv.get_storage() != sp);
            BEAST_EXPECT(jv.get_storage() == sp0);
        }

        // storage ctor
        {
            value jv1(sp);
            BEAST_EXPECT(jv1.is_null());
            BEAST_EXPECT(jv1.get_storage() == sp);
            BEAST_EXPECT(jv1.get_storage() != sp0);

            auto sp2 = sp;
            value jv2(std::move(sp2));
            BEAST_EXPECT(jv2.is_null());
            BEAST_EXPECT(jv1.get_storage() == sp);
            BEAST_EXPECT(jv1.get_storage() != sp0);
            BEAST_EXPECT(jv2.get_storage() == sp);
            BEAST_EXPECT(jv2.get_storage() != sp0);
        }

        // kind construct
        {
            value jv(kind::object);
            BEAST_EXPECT(jv.is_object());
            BEAST_EXPECT(jv.get_storage() != sp);
            BEAST_EXPECT(jv.get_storage() == sp0);
        }
        {
            value jv(kind::array);
            BEAST_EXPECT(jv.is_array());
            BEAST_EXPECT(jv.get_storage() != sp);
            BEAST_EXPECT(jv.get_storage() == sp0);
        }
        {
            value jv(kind::string);
            BEAST_EXPECT(jv.is_string());
            BEAST_EXPECT(jv.get_storage() != sp);
            BEAST_EXPECT(jv.get_storage() == sp0);
        }
        {
            value jv(kind::number);
            BEAST_EXPECT(jv.is_number());
            BEAST_EXPECT(jv.get_storage() != sp);
            BEAST_EXPECT(jv.get_storage() == sp0);
        }
        {
            value jv(kind::boolean);
            BEAST_EXPECT(jv.is_bool());
            BEAST_EXPECT(jv.get_storage() != sp);
            BEAST_EXPECT(jv.get_storage() == sp0);
        }
        {
            value jv(kind::null);
            BEAST_EXPECT(jv.is_null());
            BEAST_EXPECT(jv.get_storage() != sp);
            BEAST_EXPECT(jv.get_storage() == sp0);
        }

        // kind, storage construct
        {
            value jv(kind::object, sp);
            BEAST_EXPECT(jv.is_object());
            BEAST_EXPECT(jv.get_storage() == sp);
            BEAST_EXPECT(jv.get_storage() != sp0);
        }
        {
            value jv(kind::array, sp);
            BEAST_EXPECT(jv.is_array());
            BEAST_EXPECT(jv.get_storage() == sp);
            BEAST_EXPECT(jv.get_storage() != sp0);
        }
        {
            value jv(kind::string, sp);
            BEAST_EXPECT(jv.is_string());
            BEAST_EXPECT(jv.get_storage() == sp);
            BEAST_EXPECT(jv.get_storage() != sp0);
        }
        {
            value jv(kind::number, sp);
            BEAST_EXPECT(jv.is_number());
            BEAST_EXPECT(jv.get_storage() == sp);
            BEAST_EXPECT(jv.get_storage() != sp0);
        }
        {
            value jv(kind::boolean, sp);
            BEAST_EXPECT(jv.is_bool());
            BEAST_EXPECT(jv.get_storage() == sp);
            BEAST_EXPECT(jv.get_storage() != sp0);
        }
        {
            value jv(kind::null, sp);
            BEAST_EXPECT(jv.is_null());
            BEAST_EXPECT(jv.get_storage() == sp);
            BEAST_EXPECT(jv.get_storage() != sp0);
        }

        // construct from containers
        {
            {
                object obj;
                value jv(obj);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            {
                array arr;
                value jv(arr);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            {
                string str;
                value jv(str);
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            {
                value jv(number(1));
                BEAST_EXPECT(jv.is_number());
                BEAST_EXPECT(jv.is_int64());
                BEAST_EXPECT(jv.is_uint64());
                BEAST_EXPECT(jv.is_double());
                BEAST_EXPECT(jv.get_int64() == 1);
                BEAST_EXPECT(jv.get_uint64() == 1);
                BEAST_EXPECT(jv.get_double() == 1);
                BEAST_EXPECT(jv.as_number() == 1);
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            //
            {
                object obj;
                value jv(obj, sp);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
            {
                array arr;
                value jv(arr, sp);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
            {
                string str;
                value jv(str, sp);
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
            {
                value jv(number(1), sp);
                BEAST_EXPECT(jv.is_number());
                BEAST_EXPECT(jv.is_int64());
                BEAST_EXPECT(jv.is_uint64());
                BEAST_EXPECT(jv.is_double());
                BEAST_EXPECT(jv.get_int64() == 1);
                BEAST_EXPECT(jv.get_uint64() == 1);
                BEAST_EXPECT(jv.get_double() == 1);
                BEAST_EXPECT(jv.as_number() == 1);
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
        }

        // assign from containers
        {
            {
                object obj;
                value jv;

                jv = obj;
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);

                jv.reset();
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);

                jv = std::move(obj);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            {
                object obj;
                value jv(sp);

                jv = obj;
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);

                jv.reset();
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);

                jv = std::move(obj);
                BEAST_EXPECT(jv.is_object());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
            {
                array arr;
                value jv;

                jv = arr;
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);

                jv.reset();
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);

                jv = std::move(arr);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            {
                array arr;
                value jv(sp);

                jv = arr;
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);

                jv.reset();
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);

                jv = std::move(arr);
                BEAST_EXPECT(jv.is_array());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
            {
                string str;
                value jv;

                jv = str;
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);

                jv.reset();
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);

                jv = std::move(str);
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            {
                string str;
                value jv(sp);

                jv = str;
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);

                jv.reset();
                BEAST_EXPECT(jv.is_null());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);

                jv = std::move(str);
                BEAST_EXPECT(jv.is_string());
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
            {
                number n(1);
                value jv;
                jv = n;
                BEAST_EXPECT(jv.is_number());
                BEAST_EXPECT(jv.is_int64());
                BEAST_EXPECT(jv.is_uint64());
                BEAST_EXPECT(jv.is_double());
                BEAST_EXPECT(jv.get_int64() == 1);
                BEAST_EXPECT(jv.get_uint64() == 1);
                BEAST_EXPECT(jv.get_double() == 1);
                BEAST_EXPECT(jv.as_number() == 1);
                BEAST_EXPECT(jv.get_storage() != sp);
                BEAST_EXPECT(jv.get_storage() == sp0);
            }
            {
                number n(1);
                value jv(sp);
                jv = n;
                BEAST_EXPECT(jv.is_number());
                BEAST_EXPECT(jv.is_int64());
                BEAST_EXPECT(jv.is_uint64());
                BEAST_EXPECT(jv.is_double());
                BEAST_EXPECT(jv.get_int64() == 1);
                BEAST_EXPECT(jv.get_uint64() == 1);
                BEAST_EXPECT(jv.get_double() == 1);
                BEAST_EXPECT(jv.as_number() == 1);
                BEAST_EXPECT(jv.get_storage() == sp);
                BEAST_EXPECT(jv.get_storage() != sp0);
            }
        }
    }

    void
    testModifiers()
    {
        // reset
        {
            value jv;

            jv.reset(kind::object);
            BEAST_EXPECT(jv.is_object());

            jv.reset();
            BEAST_EXPECT(jv.is_null());

            jv.reset(kind::array);
            BEAST_EXPECT(jv.is_array());

            jv.reset(kind::string);
            BEAST_EXPECT(jv.is_string());

            jv.reset(kind::number);
            BEAST_EXPECT(jv.is_number());

            jv.reset(kind::boolean);
            BEAST_EXPECT(jv.is_bool());

            jv.reset(kind::null);
            BEAST_EXPECT(jv.is_null());
        }

        // emplace
        {
            {
                value jv;
                object& obj = jv.emplace_object();
                BEAST_EXPECT(jv.is_object());
                obj.clear();
            }
            {
                value jv;
                array& arr = jv.emplace_array();
                BEAST_EXPECT(jv.is_array());
                arr.clear();
            }
            {
                value jv;
                string& str = jv.emplace_string();
                BEAST_EXPECT(jv.is_string());
                str.clear();
            }
            {
                value jv;
                number& n= jv.emplace_number();
                BEAST_EXPECT(jv.is_number());
                n = 0;
            }
            {
                value jv;
                bool& b= jv.emplace_bool();
                BEAST_EXPECT(jv.is_bool());
                b = false;
            }
        }
    }

    void
    testExchange()
    {
        // construct from T
        {
            value v3("Hello!");
            value(std::string("Hello!"));
            value(short{0});
            value(int{0});
            value(long{0});
            value((long long)0);
            value((unsigned short)0);
            value((unsigned int)0);
            value((unsigned long)0);
            value((unsigned long long)0);
            value(float{0});
            value(double{0});
            value(true);
            value(false);
            value v4(null);
            value(nullptr);
        }

        // assign from T
        {
            value jv;
            jv = "Hello!";
            jv = std::string("Hello!");
            jv = short{};
            jv = int{};
            jv = long{};
            jv = (long long)0;
            jv = (unsigned short)0;
            jv = (unsigned int)0;
            jv = (unsigned long)0;
            jv = (unsigned long long)0;
            jv = float{};
            jv = double{};
            jv = true;
            jv = false;
            jv = null;
            jv = nullptr;
        }
    }

    void
    testAccessors()
    {
        // raw
        value jv;
        value const& jc(jv);
        {
            jv.emplace_object();
            BEAST_EXPECT(
                jv.as_object().size() == 0);
            jc.as_object();
        }
        {
            jv.emplace_array();
            BEAST_EXPECT(
                jv.as_array().size() == 0);
            jc.as_array();
        }
        {
            jv = "x";
            jv.as_string() = "y";
            BEAST_EXPECT(jc.as_string() == "y");
        }
        {
            jv = signed{};
            BEAST_EXPECT(jc.get_int64() == 0);
            jv.as_number() = -1;
            BEAST_EXPECT(jc.get_int64() == -1);
        }
        {
            jv = unsigned{};
            jv.as_number() = 2;
            BEAST_EXPECT(jc.get_uint64() == 2);
        }
        {
            jv = bool{};
            jv.as_bool() = true;
            BEAST_EXPECT(jc.as_bool());
        }
    }

    void
    testStructured()
    {
        // empty()
        {
            value jv;
            jv.emplace_object();
            BEAST_EXPECT(jv.empty());
            jv.emplace_array();
            BEAST_EXPECT(jv.empty());
        }

        // size()
        {
            value jv;
            jv.emplace_object();
            jv.as_object().emplace("x", 1);
            BEAST_EXPECT(jv.size() == 1);
            jv.emplace_array();
            jv.as_array().emplace_back(1);
            BEAST_EXPECT(jv.size() == 1);
        }
    }

    void
    testCustomization()
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

    void run() override
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
        
        testSpecial();
        testConstruct();
        testModifiers();
        testExchange();
        testAccessors();
        testStructured();
        testCustomization();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,value);

} // json
} // boost
