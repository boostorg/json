//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_VALUE_REF_HPP
#define BOOST_JSON_VALUE_REF_HPP

#include <boost/json/config.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/string.hpp>
#include <cstdint>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

#ifndef GENERATING_DOCUMENTATION

class value;
class object;
class array;
class string;

class value_ref
{
    friend class value;
    friend class object;
    friend class array;

    friend class value_ref_test;

    using func_ptr =
        value(*)(void const*);
    enum class what
    {
        val, cval,
        obj, cobj,
        arr, carr,
        str,

        svw,
        i64,
        u64,
        dub,
        boo,
        nul,

        ini,
        fun
    };

    struct string_type
    {
        char const* data;
        std::size_t size;

        string_type(
            string_view s)
            : data(s.data())
            , size(s.size())
        {
        }
    };

    struct func_type
    {
        func_ptr fp;
        void const* p;
    };

    union
    {
        value*        const val_;
        object*       const obj_;
        array*        const arr_;
        value const*  const cval_;
        object const* const cobj_;
        array const*  const carr_;
        string*       const str_;

        string_type   const sv_;
        std::int64_t  const i64_;
        std::uint64_t const u64_;
        double        const dub_;
        bool          const boo_;

        std::initializer_list<
            value_ref> const ini_;
        func_type const fun_;
    };

    what const what_;

public:
    value_ref(
        value_ref const&) = default;

    value_ref(
        std::initializer_list<
            value_ref> init) noexcept
        : ini_(init)
        , what_(what::ini)
    {
    }

    value_ref(
        value&& v) noexcept
        : val_(&v)
        , what_(what::val)
    {
    }

    value_ref(
        value const& v) noexcept
        : cval_(&v)
        , what_(what::cval)
    {
    }

    value_ref(
        object&& o) noexcept
        : obj_(&o)
        , what_(what::obj)
    {
    }

    value_ref(
        object const& o) noexcept
        : cobj_(&o)
        , what_(what::cobj)
    {
    }

    value_ref(
        array&& a) noexcept
        : arr_(&a)
        , what_(what::arr)
    {
    }

    value_ref(
        array const& a) noexcept
        : carr_(&a)
        , what_(what::carr)
    {
    }

    value_ref(
        string&& s) noexcept
        : str_(&s)
        , what_(what::str)
    {
    }

    value_ref(
        string const& s) noexcept
        : sv_(static_cast<string_view>(s))
        , what_(what::svw)
    {
    }

    //---

    value_ref(
        string_view s) noexcept
        : sv_(s)
        , what_(what::svw)
    {
    }

    value_ref(
        char const* s) noexcept
        : sv_(string_view(s))
        , what_(what::svw)
    {
    }

    value_ref(
        short i)
        : i64_(i)
        , what_(what::i64)
    {
    }

    value_ref(
        int i)
        : i64_(i)
        , what_(what::i64)
    {
    }

    value_ref(
        long i)
        : i64_(i)
        , what_(what::i64)
    {
    }

    value_ref(
        long long i)
        : i64_(i)
        , what_(what::i64)
    {
    }

    value_ref(
        unsigned short u)
        : u64_(u)
        , what_(what::u64)
    {
    }

    value_ref(
        unsigned int u)
        : u64_(u)
        , what_(what::u64)
    {
    }

    value_ref(
        unsigned long u)
        : u64_(u)
        , what_(what::u64)
    {
    }

    value_ref(
        unsigned long long u)
        : u64_(u)
        , what_(what::u64)
    {
    }

    value_ref(
        double d)
        : dub_(d)
        , what_(what::dub)
    {
    }

    value_ref(
        long double d)
        : dub_(static_cast<double>(d))
        , what_(what::dub)
    {
    }

    template<class Bool
        ,class = typename std::enable_if<
            std::is_same<Bool, bool>::value>::type
    >
    value_ref(Bool b) noexcept
        : boo_(b)
        , what_(what::boo)
    {
    }

    value_ref(std::nullptr_t)
        : what_(what::nul)
    {
    }

    BOOST_JSON_DECL
    operator value() const;

private:
    inline
    bool
    is_key_value_pair() const noexcept;

    static
    inline
    bool
    maybe_object(
        std::initializer_list<
            value_ref> init) noexcept;

    inline
    string_view
    get_string() const noexcept;

    BOOST_JSON_DECL
    value
    make_value(
        storage_ptr sp) const;

    BOOST_JSON_DECL
    static
    value
    make_value(
        std::initializer_list<
            value_ref> init,
        storage_ptr sp);

    BOOST_JSON_DECL
    static
    object
    make_object(
        std::initializer_list<value_ref> init,
        storage_ptr sp);

    BOOST_JSON_DECL
    static
    array
    make_array(
        std::initializer_list<
            value_ref> init,
        storage_ptr sp);

    BOOST_JSON_DECL
    static
    void
    write_array(
        value* dest,
        std::initializer_list<
            value_ref> init,
        storage_ptr const& sp);
};

#endif

} // json
} // boost

// Must be included here for this file to stand alone
#include <boost/json/value.hpp>

// includes are at the bottom of <boost/json/value.hpp>

#endif
