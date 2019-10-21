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
#include <boost/json/detail/exchange.hpp>
#include <boost/json/detail/assert.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace boost {
namespace json {

void
object::
element::
destroy(
    storage_ptr const& sp) const noexcept
{
    auto const size_ = size;
    this->~element();
    sp->deallocate(
        const_cast<element*>(this),
        sizeof(element) + size_ + 1,
        alignof(element));
}

//----------------------------------------------------------

void
object::
table::
destroy(
    storage_ptr const& sp) noexcept
{
    auto const n = bucket_count;
    this->~table();
    sp->deallocate(
        this,
        sizeof(*this) +
            n * sizeof(element*),
        alignof(table));
}

object::
table::
table(size_type bucket_count_) noexcept
    : size(0)
    , bucket_count(bucket_count_)
    , head(end())
{
    head->next = nullptr;
    head->prev = nullptr;
}

auto
object::
table::
construct(
    size_type bucket_count,
    storage_ptr const& sp) ->
        table*
{
    auto tab = ::new(sp->allocate(
        sizeof(table) +
            bucket_count *
            sizeof(element*),
        (std::max)(
            alignof(table),
            alignof(element*))
                )) table(bucket_count);
    auto it = &tab->bucket(0);
    auto const last =
        &tab->bucket(bucket_count);
    while(it != last)
        *it++ = tab->end();
    return tab;
}

//----------------------------------------------------------

object::
undo_range::
undo_range(object& self) noexcept
    : self_(self)
{
}

void
object::
undo_range::
insert(element* e) noexcept
{
    if(! head_)
    {
        head_ = e;
        tail_ = e;
        e->prev = nullptr;
    }
    else
    {
        e->prev = tail_;
        tail_->next = e;
        tail_ = e;
    }
    e->next = nullptr;
    ++n_;
}

object::
undo_range::
~undo_range()
{
    for(auto it = head_; it;)
    {
        auto e = it;
        it = it->next;
        e->destroy(self_.sp_);
    }
}

void
object::
undo_range::
commit(
    const_iterator pos,
    size_type count)
{
    if(head_ == nullptr)
        return;
    auto& tab = self_.tab_;
    auto before = pos.e_;

    // add space for n_ elements.
    //
    // this is the last allocation, so
    // we never have to clean it up on
    // an exception.
    //
    bool const at_end =
        before == nullptr ||
        before == tab->end();
    if( count < self_.size() + n_)
        count = self_.size() + n_;
    self_.reserve(count);
    // refresh `before`, which
    // may have been invalidated
    if(at_end)
        before = tab->end();

    // insert each item into buckets
    for(auto it = head_; it;)
    {
        auto const e = it;
        it = it->next;
        // discard dupes
        auto const result =
            self_.find_impl(e->key());
        if(result.first)
        {
            e->destroy(self_.sp_);
            continue;
        }
        // add to list
        e->next = before;
        e->prev = before->prev;
        before->prev = e;
        if(e->prev)
            e->prev->next = e;
        else
            tab->head = e;
        // add to bucket
        auto const bn = constrain_hash(
            result.second, tab->bucket_count);
        auto& local_head = tab->bucket(bn);
        e->local_next = local_head;
        local_head = e;
        ++tab->size;
    }
    // do nothing in dtor
    head_ = nullptr;
}

//----------------------------------------------------------

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

//----------------------------------------------------------

object::
const_iterator::
const_iterator(
    iterator it) noexcept
    : e_(it.e_)
{
}

//----------------------------------------------------------
//
// object
//
//----------------------------------------------------------

object::
~object()
{
    if(! tab_)
        return;
    for(auto e = tab_->head;
        e != tab_->end();)
    {
        auto next = e->next;
        e->destroy(sp_);
        e = next;
    }
    tab_->destroy(sp_);
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
    size_type count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    reserve(count);
}

object::
object(object&& other) noexcept
    : sp_(other.sp_)
    , tab_(detail::exchange(
        other.tab_, nullptr))
{
}

object::
object(
    object&& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    if(*sp_ == *other.sp_)
    {
        tab_ = detail::exchange(
            other.tab_, nullptr);
    }
    else
    {
        insert_range(
            end(),
            other.begin(),
            other.end(),
            0);
    }
}

object::
object(pilfered<object> other) noexcept
    : sp_(std::move(other.get().sp_))
    , tab_(detail::exchange(
        other.get().tab_, nullptr))
{
}

object::
object(
    object const& other)
    : object(
        other,
        other.get_storage())
{
}

object::
object(
    object const& other,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    insert_range(
        end(),
        other.begin(),
        other.end(),
        0);
}

object::
object(
    std::initializer_list<
        init_value> init,
    size_type count,
    storage_ptr sp)
    : sp_(std::move(sp))
{
    insert_range(
        end(),
        init.begin(),
        init.end(),
        count);
}

object&
object::
operator=(object&& other)
{
    object tmp(std::move(other), sp_);
    this->~object();
    ::new(this) object(pilfer(tmp));
    return *this;
}

object&
object::
operator=(object const& other)
{
    object tmp(other, sp_);
    this->~object();
    ::new(this) object(pilfer(tmp));
    return *this;
}

object&
object::
operator=(
    std::initializer_list<
        init_value> init)
{
    object tmp(init, sp_);
    this->~object();
    ::new(this) object(pilfer(tmp));
    return *this;
}

//----------------------------------------------------------
//
// Modifiers
//
//----------------------------------------------------------

void
object::
clear() noexcept
{
    object tmp(std::move(*this));
}

void
object::
insert(
    std::initializer_list<
        init_value> init)
{
    insert_range(
        end(),
        init.begin(),
        init.end(),
        0);
}

void
object::
insert(
    const_iterator pos,
    std::initializer_list<
        init_value> init)
{
    insert_range(
        pos,
        init.begin(),
        init.end(),
        0);
}

auto
object::
erase(const_iterator pos) noexcept ->
    iterator
{
    auto e = pos.e_;
    pos = e->next;
    remove(e);
    e->destroy(sp_);
    return iterator(pos.e_);
}

auto
object::
erase(
    const_iterator first,
    const_iterator last) noexcept ->
        iterator
{
    while(first != last)
    {
        auto e = first.e_;
        first = e->next;
        remove(e);
        e->destroy(sp_);
    }
    return iterator(first.e_);
}

auto
object::
erase(key_type key) noexcept ->
    size_type
{
    auto it = find(key);
    if(it == end())
        return 0;
    erase(it);
    return 1;
}

void
object::
swap(object& other)
{
    if(*sp_ == *other.sp_)
    {
        std::swap(tab_, other.tab_);
        return;
    }

    object temp1(
        std::move(*this),
        other.get_storage());
    object temp2(
        std::move(other),
        this->get_storage());
    other.~object();
    ::new(&other) object(pilfer(temp1));
    this->~object();
    ::new(this) object(pilfer(temp2));
}

//----------------------------------------------------------
//
// Lookup
//
//----------------------------------------------------------

auto
object::
at(key_type key) ->
    value&
{
    auto it = find(key);
    if(it == end())
        BOOST_JSON_THROW(
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
        BOOST_JSON_THROW(
         std::out_of_range(
            "key not found"));
    return it->second;
}

auto
object::
operator[](key_type key) ->
    value&
{
    auto const result = emplace(
        end(), key, kind::null);
    return result.first->second;
}

auto
object::
count(key_type key) const noexcept ->
    size_type
{
    if(find(key) == end())
        return 0;
    return 1;
}

auto
object::
find(key_type key) noexcept ->
    iterator
{
    auto const e =
        find_impl(key).first;
    if(e)
        return {e};
    return end();
}

auto
object::
find(key_type key) const noexcept ->
    const_iterator
{
    auto const e =
        find_impl(key).first;
    if(e)
        return {e};
    return end();
}

bool
object::
contains(key_type key) const noexcept
{
    return find(key) != end();
}

//----------------------------------------------------------
//
// Implementation
//
//----------------------------------------------------------

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
bucket(key_type key) const noexcept ->
    size_type
{
    BOOST_JSON_ASSERT(tab_);
    return constrain_hash(
        hash_function()(key),
        tab_->bucket_count);
}

// rehash to at least `n` buckets
void
object::
rehash(size_type n)
{
    auto const next_prime =
    [](size_type n) noexcept
    {
        // Taken from Boost.Intrusive and Boost.MultiIndex code,
        // thanks to Ion Gaztanaga and Joaquin M Lopez Munoz.
        static unsigned long long constexpr list[] = {
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
        return static_cast<size_type>(
            *std::lower_bound(
                &list[0],
                &list[std::extent<
                    decltype(list)>::value],
                (unsigned long long)n));
    };

    // snap to nearest prime 
    n = next_prime(n);
    auto const bc = tab_ ?
        tab_->bucket_count : 0;
    if(n == bc)
        return;
    if(n < bc)
    {
        n = next_prime(static_cast<
            size_type>(std::ceil(size() /
                max_load_factor())));

        if(n <= bc)
            return;
    }
    // create new buckets
    auto tab = table::construct(n, sp_);
    if(tab_)
    {
        tab->size = tab_->size;
        if(tab_->head != tab_->end())
        {
            tab->head = tab_->head;
            tab->end()->prev =
                tab_->end()->prev;
            tab->end()->prev->next =
                tab->end();
        }
        else
        {
            tab->head = tab->end();
        }
        tab_->destroy(sp_);
    }
    tab_ = tab;
    // rehash into new buckets
    for(auto e = tab_->head;
        e != tab_->end(); e = e->next)
    {
        auto const bn = bucket(e->key());
        auto& head = tab_->bucket(bn);
        e->local_next = head;
        head = e;
    }
}

void
object::
remove(element* e) noexcept
{
    if(e == tab_->head)
    {
        tab_->head = e->next;
    }
    else
    {
        e->prev->next = e->next;
        e->next->prev = e->prev;
    }
    auto& head = tab_->bucket(
        bucket(e->key()));
    if(head != e)
    {
        auto it = head;
        BOOST_JSON_ASSERT(it != tab_->end());
        while(it->local_next != e)
        {
            it = it->local_next;
            BOOST_JSON_ASSERT(it != tab_->end());
        }
        it->local_next = e->local_next;
    }
    else
    {
        head = head->local_next;
    }
    --tab_->size;
}

// allocate a new element
auto
object::
allocate_impl(
    key_type key,
    construct_base const& place_new) ->
        element*
{
    if( key.size() >
        detail::max_string_length_)
        BOOST_JSON_THROW(
            std::length_error("key too long"));
    auto const size =
        sizeof(element) +
        key.size() + 1;
    struct cleanup
    {
        void* p;
        std::size_t size;
        storage_ptr const& sp;

        ~cleanup()
        {
            if(p)
                sp->deallocate(p, size,
                    alignof(element));
        }
    };
    cleanup c{sp_->allocate(
        size, alignof(element)), size, sp_};
    place_new(c.p);
    char* p = static_cast<char*>(c.p);
    c.p = nullptr;
    std::memcpy(
        p + sizeof(element),
        key.data(),
        key.size());
    p[sizeof(element) +
        key.size()] = '\0';
    auto e = reinterpret_cast<
        element*>(p);
    e->size = static_cast<
        impl_size_type>(key.size());
    return e;
}

auto
object::
allocate(std::pair<
    string_view, value const&> const& p) ->
        element*
{
    return allocate(p.first, p.second);
}

auto
object::
find_impl(key_type key) const noexcept ->
    std::pair<element*, std::size_t>
{
    auto const hash = hash_function()(key);
    if(! tab_ || tab_->bucket_count == 0)
        return { nullptr, hash };
    auto bc = tab_->bucket_count;
    auto e = tab_->bucket(
        constrain_hash(hash, bc));
    auto const eq = key_eq();
    while(e != tab_->end())
    {
        if(eq(key, e->key()))
            return { e, hash };
        e = e->local_next;
    }
    return { nullptr, hash };
};

// destroys `e` on exception
void
object::
insert(
    const_iterator before,
    std::size_t hash,
    element* e)
{
    struct revert
    {
        element* e;
        storage_ptr const& sp;

        ~revert()
        {
            if(e)
                e->destroy(sp);
        }
    };
    revert r{e, sp_};

    // rehash if necessary
    if(size() >= capacity())
    {
        auto const at_end = before == end();
        rehash(static_cast<size_type>(
            (std::ceil)(
                float(size()+1) /
                max_load_factor())));
        if(at_end)
            before = end();
    }

    // perform insert
    auto const bn = constrain_hash(
        hash, tab_->bucket_count);
    auto& head = tab_->bucket(bn);
    e->local_next = head;
    head = e;
    if(tab_->head == tab_->end())
    {
        BOOST_JSON_ASSERT(
            before.e_ == tab_->end());
        tab_->head = e;
        tab_->end()->prev = e;
        e->next = tab_->end();
        e->prev = nullptr;
    }
    else
    {
        e->prev = before.e_->prev;
        if(e->prev)
            e->prev->next = e;
        else
            tab_->head = e;
        e->next = before.e_;
        e->next->prev = e;
    }
    ++tab_->size;
    r.e = nullptr;
}

void
swap(object& lhs, object& rhs)
{
    lhs.swap(rhs);
}

} // json
} // boost

#endif
