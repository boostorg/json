//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

// Test that header file is self-contained.
#include <boost/json/visit.hpp>

#include "test_suite.hpp"

namespace boost {
namespace json {

template<class T>
struct visitor_checker;

template<>
struct visitor_checker<value const&>
{
    using self_ref = visitor_checker const&;

    visitor_checker() = default;

    std::pair<kind, void const*>
    operator()(std::nullptr_t const& n) const
    { return {kind::null, &n}; }

    std::pair<kind, void const*>
    operator()(bool&)
    { BOOST_TEST_FAIL(); return {(kind)50, nullptr}; }

    std::pair<kind, void const*>
    operator()(bool const& b) const
    { return {kind::bool_, &b}; }

    std::pair<kind, void const*>
    operator()(std::int64_t const& i) const
    { return {kind::int64, &i}; }

    std::pair<kind, void const*>
    operator()(std::uint64_t const& u) const
    { return {kind::uint64, &u}; }

    std::pair<kind, void const*>
    operator()(double const& d) const
    { return {kind::double_, &d}; }

    std::pair<kind, void const*>
    operator()(string const& s) const
    { return {kind::string, &s}; }

    std::pair<kind, void const*>
    operator()(array const& a) const
    { return {kind::array, &a}; }

    std::pair<kind, void const*>
    operator()(object const& o) const
    { return {kind::object, &o}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) const&
    { return {(kind)40, nullptr}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) &
    { return {(kind)41, nullptr}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) &&
    { return {(kind)42, nullptr}; }
};

template<>
struct visitor_checker<value&>
{
    using self_ref = visitor_checker&;

    visitor_checker() = default;

    std::pair<kind, void*>
    operator()(std::nullptr_t& n)
    { return {kind::null, &n}; }

    std::pair<kind, void*>
    operator()(bool& b)
    { return {kind::bool_, &b}; }

    std::pair<kind, void*>
    operator()(std::int64_t& i)
    { return {kind::int64, &i}; }

    std::pair<kind, void*>
    operator()(std::uint64_t& u)
    { return {kind::uint64, &u}; }

    std::pair<kind, void*>
    operator()(double& d)
    { return {kind::double_, &d}; }

    std::pair<kind, void*>
    operator()(string& s)
    { return {kind::string, &s}; }

    std::pair<kind, void*>
    operator()(array& a)
    { return {kind::array, &a}; }

    std::pair<kind, void*>
    operator()(object& o)
    { return {kind::object, &o}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) const&
    { return {(kind)40, nullptr}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) &
    { return {(kind)41, nullptr}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) &&
    { return {(kind)42, nullptr}; }
};

template<>
struct visitor_checker<value&&>
{
    using self_ref = visitor_checker&&;

    visitor_checker() = default;

    std::pair<kind, void*>
    operator()(std::nullptr_t&& n) &&
    { return {kind::null, &n}; }

    std::pair<kind, void*>
    operator()(bool&& b) &&
    { return {kind::bool_, &b}; }

    std::pair<kind, void*>
    operator()(std::int64_t&& i) &&
    { return {kind::int64, &i}; }

    std::pair<kind, void*>
    operator()(std::uint64_t&& u) &&
    { return {kind::uint64, &u}; }

    std::pair<kind, void*>
    operator()(double&& d) &&
    { return {kind::double_, &d}; }

    std::pair<kind, void*>
    operator()(string&& s) &&
    { return {kind::string, &s}; }

    std::pair<kind, void*>
    operator()(array&& a) &&
    { return {kind::array, &a}; }

    std::pair<kind, void*>
    operator()(object&& o) &&
    { return {kind::object, &o}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) const&
    { static_assert( !std::is_same<T, T>::value, "" ); return {(kind)40, nullptr}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) &
    { static_assert( !std::is_same<T, T>::value, "" ); return {(kind)41, nullptr}; }

