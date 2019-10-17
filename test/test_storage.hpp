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

#include <boost/json/value.hpp>
#include <boost/json/storage.hpp>
#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

namespace boost {
namespace json {

struct unique_storage : storage
{
    void*
    allocate(
        std::size_t n,
        std::size_t) override
    {
        return std::allocator<
            char>{}.allocate(n);
    }

    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t) noexcept override
    {
        auto cp =
            reinterpret_cast<char*>(p);
        return std::allocator<
            char>{}.deallocate(cp, n);
    }
    bool
    is_equal(
        storage const&) const noexcept override
    {
        return false;
    }
};

//------------------------------------------------------------------------------

struct test_failure : std::exception
{
    virtual
    char const*
    what() const noexcept override
    {
        return "test failure";
    }
};

struct fail_storage : storage
{
    std::size_t fail_max = 1;
    std::size_t fail = 0;

    ~fail_storage()
    {
    }

    void*
    allocate(
        std::size_t n,
        std::size_t) override
    {
        if(++fail == fail_max)
        {
            ++fail_max;
            fail = 0;
            throw test_failure{};
        }
        return std::allocator<
            char>{}.allocate(n);
    }

    void
    deallocate(
        void* p,
        std::size_t n,
        std::size_t) noexcept override
    {
        auto cp =
            reinterpret_cast<char*>(p);
        return std::allocator<
            char>{}.deallocate(cp, n);
    }
    bool
    is_equal(
        storage const&) const noexcept override
    {
        return false;
    }
};

//------------------------------------------------------------------------------

class scoped_fail_storage
{
    storage_ptr sp_;

public:
    scoped_fail_storage()
        : sp_(default_storage())
    {
        default_storage(
            make_storage<fail_storage>());
    }

    ~scoped_fail_storage()
    {
        default_storage(sp_);
    }

    storage_ptr const&
    get() const noexcept
    {
        return sp_;
    }
};

//------------------------------------------------------------------------------

namespace detail {

template<class F>
typename std::enable_if<
    std::is_same<void,
        decltype(std::declval<F const&>()(
            std::declval<storage_ptr>()))>::value
    >::type
fail_loop(F&& f)
{
    auto sp = make_storage<fail_storage>();
    while(sp->fail < 200)
    {
        try
        {
            f(sp);
        }
        catch(test_failure const&)
        {
            continue;
        }
        break;
    }
    BEAST_EXPECT(sp->fail < 200);
}

template<class F>
typename std::enable_if<
    std::is_same<void, decltype(
        std::declval<F const&>()())>::value
    >::type
fail_loop(F&& f)
{
    auto saved = default_storage();
    auto sp =
        make_storage<fail_storage>();
    default_storage(sp);
    while(sp->fail < 200)
    {
        try
        {
            f();
        }
        catch(test_failure const&)
        {
            continue;
        }
        break;
    }
    BEAST_EXPECT(sp->fail < 200);
    default_storage(saved);
}

} // detail

template<class F>
void
fail_loop(F&& f)
{
    detail::fail_loop(
        std::forward<F>(f));
}

//------------------------------------------------------------------------------

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

    case json::kind::number:
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

//------------------------------------------------------------------------------

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

} // json
} // boost

#endif
