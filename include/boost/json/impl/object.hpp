//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_OBJECT_HPP
#define BOOST_JSON_IMPL_OBJECT_HPP

#include <boost/json/value.hpp>
#include <boost/json/detail/varint.hpp>
#include <boost/core/exchange.hpp>
#include <boost/core/ignore_unused.hpp>
#include <algorithm>
#include <type_traits>

namespace boost {
namespace beast {
namespace json {

//------------------------------------------------------------------------------

struct object::list_hook
{
public:
    element* prev_;
    element* next_;
};

//------------------------------------------------------------------------------

struct object::element
    : public list_hook
{
    value v_;
    element* local_next_;

    BOOST_JSON_DECL
    string_view
    key() const noexcept;

    struct cleanup
    {
        std::size_t size;
        storage_ptr const& sp;
        std::size_t n;

        void
        operator()(char* p)
        {
            sp->deallocate(p,
                size, alignof(element));
        }
    };

    template<class Arg>
    static
    element*
    allocate(
        storage_ptr const& sp,
        key_type key,
        Arg&& arg)
    {
        auto up =
            prepare_allocate(sp, key);
        auto const p = up.get();
        auto const n = up.get_deleter().n;
        auto e = ::new(up.get()) element(
            std::forward<Arg>(arg), sp);
        up.release();
        detail::varint_write(
            p + sizeof(element), key.size());
        std::memcpy(
            p + sizeof(element) + n,
            key.data(),
            key.size());
        p[sizeof(element) +
            n + key.size()] = '\0';
        boost::ignore_unused(e);
        BOOST_ASSERT(
            *e->v_.get_storage() == *sp);
        return reinterpret_cast<element*>(p);
    }

    BOOST_JSON_DECL
    static
    void
    destroy(
        element const* e,
        storage_ptr const& sp);

private:
    template<class Arg>
    element(
        Arg&& arg,
        storage_ptr sp)
        : v_(std::forward<Arg>(arg),
            std::move(sp))
    {
    }

    BOOST_JSON_DECL
    static
    std::unique_ptr<char, cleanup>
    prepare_allocate(
        storage_ptr const& sp,
        key_type key);
};

//------------------------------------------------------------------------------

class object::hasher
{
    BOOST_JSON_DECL
    static
    std::pair<
        std::uint64_t, std::uint64_t>
    init(std::true_type) noexcept;

    BOOST_JSON_DECL
    static
    std::pair<
        std::uint32_t, std::uint32_t>
    init(std::false_type) noexcept;

public:
    BOOST_JSON_DECL
    std::size_t
    operator()(key_type key) const noexcept;
};

//------------------------------------------------------------------------------

class object::key_equal
{
public:
    bool
    operator()(
        beast::string_view lhs,
        beast::string_view rhs) const noexcept
    {
        return lhs == rhs;
    }
};

//------------------------------------------------------------------------------

class object::pointer
{
    reference t_;

public:
    pointer(reference t)
        : t_(t)
    {
    }

    reference*
    operator->() noexcept
    {
        return &t_;
    }
};

//------------------------------------------------------------------------------

class object::const_pointer
{
    const_reference t_;

public:
    const_pointer(
        const_reference t)
        : t_(t)
    {
    }

    const_reference*
    operator->() noexcept
    {
        return &t_;
    }
};

//------------------------------------------------------------------------------

class object::iterator
{
    element* e_ = nullptr;

    friend class object;

    iterator(element* e)
        : e_(e)
    {
    }

public:
    using value_type = object::value_type;
    using pointer = object::pointer;
    using reference = object::reference;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    iterator() = default;

    iterator(
        iterator const&) = default;

    iterator& operator=(
        iterator const&) = default;

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return e_ != other.e_;
    }

    bool
    operator==(
        const_iterator const& other) const noexcept;

    bool
    operator!=(
        const_iterator const& other) const noexcept;

    iterator&
    operator++() noexcept
    {
        e_ = e_->next_;
        return *this;
    }

    iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    iterator&
    operator--() noexcept
    {
        e_ = e_->prev_;
        return *this;
    }

    iterator
    operator--(int) noexcept
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    pointer
    operator->() const noexcept
    {
        return reference{
            e_->key(), e_->v_ };
    }

    reference
    operator*() const noexcept
    {
        return {
            e_->key(), e_->v_ };
    }
};

//------------------------------------------------------------------------------

class object::const_iterator
{
    element* e_ = nullptr;

    friend class object;

    const_iterator(element* e)
        : e_(e)
    {
    }

public:
    using value_type = object::value_type;
    using pointer = object::const_pointer;
    using reference = object::const_reference;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::bidirectional_iterator_tag;

    const_iterator() = default;

    const_iterator(
        const_iterator const&) = default;

