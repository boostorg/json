//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_TEST_HPP
#define BOOST_JSON_TEST_HPP

#include <boost/json/basic_parser.hpp>
#include <boost/json/value.hpp>
#include <boost/json/serializer.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/detail/format.hpp>

#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

#include "test_suite.hpp"

namespace boost {
namespace json {

//----------------------------------------------------------

struct test_failure : std::exception
{
    virtual
    char const*
    what() const noexcept override
    {
        return "test failure";
    }
};

struct fail_storage
{
    static
    constexpr
    std::uint64_t
    id = 0;

    static
    constexpr
    bool
    need_free = true;

    std::size_t fail_max = 0;
    std::size_t fail = 0;
    std::size_t nalloc = 0;

    ~fail_storage()
    {
        BOOST_TEST(nalloc == 0);
    }

    void*
    allocate(
        std::size_t n,
        std::size_t)
    {
        if(++fail == fail_max)
        {
            ++fail_max;
            fail = 0;
            throw test_failure{};
        }
        auto p = ::operator new(n);
        ++nalloc;
        return p;
    }

    void
    deallocate(
        void* p,
        std::size_t,
        std::size_t) noexcept
    {
        if(BOOST_TEST(nalloc > 0))
            --nalloc;
        ::operator delete(p);
    }
};

template<class F>
void
fail_loop(F&& f)
{
    scoped_storage<fail_storage> ss;
    ss->fail_max = 1;
    while(ss->fail < 200)
    {
        try
        {
            f(ss);
        }
        catch(test_failure const&)
        {
            continue;
        }
        break;
    }
    BOOST_TEST(ss->fail < 200);
}

//----------------------------------------------------------

struct unique_storage
{
    static
    constexpr
    std::uint64_t
    id = 0;

    static
    constexpr
    bool
    need_free = true;

    void*
    allocate(
        std::size_t n,
        std::size_t)
    {
        return ::operator new(n);
    }

