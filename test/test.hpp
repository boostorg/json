//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_TEST_STORAGE_HPP
#define BOOST_JSON_TEST_STORAGE_HPP

#include <boost/json/basic_parser.hpp>
#include <boost/json/value.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

namespace boost {
namespace json {

//----------------------------------------------------------

struct unique_storage
{
    static
    constexpr
    unsigned long long
    id()
    {
        return 0;
    }

    static
    constexpr
    bool
    need_free()
    {
        return true;
    }

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
    unsigned long long
    id()
    {
        return 0;
    }

    static
    constexpr
    bool
    need_free()
    {
        return true;
    }

    std::size_t fail_max = 1;
    std::size_t fail = 0;

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
        return ::operator new(n);
    }

    void
    deallocate(
        void* p,
        std::size_t,
        std::size_t) noexcept
    {
        ::operator delete(p);
    }
};

//----------------------------------------------------------

template<class F>
void
fail_loop(F&& f)
{
    scoped_storage<fail_storage> ss;
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
    BEAST_EXPECT(ss->fail < 200);
}

//----------------------------------------------------------

class fail_parser
    : public basic_parser
{
    std::size_t n_ = std::size_t(-1);
    char buf[256];

    void
    maybe_fail(error_code& ec)
    {
        if(n_ && --n_ > 0)
            return;
        ec = error::test_failure;
    }

    void
    on_stack_info(
        stack& s) noexcept override
    {
        s.base = buf;
        s.capacity = sizeof(buf);
    }

    void
    on_stack_grow(
        stack&,
        unsigned,
        error_code& ec) override
    {
        ec = error::too_deep;
    }

    void
    on_document_begin(
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
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_key_data(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_key_end(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }
        
    void
    on_string_data(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_string_end(
        string_view,
        error_code& ec) override
    {
        maybe_fail(ec);
    }

    void
    on_number(
        ieee_decimal,
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
    char buf[256];

    void
    maybe_throw()
    {
        if(n_ && --n_ > 0)
            return;
        throw test_exception{};
    }

    void
    on_stack_info(
        stack& s) noexcept override
    {
        s.base = buf;
        s.capacity = sizeof(buf);
    }

    void
    on_stack_grow(
        stack&,
        unsigned,
        error_code& ec) override
    {
        ec = error::too_deep;
    }

    void
    on_document_begin(
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
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_key_data(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_key_end(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }
        
    void
    on_string_data(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_string_end(
        string_view,
        error_code&) override
    {
        maybe_throw();
    }

    void
    on_number(
        ieee_decimal,
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

inline
ieee_decimal
parse_ieee_decimal(string_view s)
{
    struct number_parser : basic_parser
    {
        std::size_t n_ = std::size_t(-1);
        char buf[256];

        void
        on_stack_info(
            stack& s) noexcept override
        {
            s.base = buf;
            s.capacity = sizeof(buf);
        }

        void
        on_stack_grow(
            stack&,
            unsigned,
            error_code& ec) override
        {
            ec = error::too_deep;
        }

        void
        on_document_begin(
            error_code&) override
        {
        }

        void
        on_object_begin(
            error_code&) override
        {
        }

        void
        on_object_end(
            error_code&) override
        {
        }

        void
        on_array_begin(
            error_code&) override
        {
        }

        void
        on_array_end(
            error_code&) override
        {
        }

        void
        on_key_data(
            string_view,
            error_code&) override
        {
        }

        void
        on_key_end(
            string_view,
            error_code&) override
        {
        }
        
        void
        on_string_data(
            string_view,
            error_code&) override
        {
        }

        void
        on_string_end(
            string_view,
            error_code&) override
        {
        }

        void
        on_number(
            ieee_decimal dec_,
            error_code&) override
        {
            dec = dec_;
        }

        void
        on_bool(
            bool,
            error_code&) override
        {
        }

        void
        on_null(error_code&) override
        {
        }

        ieee_decimal dec;
        number_parser() = default;
    };
    error_code ec;
    number_parser p;
    p.write(s.data(), s.size(), ec);
    if(ec)
        throw system_error(ec);
    return p.dec;
}

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
    array const& a,
    storage_ptr const& sp)
{
    if(*a.get_storage() != *sp)
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
        if(*v.as_object().get_storage() != *sp)
            return false;
        for(auto const& e : v.as_object())
            if(! equal_storage(e.second, sp))
                return false;
        return true;

    case json::kind::array:
        if(*v.as_array().get_storage() != *sp)
            return false;
        return equal_storage(v.as_array(), sp);

    case json::kind::string:
        return *v.as_string().get_storage() == *sp;

    case json::kind::int64:
    case json::kind::uint64:
    case json::kind::double_:
    case json::kind::boolean:
    case json::kind::null:
    break;
    }

    return *v.get_storage() == *sp;
}

inline
void
check_storage(
    array const& a,
    storage_ptr const& sp)
{
    BEAST_EXPECT(equal_storage(a, sp));
}

inline
void
check_storage(
    value const& v,
    storage_ptr const& sp)
{
    BEAST_EXPECT(equal_storage(v, sp));
}

//----------------------------------------------------------

} // json
} // boost

#endif