    const_iterator& operator=(
        const_iterator const&) = default;

    const_iterator(iterator it)
        : e_(it.e_)
    {
    }

    const_iterator&
    operator=(iterator it) noexcept
    {
        e_ = it.e_;
        return *this;
    }

    bool
    operator==(
        const_iterator const& other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(
        const_iterator const& other) const noexcept
    {
        return e_ != other.e_;
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return e_ != other.e_;
    }

    const_iterator&
    operator++() noexcept
    {
        e_ = e_->next_;
        return *this;
    }

    const_iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    const_iterator&
    operator--() noexcept
    {
        e_ = e_->prev_;
        return *this;
    }

    const_iterator
    operator--(int) noexcept
    {
        auto tmp = *this;
        --*this;
        return tmp;
    }

    pointer
    operator->() const noexcept
    {
        return const_reference{
            e_->key(), e_->v_ };
    }

    reference
    operator*() const noexcept
    {
        return {
            e_->key(), e_->v_ };
    }
};

inline
bool
object::
iterator::
operator==(
    const_iterator const& other) const noexcept
{
    return e_ == other.e_;
}

inline
bool
object::
iterator::
operator!=(
    const_iterator const& other) const noexcept
{
    return e_ != other.e_;
}

//------------------------------------------------------------------------------

class object::local_iterator
{
    element* e_ = nullptr;

    friend class object;

    local_iterator(element* e)
        : e_(e)
    {
    }

public:
    using value_type = object::value_type;

    using reference = object::reference;

    local_iterator() = default;

    local_iterator(
        local_iterator const&) = default;

    local_iterator& operator=(
        local_iterator const&) = default;

    bool
    operator==(
        local_iterator const& other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(
        local_iterator const& other) const noexcept
    {
        return e_ != other.e_;
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return e_ != other.e_;
    }

    local_iterator&
    operator++() noexcept
    {
        e_ = e_->local_next_;
        return *this;
    }

    local_iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    const_pointer
    operator->() const noexcept
    {
        return const_reference{
            e_->key(), e_->v_ };
    }

    const_reference
    operator*() const noexcept
    {
        return {
            e_->key(), e_->v_ };
    }
};

//------------------------------------------------------------------------------

class object::const_local_iterator
{
    element const* e_ = nullptr;

    friend class object;

    const_local_iterator(element const* e)
        : e_(e)
    {
    }

public:
    using value_type = object::value_type;
    
    using reference = object::const_reference;

    const_local_iterator() = default;

    const_local_iterator(
        const_local_iterator const&) = default;

    const_local_iterator& operator=(
        const_local_iterator const&) = default;

    bool
    operator==(
        const_local_iterator const& other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(
        const_local_iterator const& other) const noexcept
    {
        return e_ != other.e_;
    }

    bool
    operator==(
        iterator const& other) const noexcept
    {
        return e_ == other.e_;
    }

    bool
    operator!=(
        iterator const& other) const noexcept
    {
        return e_ != other.e_;
    }

    const_local_iterator&
    operator++() noexcept
    {
        e_ = e_->local_next_;
        return *this;
    }

    const_local_iterator
    operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    const_pointer
    operator->() const noexcept
    {
        return const_reference{
            e_->key(), e_->v_ };
    }

    const_reference
    operator*() const noexcept
    {
        return {
            e_->key(), e_->v_ };
    }
};

//------------------------------------------------------------------------------

class object::node_type
{
    element* e_ = nullptr;
    storage_ptr sp_;

    friend class object;

    node_type(
        element* e,
        storage_ptr sp)
        : e_(e)
        , sp_(std::move(sp))
    {
    }

public:
    using key_type = beast::string_view;
    using mapped_type = json::value;

    node_type() = default;
    node_type(node_type const&) = delete;

    ~node_type()
    {
        if(e_)
            element::destroy(e_, sp_);
    }

    node_type(node_type&& other)
        : e_(boost::exchange(
            other.e_, nullptr))
        , sp_(boost::exchange(
            other.sp_, nullptr))
    {
    }

    node_type&
    operator=(node_type&& other)
    {
        if(e_)
        {
            element::destroy(e_, sp_);
            e_ = nullptr;
            sp_ = nullptr;
        }
        if(other.e_)
        {
            e_ = boost::exchange(
                other.e_, nullptr);
            sp_ = std::move(other.sp_);
        }
        return *this;
    }

    storage_ptr const&
    get_storage() const noexcept
    {
        return sp_;
    }

    bool
    empty() const noexcept
    {
        return e_ == nullptr;
    }

    explicit
    operator bool() const noexcept
    {
        return e_ != nullptr;
    }

    key_type
    key() const noexcept
    {
        return e_->key();
    }

    mapped_type&
    value() noexcept
    {
        return e_->v_;
    }