    template<class T>
    std::pair<kind, void const*>
    operator()(T&&) &&
    { static_assert( !std::is_same<T, T>::value, "" ); return {(kind)42, nullptr}; }
};

class visit_test
{
public:
    struct visitor_ref_tester
    {
        using Refs = mp11::mp_list<value const&, value&, value&&>;

        template< class I >
        void
        operator()(I) const
        {
            using ValueRef = mp11::mp_at<Refs, I>;
            check_ref<ValueRef>(kind::null,    nullptr);
            check_ref<ValueRef>(kind::bool_,   true);
            check_ref<ValueRef>(kind::int64,   -1);
            check_ref<ValueRef>(kind::uint64,  1U);
            check_ref<ValueRef>(kind::double_, 3.14);
            check_ref<ValueRef>(kind::string,  string_kind);
            check_ref<ValueRef>(kind::array,   array_kind);
            check_ref<ValueRef>(kind::object,  object_kind);
        }

        void
        operator()() const
        {
            mp11::mp_for_each< mp11::mp_iota<mp11::mp_size<Refs>> >(*this);
        }
    };

    template<class ValueRef, class T>
    static
    void
    check_ref(
        kind k,
        T t,
        boost::source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        using Visitor = visitor_checker<ValueRef>;
        using VisitorRef = typename Visitor::self_ref;
        Visitor v = {};

        value jv(t);
        auto pr1 = visit(
            static_cast<VisitorRef>(v), static_cast<ValueRef>(jv) );
        auto pr2 = visit(
            static_cast<VisitorRef>(v), static_cast<ValueRef>(jv) );
        BOOST_TEST(pr1 == pr2, loc);
        BOOST_TEST(k == pr1.first, loc);
        switch(k)
        {
        case json::kind::array:
            BOOST_TEST( pr1.second == jv.if_array(), loc );
            break;
        case json::kind::object:
            BOOST_TEST( pr1.second == jv.if_object(), loc );
            break;
        case json::kind::string:
            BOOST_TEST( pr1.second == jv.if_string(), loc );
            break;
        case json::kind::double_:
            BOOST_TEST( pr1.second == jv.if_double(), loc );
            break;
        case json::kind::int64:
            BOOST_TEST( pr1.second == jv.if_int64(), loc );
            break;
        case json::kind::uint64:
            BOOST_TEST( pr1.second == jv.if_uint64(), loc );
            break;
        case json::kind::bool_:
            BOOST_TEST( pr1.second == jv.if_bool(), loc );
            break;
        case json::kind::null:
            break;
        default:
            BOOST_TEST_FAIL(loc);
        }
    }

    template<class T>
    void
    check_nonref(
        kind k,
        T t,
        boost::source_location const& loc = BOOST_CURRENT_LOCATION)
    {
        struct f
        {
            kind k;
            bool operator()(std::nullptr_t) && { return k == kind::null; }
            bool operator()(bool) && { return k == kind::bool_; }
            bool operator()(std::int64_t) && { return k == kind::int64; }
            bool operator()(std::uint64_t) && { return k == kind::uint64; }
            bool operator()(double) && { return k == kind::double_; }
            bool operator()(string) && { return k == kind::string; }
            bool operator()(array) && { return k == kind::array; }
            bool operator()(object) && { return k == kind::object; }
            bool operator()(...) const { return false; }
        };
        value v(t);
        f f_{k};
        BOOST_TEST( visit(std::move(f_), std::move(v)), loc );
    }

    void run()
    {
        value const jv;
        visitor_ref_tester()();

        BOOST_TEST_CHECKPOINT();
        check_nonref(kind::null,    nullptr);
        check_nonref(kind::bool_,   true);
        check_nonref(kind::int64,   -1);
        check_nonref(kind::uint64,  1U);
        check_nonref(kind::double_, 3.14);
        check_nonref(kind::string,  string_kind);
        check_nonref(kind::array,   array_kind);
        check_nonref(kind::object,  object_kind);
    }
};

TEST_SUITE(visit_test, "boost.json.visit");

} // namespace json
} // namespace boost
