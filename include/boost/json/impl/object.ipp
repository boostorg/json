//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_OBJECT_IPP
#define BOOST_JSON_IMPL_OBJECT_IPP

#include <boost/json/object.hpp>
#include <boost/core/exchange.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/throw_exception.hpp>
#include <boost/assert.hpp>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace boost {
namespace json {

//------------------------------------------------------------------------------

string_view
object::
element::
key() const noexcept
{
    auto p =reinterpret_cast<
        char const*>(this + 1);
    auto const result =
        detail::varint_read(p);
    return {
        p + result.second,
        static_cast<std::size_t>(
            result.first) };
}

void
object::
element::
destroy(
    element const* e,
    storage_ptr const& sp) noexcept
{
    auto const len = e->key().size();
    auto const n =
        detail::varint_size(len);
    e->~element();
    sp->deallocate(
        const_cast<element*>(e),
        sizeof(element) + n + len + 1,
        alignof(element));
}

auto
object::
element::
prepare_allocate(
    storage_ptr const& sp,
    key_type key) ->
        std::unique_ptr<char, cleanup>
{
    auto const n = static_cast<std::size_t>(
        detail::varint_size(key.size()));
    auto const size =
        sizeof(element) + n + key.size() + 1;
    auto p = reinterpret_cast<char*>(
        sp->allocate(size, alignof(element)));
    return { p, cleanup{size, sp, n} };
}

//------------------------------------------------------------------------------

struct object::table
{
    // number of values in the object
    std::size_t count = 0;

    // number of buckets in table
    std::size_t bucket_count = 0;

    // insertion-order list of all objects
    element* head;

    list_hook end_element;

    table() noexcept
        : head(end())
    {
    }

    element*
    begin() noexcept
    {
        return head;
    }

    element*
    end() noexcept
    {
        return reinterpret_cast<
            element*>(&end_element);
    }

    element*&
    bucket(std::size_t n) noexcept
    {
        return reinterpret_cast<
            element**>(this + 1)[n];
    }

    static
    table*
    construct(
        size_type bucket_count,
        storage_ptr const& sp)
    {
        auto tab = ::new(sp->allocate(
            sizeof(table) +
                bucket_count *
                sizeof(element*),
            (std::max)(
                alignof(table),
                alignof(element*))
                    )) table;
        tab->count = 0;
        tab->bucket_count = bucket_count;
        auto it = &tab->bucket(0);
        auto const last =
            &tab->bucket(bucket_count);
        while(it != last)
            *it++ = tab->end();
        return tab;
    }

    static
    void
    destroy(
        table* tab,
        storage_ptr const& sp) noexcept
    {
        auto const count =
            tab->bucket_count;
        tab->~table();
        sp->deallocate(
            tab,
            sizeof(table) +
                count * sizeof(element*),
            alignof(table));
    }

    static
    void
    destroy_list(
        table* tab,
        storage_ptr const& sp) noexcept
    {
        for(auto it = tab->head;
            it != tab->end();)
        {
            auto next = it->next_;
            element::destroy(it, sp);
            it = next;
        }
    }

    static
    table*
    allocate(
        table* from,
        size_type bucket_count,
        storage_ptr const& sp)
    {
        auto tab =
            construct(bucket_count, sp);
        if(from)
        {
            tab->count = from->count;
            tab->bucket_count = bucket_count;
            if(from->head != from->end())
            {
                tab->head = from->head;
                tab->end()->prev_ =
                    from->end()->prev_;
                tab->end()->prev_->next_ =
                    tab->end();
            }
            else
            {
                tab->head = tab->end();
            }
        }
        if(from)
            destroy(from, sp);
        return tab;
    }
};

//------------------------------------------------------------------------------

std::pair<
    std::uint64_t,
    std::uint64_t>
object::
hasher::
init(std::true_type) noexcept
{
    return {
        0x100000001B3ULL,
        0xcbf29ce484222325ULL
    };
}

std::pair<
    std::uint32_t,
    std::uint32_t>
object::
hasher::
init(std::false_type) noexcept
{
    return {
        0x01000193UL,
        0x811C9DC5UL
    };
}

std::size_t
object::
hasher::
operator()(key_type key) const noexcept
{
    std::size_t prime;
    std::size_t hash;
    std::tie(prime, hash) = init(
        std::integral_constant<bool,
            sizeof(std::size_t) >=
        sizeof(unsigned long long)>{});
    for(auto p = key.begin(),
        end = key.end(); p != end; ++p)
        hash = (*p ^ hash) * prime;
    return hash;
}

//------------------------------------------------------------------------------

struct object::cleanup_replace
{
    object& obj;
    table* tab;
    bool ok = false;