    mapped_type const&
    value() const noexcept
    {
        return e_->v_;
    }
};

struct object::insert_return_type
{
    iterator position;
    bool inserted;
    node_type node;
};


//------------------------------------------------------------------------------

template<class InputIt>
object::
object(
    InputIt first,
    InputIt last)
    : sp_(default_storage())
{
    construct(first, last, 0,
        iter_cat<InputIt>{});
}

template<class InputIt>
object::
object(
    InputIt first,
    InputIt last,
    size_type capacity)
    : sp_(default_storage())
{
    construct(first, last, capacity,
        iter_cat<InputIt>{});
}

template<class InputIt>
object::
object(
    InputIt first,
    InputIt last,
    storage_ptr store)
    : sp_(std::move(store))
{
    construct(first, last, 0,
        iter_cat<InputIt>{});
}

template<class InputIt>
object::
object(
    InputIt first,
    InputIt last,
    size_type capacity,
    storage_ptr store)
    : sp_(std::move(store))
{
    construct(first, last, capacity,
        iter_cat<InputIt>{});
}

//------------------------------------------------------------------------------

template<class P, class>
auto
object::
insert(P&& p)->
    std::pair<iterator, bool>
{
    return insert(end(), std::forward<P>(p));
}

template<class P, class>
auto
object::
insert(const_iterator before, P&& p) ->
    std::pair<iterator, bool>
{
    value_type v(std::forward<P>(p));
    return emplace_impl(before, v.first,
        std::move(v.second));
}

template<class InputIt>
void
object::
insert(InputIt first, InputIt last)
{
    insert(first, last, iter_cat<InputIt>{});
}

template<class M>
auto
object::
insert_or_assign(
    key_type key, M&& obj) ->
        std::pair<iterator, bool>
{
    return insert_or_assign(end(), key,
        std::forward<M>(obj));
}

template<class M>
auto
object::
insert_or_assign(
    const_iterator before,
    key_type key,
    M&& obj) ->
        std::pair<iterator, bool>
{
    auto const hash = hasher{}(key);
    auto e = prepare_insert(&before, key, hash);
    if(e)
    {
        e->v_ = std::forward<M>(obj);
        return { iterator(e), false };
    }
    e = element::allocate(sp_, key,
        std::forward<M>(obj));
    finish_insert(before, e, hash);
    return { iterator(e), true, };
}

template<class Arg>
auto
object::
emplace(key_type key, Arg&& arg) ->
    std::pair<iterator, bool>
{
    return emplace_impl(end(), key,
        std::forward<Arg>(arg));
}

template<class Arg>
auto
object::
emplace(const_iterator before,
    key_type key, Arg&& arg) ->
        std::pair<iterator, bool>
{
    return emplace_impl(before, key,
        std::forward<Arg>(arg));
}

inline
auto
object::
hash_function() const ->
    hasher
{
    return hasher{};
}

inline
auto
object::
key_eq() const ->
    key_equal
{
    return key_equal{};
}

//------------------------------------------------------------------------------

template<class InputIt>
void
object::
construct(
    InputIt first,
    InputIt last,
    size_type capacity,
    std::forward_iterator_tag)
{
    reserve(std::max<size_type>(capacity,
        std::distance(first, last)));
    while(first != last)
    {
        value_type v(*first++);
        emplace_impl(end(), v.first,
            std::move(v.second));
    }
}

template<class InputIt>
void
object::
construct(
    InputIt first,
    InputIt last,
    size_type capacity,
    std::input_iterator_tag)
{
    reserve(capacity);
    while(first != last)
    {
        value_type v(*first++);
        emplace_impl(end(), v.first,
            std::move(v.second));
    }
}

template<class InputIt>
void
object::
insert(InputIt first, InputIt last,
    std::forward_iterator_tag)
{
    reserve(size() +
        std::distance(first, last));
    while(first != last)
    {
        value_type v(*first++);
        emplace_impl(end(), v.first,
            std::move(v.second));
    }
}

template<class InputIt>
void
object::
insert(InputIt first, InputIt last,
    std::input_iterator_tag)
{
    while(first != last)
    {
        value_type v(*first++);
        emplace_impl(end(), v.first,
            std::move(v.second));
    }
}

template<class Arg>
auto
object::
emplace_impl(
    const_iterator before,
    key_type key,
    Arg&& arg) ->
        std::pair<iterator, bool>
{
    auto const hash = hasher{}(key);
    auto e = prepare_insert(
        &before, key, hash);
    if(e)
        return { iterator(e), false };
    e = element::allocate(sp_, key,
        std::forward<Arg>(arg));
    finish_insert(before, e, hash);
    return { iterator(e), true };
}

} // json
} // beast
} // boost

#endif
