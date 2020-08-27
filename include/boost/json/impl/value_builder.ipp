//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_VALUE_BUILDER_IPP
#define BOOST_JSON_IMPL_VALUE_BUILDER_IPP

#include <boost/json/value_builder.hpp>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace boost {
namespace json {

value_builder::
stack::
~stack()
{
    clear();
    if(begin_ != temp_)
        sp_->deallocate(
            begin_,
            (end_ - begin_) *
                sizeof(value));
}

value_builder::
stack::
stack(
    void* temp,
    std::size_t size,
    storage_ptr sp) noexcept
    : sp_(std::move(sp))
    , temp_(temp)
{
    if(size >= min_size_ *
        sizeof(value))
    {
        begin_ = reinterpret_cast<
            value*>(temp);
        top_ = begin_;
        end_ = begin_ +
            size / sizeof(value);
    }
    else
    {
        begin_ = nullptr;
        top_ = nullptr;
        end_ = nullptr;
    }
}

void
value_builder::
stack::
run_dtors(bool b) noexcept
{
    run_dtors_ = b;
}

std::size_t
value_builder::
stack::
size() const noexcept
{
    return top_ - begin_;
}

bool
value_builder::
stack::
has_part()
{
    return chars_ != 0;
}

//---

void
value_builder::
stack::
prepare()
{
    if(begin_)
        return;
    begin_ = reinterpret_cast<
        value*>(sp_->allocate(
            min_size_ * sizeof(value)));
    top_ = begin_;
    end_ = begin_ + min_size_;
}

// destroy the values but
// not the stack allocation.
void
value_builder::
stack::
clear() noexcept
{
    if(top_ != begin_)
    {
        if(run_dtors_)
            for(auto it = top_;
                it-- != begin_;)
                it->~value();
        top_ = begin_;
    }
    chars_ = 0;
}

// make room for at least one more value
void
value_builder::
stack::
grow_one()
{
    BOOST_ASSERT(begin_);
    BOOST_ASSERT(chars_ == 0);
    std::size_t const capacity =
        end_ - begin_;
    // must be power of 2
    BOOST_ASSERT((capacity &
        (capacity - 1)) == 0);
    // VFALCO check overflow here
    std::size_t new_cap = 2 * capacity;
    BOOST_ASSERT(
        new_cap - capacity >= 1);
    auto const begin =
        reinterpret_cast<value*>(
            sp_->allocate(
                new_cap * sizeof(value)));
    std::memcpy(
        reinterpret_cast<char*>(begin),
        reinterpret_cast<char*>(begin_),
        size() * sizeof(value));
    if(begin_ != temp_)
        sp_->deallocate(begin_,
            capacity * sizeof(value));
    // book-keeping
    top_ = begin + (top_ - begin_);
    end_ = begin + new_cap;
    begin_ = begin;
}

// make room for nchars additional characters.
void
value_builder::
stack::
grow(std::size_t nchars)
{
    BOOST_ASSERT(begin_);
    // needed capacity in values
    std::size_t const needed_cap =
        size() +
        1 +
        ((chars_ + nchars +
            sizeof(value) - 1) /
                sizeof(value));
    std::size_t const capacity =
        end_ - begin_;
    // must be power of 2
    BOOST_ASSERT((capacity &
        (capacity - 1)) == 0);
    BOOST_ASSERT(
        needed_cap > capacity);
    // VFALCO check overflow here
    std::size_t new_cap = capacity;
    while(new_cap < needed_cap)
        new_cap <<= 1;
    auto const begin = reinterpret_cast<
        value*>(sp_->allocate(
            new_cap * sizeof(value)));
    std::size_t amount =
        size() * sizeof(value);
    if(chars_ > 0)
        amount += sizeof(value) + chars_;
    std::memcpy(
        reinterpret_cast<char*>(begin),
        reinterpret_cast<char*>(begin_),
        amount);
    if(begin_ != temp_)
        sp_->deallocate(begin_,
            capacity * sizeof(value));
    // book-keeping
    top_ = begin + (top_ - begin_);
    end_ = begin + new_cap;
    begin_ = begin;
}

void
value_builder::
stack::
save(std::size_t n)
{
    BOOST_ASSERT(chars_ == 0);
    if(top_ >= end_)
        grow(0);
    // use default storage here to
    // avoid needless refcounting
    ::new(top_) value(n);
    ++top_;
}

void
value_builder::
stack::
append(string_view s)
{
    std::size_t const bytes_avail =
        reinterpret_cast<
            char const*>(end_) -
        reinterpret_cast<
            char const*>(top_);
    // make sure there is room for
    // pushing one more value without
    // clobbering the string.
    if(sizeof(value) + chars_ +
            s.size() > bytes_avail)
        grow(s.size());

    // copy the new piece
    std::memcpy(
        reinterpret_cast<char*>(
            top_ + 1) + chars_,
        s.data(), s.size());
    chars_ += s.size();

    // ensure a pushed value cannot
    // clobber the released string.
    BOOST_ASSERT(
        reinterpret_cast<char*>(
            top_ + 1) + chars_ <=
        reinterpret_cast<char*>(
            end_));
}

template<class... Args>
value&
value_builder::
stack::
push(Args&&... args)
{
    BOOST_ASSERT(chars_ == 0);
    if(top_ >= end_)
        grow_one();
    value& jv = detail::value_access::
        construct_value(top_,
            std::forward<Args>(args)...);
    ++top_;
    return jv;
}

//---

void
value_builder::
stack::
restore(std::size_t* n) noexcept
{
    BOOST_ASSERT(chars_ == 0);
    BOOST_ASSERT(top_ > begin_);
    auto p = --top_;
    BOOST_ASSERT(p->is_uint64());
    *n = p->get_uint64();
    //p->~value(); // not needed
}

string_view
value_builder::
stack::
release_string() noexcept
{
    // ensure a pushed value cannot
    // clobber the released string.
    BOOST_ASSERT(
        reinterpret_cast<char*>(
            top_ + 1) + chars_ <=
        reinterpret_cast<char*>(
            end_));
    auto const n = chars_;
    chars_ = 0;
    return { reinterpret_cast<
        char const*>(top_ + 1), n };
}

// transfer ownership of the top n
// elements of the stack to the caller
value*
value_builder::
stack::
release(std::size_t n) noexcept
{
    BOOST_ASSERT(n <= size());
    BOOST_ASSERT(chars_ == 0);
    top_ -= n;
    return top_;
}

//----------------------------------------------------------

value_builder::
~value_builder()
{
}

value_builder::
value_builder(
    storage_ptr sp) noexcept
    : value_builder(
        nullptr, 0, std::move(sp))
{
}

value_builder::
value_builder(
    void* temp_buffer,
    std::size_t temp_size,
    storage_ptr sp) noexcept
    : st_(
        temp_buffer,
        temp_size,
        std::move(sp))
{

}

void
value_builder::
reserve(std::size_t n)
{
#ifndef BOOST_NO_EXCEPTIONS
    try
    {
#endif
        // VFALCO TODO
        // st_.reserve(n);
        (void)n;
#ifndef BOOST_NO_EXCEPTIONS
    }
    catch(std::bad_alloc const&)
    {
        // silence the exception, per contract
    }
#endif
}

void
value_builder::
reset(storage_ptr sp) noexcept
{
    clear();
    sp_ = std::move(sp);
    st_.prepare();

    // `stack` needs this
    // to clean up correctly
    st_.run_dtors(
        ! sp_.is_not_counted_and_deallocate_is_null());
}

value
value_builder::
release()
{
    // give up shared ownership
    sp_ = {};

    if(st_.size() == 1)
        return pilfer(*st_.release(1));

    // This means the caller did not
    // cause a single top level element
    // to be produced.
    throw std::logic_error("no value");
}

void
value_builder::
clear() noexcept
{
    // give up shared ownership
    sp_ = {};

    st_.clear();
    top_ = 0;
}

//----------------------------------------------------------

void
value_builder::
begin_array()
{
    st_.save(top_);
    top_ = st_.size();
}

void
value_builder::
end_array()
{
    auto const n =
        st_.size() - top_;
    detail::unchecked_array ua(
        st_.release(n), n, sp_);
    st_.restore(&top_);
    st_.push(std::move(ua));
}

void
value_builder::
begin_object()
{
    st_.push(top_);
    top_ = st_.size();
}

void
value_builder::
end_object()
{
    // must be even
    BOOST_ASSERT(
        ((st_.size() - top_) & 1) == 0);

    auto const n =
        st_.size() - top_;
    detail::unchecked_object uo(
        st_.release(n), n/2, sp_);
    st_.restore(&top_);
    st_.push(std::move(uo));
}

void
value_builder::
insert_key_part(
    string_view s)
{
    st_.append(s);
}

void
value_builder::
insert_key(
    string_view s)
{
    if(! st_.has_part())
    {
        // fast path
        char* dest = nullptr;
        st_.push(&dest, s.size(), sp_);
        std::memcpy(
            dest, s.data(), s.size());
        return;
    }

    auto part = st_.release_string();
    char* dest = nullptr;
    st_.push(&dest,
        part.size() + s.size(), sp_);
    std::memcpy(dest,
        part.data(), part.size());
    std::memcpy(dest + part.size(),
        s.data(), s.size());
}

void
value_builder::
insert_string_part(
    string_view s)
{
    st_.append(s);
}

void
value_builder::
insert_string(
    string_view s)
{
    if(! st_.has_part())
    {
        // fast path
        st_.push(s, sp_);
        return;
    }

    // VFALCO We could add a special
    // private ctor to string that just
    // creates uninitialized space,
    // to reduce member function calls.
    auto part = st_.release_string();
    auto& str = st_.push(
        string_kind, sp_).get_string();
    str.reserve(
        part.size() + s.size());
    std::memcpy(
        str.data(),
        part.data(), part.size());
    std::memcpy(
        str.data() + part.size(),
        s.data(), s.size());
    str.grow(part.size() + s.size());
}

void
value_builder::
insert_int64(
    int64_t i)
{
    st_.push(i, sp_);
}

void
value_builder::
insert_uint64(
    uint64_t u)
{
    st_.push(u, sp_);
}

void
value_builder::
insert_double(
    double d)
{
    st_.push(d, sp_);
}

void
value_builder::
insert_bool(
    bool b)
{
    st_.push(b, sp_);
}

void
value_builder::
insert_null()
{
    st_.push(nullptr, sp_);
}

} // json
} // boost

#endif