    void
    deallocate(
        void* p,
        std::size_t,
        std::size_t) noexcept
    {
        return ::operator delete(p);
    }
};

//----------------------------------------------------------

class fail_parser
    : public basic_parser
{
    std::size_t n_ = std::size_t(-1);

    void
    maybe_fail(error_code& ec)
    {
        if(n_ && --n_ > 0)
            return;
        ec = error::test_failure;
    }

    void
    on_document_begin(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_document_end(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_object_begin(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_object_end(
        std::size_t,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_array_begin(
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_array_end(
        std::size_t,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_key_part(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_key(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }
        
    void
    on_string_part(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_string(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_int64(
        int64_t,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_uint64(
        uint64_t,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_double(
        double,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_bool(
        bool,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_null(error_code& ec) override
    {
        maybe_fail(ec);
    }

public:
    fail_parser() = default;

    explicit
    fail_parser(
        std::size_t n)
        : n_(n)
    {
    }

    void
    reset()
    {
        basic_parser::reset();
    }
};

//----------------------------------------------------------

struct test_exception
    : std::exception
{
    char const*
    what() const noexcept
    {
        return "test exception";
    }
};

// Exercises every exception path
class throw_parser
    : public basic_parser
{
    std::size_t n_ = std::size_t(-1);

    void
    maybe_throw()
    {
        if(n_ && --n_ > 0)
            return;
        throw test_exception{};
    }

    void
    on_document_begin(
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_document_end(
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_object_begin(
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_object_end(
        std::size_t,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_array_begin(
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_array_end(
        std::size_t,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_key_part(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_key(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }
        
    void
    on_string_part(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_string(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_int64(
        int64_t,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_uint64(
        uint64_t,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_double(
        double,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_bool(
        bool,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_null(error_code&) override
    {
        maybe_throw();
    }

public:
    throw_parser() = default;

    explicit
    throw_parser(
        std::size_t n)
        : n_(n)
    {
    }
};

//----------------------------------------------------------

// wrap an iterator to make an input iterator
template<class FwdIt>
class input_iterator
{
    FwdIt it_;


public:
    using value_type = typename std::iterator_traits<FwdIt>::value_type;
    using pointer = typename std::iterator_traits<FwdIt>::pointer;
    using reference = typename std::iterator_traits<FwdIt>::reference;
    using difference_type = typename std::iterator_traits<FwdIt>::difference_type;
    using iterator_category = std::input_iterator_tag;

    input_iterator() = default;
    input_iterator(input_iterator const&) = default;
    input_iterator& operator=(
        input_iterator const&) = default;

    input_iterator(FwdIt it)
        : it_(it)
    {
    }

    input_iterator&
    operator++() noexcept
    {
        ++it_;
        return *this;
    }

    input_iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    pointer
    operator->() const noexcept
    {
        return it_.operator->();
    }

    reference
    operator*() const noexcept
    {
        return *it_;
    }

    bool
    operator==(input_iterator other) const noexcept
    {
        return it_ == other.it_;
    }

    bool
    operator!=(input_iterator other) const noexcept
    {
        return it_ != other.it_;
    }
};

template<class FwdIt>
input_iterator<FwdIt>
make_input_iterator(FwdIt it)
{
    return input_iterator<FwdIt>(it);
}

//----------------------------------------------------------

inline
bool
equal_storage(
    value const& v,
    storage_ptr const& sp);

inline
bool
equal_storage(
    object const& o,
    storage_ptr const& sp)
{
    if(*o.storage() != *sp)
        return false;
    for(auto const& e : o)
        if(! equal_storage(e.value(), sp))
            return false;
    return true;
}

inline
bool
equal_storage(
    array const& a,
    storage_ptr const& sp)
{
    if(*a.storage() != *sp)
        return false;
    for(auto const& v : a)
        if(! equal_storage(v, sp))
            return false;
    return true;
}

bool
equal_storage(
    value const& v,
    storage_ptr const& sp)
{
    switch(v.kind())
    {
    case json::kind::object:
        if(*v.as_object().storage() != *sp)
            return false;
        return equal_storage(v.as_object(), sp);

    case json::kind::array:
        if(*v.as_array().storage() != *sp)
            return false;
        return equal_storage(v.as_array(), sp);

    case json::kind::string:
        return *v.as_string().storage() == *sp;

    case json::kind::int64:
    case json::kind::uint64:
    case json::kind::double_:
    case json::kind::bool_:
    case json::kind::null:
    break;
    }

    return *v.storage() == *sp;
}

inline
void
check_storage(
    object const& o,
    storage_ptr const& sp)
{
    BOOST_TEST(equal_storage(o, sp));
}

inline
void
check_storage(
    array const& a,
    storage_ptr const& sp)
{
    BOOST_TEST(equal_storage(a, sp));
}

inline
void
check_storage(
    value const& v,
    storage_ptr const& sp)
{
    BOOST_TEST(equal_storage(v, sp));
}

//----------------------------------------------------------

namespace detail {

inline
void
to_string_test(
    string& dest,
    json::value const& jv)
{
    switch(jv.kind())
    {
    case kind::object:
    {
        dest.push_back('{');
        auto const& obj(
            jv.get_object());
        auto it = obj.begin();
        if(it != obj.end())
        {
            goto obj_first;
            while(it != obj.end())
            {
                dest.push_back(',');
            obj_first:
                dest.push_back('\"');
                dest.append(it->key());
                dest.push_back('\"');
                dest.push_back(':');
                to_string_test(
                    dest, it->value());
                ++it;
            }
        }
        dest.push_back('}');
        break;
    }

    case kind::array:
    {
        dest.push_back('[');
        auto const& arr(
            jv.get_array());
        auto it = arr.begin();
        if(it != arr.end())
        {
            goto arr_first;
            while(it != arr.end())
            {
                dest.push_back(',');
            arr_first:
                to_string_test(
                    dest, *it);
                ++it;
            }
        }
        dest.push_back(']');
        break;
    }

    case kind::string:
    #if 1
        // safe, but doesn't handle escsapes
        dest.push_back('\"');
        dest.append(jv.get_string());
        dest.push_back('\"');
    #else
        dest.append(to_string(jv));
    #endif
        break;

    case kind::int64:
    {
        char buf[detail::max_number_chars];
        auto const n =
            detail::format_int64(
                buf, jv.as_int64());
        dest.append(string_view(buf).substr(0, n));
        break;
    }

    case kind::uint64:
    {
        char buf[detail::max_number_chars];
        auto const n =
            detail::format_uint64(
                buf, jv.as_uint64());
        dest.append(string_view(buf).substr(0, n));
        break;
    }

    case kind::double_:
    {
        char buf[detail::max_number_chars];
        auto const n =
            detail::format_double(
                buf, jv.as_double());
        dest.append(string_view(buf).substr(0, n));
        break;
    }

    case kind::bool_:
        if(jv.as_bool())
            dest.append("true");
        else
            dest.append("false");
        break;

    case kind::null:
        dest.append("null");
        break;

    default:
        // should never get here
        dest.append("?");
        break;
    }
}

} // detail

inline
string
to_string_test(
    json::value const& jv)
{
    string s;
    s.reserve(1024);
    detail::to_string_test(s, jv);
    return s;
}

//----------------------------------------------------------

inline
bool
equal(
    value const& lhs,
    value const& rhs)
{
    if(lhs.kind() != rhs.kind())
        return false;
    switch(lhs.kind())
    {
    case kind::object:
    {
        auto const& obj1 =
            lhs.get_object();
        auto const& obj2 =
            rhs.get_object();
        auto n = obj1.size();
        if(obj2.size() != n)
            return false;
        auto it1 = obj1.begin();
        auto it2 = obj2.begin();
        while(n--)
        {
            if( it1->key() !=
                it2->key())
                return false;
            if(! equal(
                it1->value(),
                it2->value()))
                return false;
            ++it1;
            ++it2;
        }
        return true;
    }

    case kind::array:
    {
        auto const& arr1 =
            lhs.get_array();
        auto const& arr2 =
            rhs.get_array();
        auto n = arr1.size();
        if(arr2.size() != n)
            return false;
        auto it1 = arr1.begin();
        auto it2 = arr2.begin();
        while(n--)
            if(! equal(*it1++, *it2++))
                return false;
        return true;
    }

    case kind::string:
        return
            lhs.get_string() ==
            rhs.get_string();

    case kind::double_:
        return
            lhs.as_double() ==
            rhs.as_double();

    case kind::int64:
        return
            *lhs.if_int64() ==
            *rhs.if_int64();

    case kind::uint64:
        return
            *lhs.if_uint64() ==
            *rhs.if_uint64();

    case kind::bool_:
        return
            *lhs.if_bool() ==
            *rhs.if_bool();

    case kind::null:
        return true;
    }

    return false;
}

//----------------------------------------------------------

} // json
} // boost

#endif