    explicit
    cleanup_replace(
        object& obj_)
        : obj(obj_)
        , tab(boost::exchange(
            obj_.tab_, nullptr))
    {
    }

    ~cleanup_replace()
    {
        if(! ok)
        {
            if(obj.tab_)
            {
                table::destroy_list(
                    obj.tab_, obj.sp_);
                table::destroy(
                    obj.tab_, obj.sp_);
            }
            obj.tab_ = tab;
        }
        else
        {
            if(tab)
            {
                table::destroy_list(
                    tab, obj.sp_);
                table::destroy(
                    tab, obj.sp_);
            }
        }
    }
};

//------------------------------------------------------------------------------
//
// Special Members
//
//------------------------------------------------------------------------------

object::
~object()
{
    if(tab_)
    {
        table::destroy_list(tab_, sp_);
        table::destroy(tab_, sp_);
    }
}

object::
object() noexcept
    : sp_(default_storage())
{
}

object::
object(storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
}

object::
object(
    size_type bucket_count)
    : object(
        bucket_count,
    default_storage())
{
}

object::
object(
    size_type bucket_count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    reserve(bucket_count);
}

object::
object(object&& other) noexcept
    : sp_(other.sp_)
    , tab_(boost::exchange(
        other.tab_, nullptr))
{
}

object::
object(pilfered<object> other) noexcept
    : sp_(std::move(other.get().sp_))
    , tab_(boost::exchange(
        other.get().tab_, nullptr))
{
}

object::
object(
    object&& other,
    storage_ptr sp) noexcept
    : sp_(std::move(sp))
{
    *this = std::move(other);
}

object::
object(
    object const& other)
    : object(other, other.get_storage())
{
}

object::
object(
    object const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    *this = other;
}

object::
object(
    std::initializer_list<value> init)
    : object(
        init,
        init.size(),
        default_storage())
{
}

object::
object(
    std::initializer_list<value> init,
    size_type bucket_count)
    : object(
        init,
        bucket_count,
        default_storage())
{
}

object::
object(
    std::initializer_list<value> init,
    storage_ptr sp)
    : object(
        init,
        init.size(),
        std::move(sp))
{
}      
        
object::
object(
    std::initializer_list<value> init,
    size_type bucket_count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    reserve(std::max<size_type>(
        bucket_count, init.size()));
    for(auto& e : init)
    {
        if(! e.is_key_value_pair())
            BOOST_THROW_EXCEPTION(
                std::invalid_argument(
                    "expected key/value pair"));
        emplace_impl(
            end(),
            e.as_array()[0].as_string(),
            std::move(e.as_array()[1]));
    }
}

object&
object::
operator=(object&& other)
{
    if(*other.sp_ == *sp_)
    {
        if(tab_)
        {
            table::destroy_list(tab_, sp_);
            table::destroy(tab_, sp_);
        }
        tab_ = other.tab_;
        other.tab_ = nullptr;
    }
    else
    {
        *this = const_cast<
            object const&>(other);
    }
    return *this;
}

object&
object::
operator=(object const& other)
{
    cleanup_replace c(*this);
    reserve(other.size());
    for(auto v : other)
        emplace_impl(end(), v.first,
            v.second);
    c.ok = true;
    return *this;
}

object&
object::
operator=(
    std::initializer_list<value> init)
{
    cleanup_replace c(*this);
    reserve(init.size());
    for(auto& e : init)
    {
        if(! e.is_key_value_pair())
            BOOST_THROW_EXCEPTION(
                std::invalid_argument(
                    "expected key/value pair"));
        emplace_impl(
            end(),
            e.as_array()[0].as_string(),
            std::move(e.as_array()[1]));
    }
    c.ok = true;
    return *this;
}

storage_ptr const&
object::
get_storage() const noexcept
{
    return sp_;
}

//------------------------------------------------------------------------------
//
// Iterators
//
//------------------------------------------------------------------------------

auto
object::
begin() noexcept ->
    iterator
{
    if(! tab_)
        return {};
    return tab_->head;
}

auto
object::
begin() const noexcept ->
    const_iterator
{
    if(! tab_)
        return {};
    return tab_->head;
}

auto
object::
cbegin() const noexcept ->
    const_iterator
{
    if(! tab_)
        return {};
    return tab_->head;
}

auto
object::
end() noexcept ->
    iterator
{
    if(! tab_)
        return {};
    return tab_->end();
}

auto
object::
end() const noexcept ->
    const_iterator
{
    if(! tab_)
        return {};
    return tab_->end();
}

auto
object::
cend() const noexcept ->
    const_iterator
{
    if(! tab_)
        return {};
    return tab_->end();
}

//------------------------------------------------------------------------------
//
// Capacity
//
//------------------------------------------------------------------------------

bool
object::
empty() const noexcept
{
    return ! tab_ ||
        tab_->count == 0;
}

auto
object::
size() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return tab_->count;
}

