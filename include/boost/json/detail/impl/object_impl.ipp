//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_IPP
#define BOOST_JSON_DETAIL_IMPL_OBJECT_IMPL_IPP

namespace boost {
namespace json {
namespace detail {

void
object_impl::
do_destroy(storage_ptr const& sp) noexcept
{
    if(tab_)
    {
        detail::destroy(begin(), size());
        sp->deallocate(tab_,
            sizeof(table) +
            capacity() * sizeof(value_type) +
            buckets() * sizeof(index_t));
    }
}

inline
object_impl::
object_impl(
    std::size_t capacity,
    std::size_t prime_index,
    std::uintptr_t salt,
    storage_ptr const& sp)
{
    const auto n = 
        sizeof(table) + 
        capacity *
            (sizeof(value_type) + 
            sizeof(index_t));
    tab_ = ::new(sp->allocate(n)) 
        table{0, capacity, prime_index, salt};
    // capacity == buckets()
    std::memset(bucket_begin(), 0xff, // null_index
        capacity * sizeof(index_t));
}

object_impl::
object_impl(object_impl&& other) noexcept
    : tab_(detail::exchange(
        other.tab_, nullptr))
{
}

void
object_impl::
clear() noexcept
{
    if(! tab_)
        return;
    detail::destroy(begin(), size());
    std::memset(bucket_begin(), 0xff, // null_index
        buckets() * sizeof(index_t));
    tab_->size = 0;
}

// checks for dupes
void
object_impl::
build() noexcept
{
    // must work when table pointer is null
    auto const first = begin();
    for(auto last = end(); last > first;)
    {
        --last;
        auto head = &bucket(last->key());
        auto i = *head;
        while(i != null_index &&
            get(i).key() != last->key())
            i = next(get(i));
        if(i != null_index)
        {
            // handle duplicate
            last->~value_type();
            std::memcpy(
                static_cast<void*>(last),
                static_cast<void const*>(
                    first + tab_->size - 1),
                sizeof(*last));
            --tab_->size;
            head = &bucket(last->key());
        }
        next(*last) = *head;
        *head = index_of(*last);
    }
}

// does not check for dupes
void
object_impl::
rebuild() noexcept
{
    auto const end = this->end();
    for(auto p = begin();
        p != end; ++p)
    {
        auto& head = bucket(p->key());
        next(*p) = head;
        head = index_of(*p);
    }
}

void
object_impl::
swap(object_impl& rhs) noexcept
{
    auto tmp = tab_;
    tab_ = rhs.tab_;
    rhs.tab_ = tmp;
}

//----------------------------------------------------------

void
destroy(
    key_value_pair* p,
    std::size_t n) noexcept
{
    // VFALCO We check again here even
    // though some callers already check it.
    if(n == 0 || ! p)
        return;
    auto const& sp = p->value().storage();
    if(sp.is_not_counted_and_deallocate_is_null())
        return;
    p += n;
    while(n--)
        (*--p).~key_value_pair();
}

} // detail
} // json
} // boost

#endif