auto
object::
max_size() const noexcept ->
    size_type
{
    return (std::numeric_limits<
        size_type>::max)();
}

//------------------------------------------------------------------------------
//
// Modifiers
//
//------------------------------------------------------------------------------

void
object::
clear() noexcept
{
    if(! tab_)
        return;
    table::destroy_list(tab_, sp_);
    table::destroy(tab_, sp_);
    tab_ = nullptr;
}

void
object::
insert(
    std::initializer_list<value> init)
{
    reserve(size() + init.size());
    for(auto& e : init)
    {
        if(! e.is_key_value_pair())
            BOOST_THROW_EXCEPTION(
                std::invalid_argument(
                    "expected key/value pair"));
        emplace_impl(
            end(),
            e.as_array()[0].as_string(),
            std::move(e.as_array()[1]));
    }
}

auto
object::
insert(node_type&& nh) ->
    insert_return_type
{
    return insert(end(), std::move(nh));
}

auto
object::
insert(
    const_iterator before,
    node_type&& nh) ->
        insert_return_type
{
    if(! nh.e_)
        return { end(), false, {} };
    auto const hash = hasher{}(nh.e_->key());
    auto e = prepare_insert(
        &before, nh.key(), hash);
    if(e)
        return { iterator(e), false, std::move(nh) };
    e = nh.e_;
    finish_insert(before, e, hash);
    nh.e_ = nullptr;
    return { iterator(e), true, {} };
}

auto
object::
erase(const_iterator pos) ->
    iterator
{
    auto e = pos.e_->next_;
    remove(pos.e_);
    element::destroy(pos.e_, sp_);
    return e;
}

auto
object::
erase(
    const_iterator first,
    const_iterator last) ->
        iterator
{
    if(! tab_)
        return first.e_;
    auto e = first.e_;
    while(e != last.e_)
    {
        auto next = e->next_;
        remove(e);
        element::destroy(e, sp_);
        e = next;
    }
    return e;
}

auto
object::
erase(key_type key) ->
    size_type
{
    auto it = find(key);
    if(it == end())
        return 0;
    erase(it);
    return true;
}

void
object::
swap(object& other) noexcept
{
    // undefined if storage not equal
    BOOST_ASSERT(*sp_ == *other.sp_);
    std::swap(tab_, other.tab_);
    std::swap(mf_, other.mf_);
}

auto
object::
extract(const_iterator pos) ->
    node_type
{
    remove(pos.e_);
    return { pos.e_, sp_ };
}

auto
object::
extract(key_type key) ->
    node_type
{
    auto it = find(key);
    if(it == end())
        return {};
    return extract(it);
}

void
object::
merge(object&)
{
    // TODO
}

void
object::
merge(object&&)
{
    // TODO
}

//------------------------------------------------------------------------------
//
// Lookup
//
//------------------------------------------------------------------------------

auto
object::
at(key_type key) ->
    value&
{
    auto it = find(key);
    if(it == end())
        BOOST_THROW_EXCEPTION(
         std::out_of_range(
            "key not found"));
    return it->second;
}
    
auto
object::
at(key_type key) const ->
    value const&
{
    auto it = find(key);
    if(it == end())
        BOOST_THROW_EXCEPTION(
         std::out_of_range(
            "key not found"));
    return it->second;
}

auto
object::
operator[](key_type key) ->
    value&
{
    auto const result =
        emplace_impl(end(), key, kind::null);
    return result.first->second;
}

auto
object::
operator[](key_type key) const ->
    value const&
{
    return find(key)->second;
}

auto
object::
count(key_type key) const ->
    size_type
{
    if(find(key) == end())
        return 0;
    return 1;
}

auto
object::
count(
    key_type key,
    std::size_t hash) const ->
        size_type
{
    if(find(key, hash) == end())
        return 0;
    return 1;
}

auto
object::
find(key_type key) ->
    iterator
{
    return find(key, hasher{}(key));
}

auto
object::
find(
    key_type key,
    std::size_t hash) ->
        iterator
{
    auto it = static_cast<
        object const&>(*this).find(
            key, hash);
    return iterator(it.e_);
}

auto
object::
find(key_type key) const ->
    const_iterator
{
    return find(key, hasher{}(key));
}

auto
object::
find(
    key_type key,
    std::size_t hash) const ->
        const_iterator
{
    auto e = find_element(key, hash);
    if(! e)
        return end();
    return e;
}

bool
object::
contains(key_type key) const
{
    return find(key) != end();
}

bool
object::
contains(
    key_type key,
    std::size_t hash) const
{
    return find(key, hash) != end();
}

//------------------------------------------------------------------------------
//
// Bucket Interface
//
//------------------------------------------------------------------------------

auto
object::
begin(size_type n) noexcept ->
    local_iterator
{
    if(! tab_)
        return {};
    return tab_->bucket(n);
}

auto
object::
begin(size_type n) const noexcept ->
    const_local_iterator
{
    if(! tab_)
        return {};
    return tab_->bucket(n);
}

auto
object::
cbegin(size_type n) noexcept ->
    const_local_iterator
{
    if(! tab_)
        return {};
    return tab_->bucket(n);
}

auto
object::
end(size_type n)  noexcept ->
    local_iterator
{
    boost::ignore_unused(n);
    if(! tab_)
        return {};
    return tab_->end();
}

auto
object::
end(size_type n) const noexcept ->
    const_local_iterator
{
    boost::ignore_unused(n);
    if(! tab_)
        return {};
    return tab_->end();
}

auto
object::
cend(size_type n) noexcept ->
    const_local_iterator
{
    boost::ignore_unused(n);
    if(! tab_)
        return {};
    return tab_->end();
}

auto
object::
bucket_count() const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    return tab_->bucket_count;
}

auto
object::
max_bucket_count() const noexcept ->
    size_type
{
    return (std::numeric_limits<
        size_type>::max)();
}

auto
object::
bucket_size(size_type n) const noexcept ->
    size_type
{
    if(! tab_)
        return 0;
    size_type size = 0;
    for(auto e = tab_->bucket(n);
        e != tab_->end(); ++e)
        ++size;
    return size;
}

auto
object::
bucket(key_type key) const noexcept ->
    size_type
{
    if(! tab_)
        return 0; // undefined
    return hasher{}(key) %
        bucket_count();
}

//------------------------------------------------------------------------------

namespace detail {

struct primes
{
    using value_type = std::size_t;
    using iterator = std::size_t const*;

    std::size_t const* begin_;
    std::size_t const* end_;

    iterator
    begin() const noexcept
    {
        return begin_;
    }

    iterator
    end() const noexcept
    {
        return end_;
    }
};

// Taken from Boost.Intrusive and Boost.MultiIndex code,
// thanks to Ion Gaztanaga and Joaquin M Lopez Munoz.

template<class = void>
primes
get_primes(std::false_type) noexcept
{
    static std::size_t constexpr list[] = {
        0UL,

        3UL,                     7UL,
        11UL,                    17UL,
        29UL,                    53UL,
        97UL,                    193UL,
        389UL,                   769UL,
        1543UL,                  3079UL,
        6151UL,                  12289UL,
        24593UL,                 49157UL,
        98317UL,                 196613UL,
        393241UL,                786433UL,
        1572869UL,               3145739UL,
        6291469UL,               12582917UL,
        25165843UL,              50331653UL,
        100663319UL,             201326611UL,
        402653189UL,             805306457UL,
        1610612741UL,            3221225473UL,

        4294967291UL,            4294967295UL
    };
    return {
        &list[0],
        &list[std::extent<
            decltype(list)>::value] };
}

template<class = void>
primes
get_primes(std::true_type) noexcept
{
    static std::size_t constexpr list[] = {
        0ULL,

        3ULL,                     7ULL,
        11ULL,                    17ULL,
        29ULL,                    53ULL,
        97ULL,                    193ULL,
        389ULL,                   769ULL,
        1543ULL,                  3079ULL,
        6151ULL,                  12289ULL,
        24593ULL,                 49157ULL,
        98317ULL,                 196613ULL,
        393241ULL,                786433ULL,
        1572869ULL,               3145739ULL,
        6291469ULL,               12582917ULL,
        25165843ULL,              50331653ULL,
        100663319ULL,             201326611ULL,
        402653189ULL,             805306457ULL,
        1610612741ULL,            3221225473ULL,

        6442450939ULL,            12884901893ULL,
        25769803751ULL,           51539607551ULL,
        103079215111ULL,          206158430209ULL,
        412316860441ULL,          824633720831ULL,
        1649267441651ULL,         3298534883309ULL,
        6597069766657ULL,         13194139533299ULL,
        26388279066623ULL,        52776558133303ULL,
        105553116266489ULL,       211106232532969ULL,
        422212465066001ULL,       844424930131963ULL,
        1688849860263953ULL,      3377699720527861ULL,
        6755399441055731ULL,      13510798882111483ULL,
        27021597764222939ULL,     54043195528445957ULL,
        108086391056891903ULL,    216172782113783843ULL,
        432345564227567621ULL,    864691128455135207ULL,
        1729382256910270481ULL,   3458764513820540933ULL,
        6917529027641081903ULL,   13835058055282163729ULL,
        18446744073709551557ULL,  18446744073709551615ULL
    };
    return {
        &list[0],
        &list[std::extent<
            decltype(list)>::value] };
}

BOOST_JSON_DECL
primes
get_primes() noexcept
{
    return get_primes(
        std::integral_constant<bool,
            sizeof(std::size_t) >=
                sizeof(unsigned long long)>{});
}

} // detail

//------------------------------------------------------------------------------
//
// Hash Policy
//
//------------------------------------------------------------------------------

float
object::
load_factor() const noexcept
{
    if(! tab_)
        return 0;
    return static_cast<float>(
        size()) / bucket_count();
}

float
object::
max_load_factor() const
{
    return mf_;
}

void
object::
max_load_factor(float ml)
{
    mf_ = ml;
}

void
object::
rehash(size_type count)
{
    // snap to nearest prime 
    auto const primes =
        detail::get_primes();
    count = *std::lower_bound(
        primes.begin(), primes.end(), count);
    auto const bc = bucket_count();
    if(count == bc)
        return;
    if(count < bc)
    {
        count = (std::max<size_type>)(
            count, *std::lower_bound(
            primes.begin(), primes.end(),
            static_cast<size_type>(
                std::ceil(size() /
                max_load_factor()))));
        if(count >= bc)
            return;
    }
    // rehash
    //auto tab = table::construct(count, sp_);
    tab_ = table::allocate(
        tab_, count, sp_);
    for(auto e = tab_->head;
        e != tab_->end(); e = e->next_)
    {
        auto const n = bucket(e->key());
        auto& head = tab_->bucket(n);
        e->local_next_ = head;
        head = e;
    }
}

void
object::
reserve(size_type count)
{
    rehash(static_cast<
        size_type>(std::ceil(
            count / max_load_factor())));
}

//------------------------------------------------------------------------------

auto
object::
constrain_hash(
    std::size_t hash,
    size_type bucket_count) noexcept ->
        size_type
{
    return hash % bucket_count;
}

auto
object::
find_element(
    key_type key,
    std::size_t hash) const noexcept ->
        element*
{
    auto bc = bucket_count();
    if(bc == 0)
        return nullptr;
    auto e = tab_->bucket(
        constrain_hash(hash, bc));
    auto eq = key_eq();
    while(e != tab_->end())
    {
        if(eq(key, e->key()))
            return e;
        e = e->local_next_;
    }
    return nullptr;
}

auto
object::
prepare_insert(
    const_iterator* before,
    key_type key,
    std::size_t hash) ->
        element*
{
    auto bc = bucket_count();
    if(bc > 0)
    {
        auto e = find_element(key, hash);
        if(e)
            return e;
    }
    if(size() + 1 > bc * max_load_factor()
        || bc == 0)
    {
        auto const at_end = *before == end();
        rehash(static_cast<size_type>(
            (std::ceil)(
                float(size()+1) /
                max_load_factor())));
        if(at_end)
            *before = end();
    }
    return nullptr;
}

void
object::
finish_insert(
    const_iterator before,
    element* e,
    std::size_t hash)
{
    auto const bn = constrain_hash(
        hash, tab_->bucket_count);
    auto& head = tab_->bucket(bn);
    e->local_next_ = head;
    head = e;
    if(tab_->head == tab_->end())
    {
        BOOST_ASSERT(before.e_ == tab_->end());
        tab_->head = e;
        tab_->end()->prev_ = e;
        e->next_ = tab_->end();
    }
    else
    {
        e->prev_ = before.e_->prev_;
        e->next_ = before.e_;
        e->prev_->next_ = e;
        e->next_->prev_ = e;
        if(tab_->head == before.e_)
            tab_->head = e;
    }
    ++tab_->count;
}

void
object::
remove(element* e)
{
    if(e == tab_->head)
    {
        tab_->head = e->next_;
    }
    else
    {
        e->prev_->next_ = e->next_;
        e->next_->prev_ = e->prev_;
    }
    auto& head = tab_->bucket(
        bucket(e->key()));
    if(head != e)
    {
        auto it = head;
        BOOST_ASSERT(it != tab_->end());
        while(it->local_next_ != e)
        {
            it = it->local_next_;
            BOOST_ASSERT(it != tab_->end());
        }
        it->local_next_ = e->local_next_;
    }
    else
    {
        head = head->local_next_;
    }
    --tab_->count;
}

} // json
} // boost

#endif
