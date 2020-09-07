//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_DETAIL_IMPL_ARRAY_IMPL_HPP
#define BOOST_JSON_DETAIL_IMPL_ARRAY_IMPL_HPP

BOOST_JSON_NS_BEGIN
namespace detail {

constexpr
std::size_t
array_impl::
max_size() noexcept
{
    // max_size depends on the address model
    using min = std::integral_constant<std::size_t,
        (std::size_t(-1) - sizeof(table)) / sizeof(value)>;
    return min::value < BOOST_JSON_MAX_STRUCTURED_SIZE ?
        min::value : BOOST_JSON_MAX_STRUCTURED_SIZE;
}

auto
array_impl::
index_of(value const* pos) const noexcept ->
    std::size_t
{
    return static_cast<
        std::size_t>(pos - data());
}

//----------------------------------------------------------

unchecked_array::
~unchecked_array()
{
    if(! data_ ||
        sp_.is_not_counted_and_deallocate_is_trivial())
        return;
    for(unsigned long i = 0;
        i < size_; ++i)
        data_[i].~value();
}

void
unchecked_array::
relocate(value* dest) noexcept
{
    if(size_ > 0)
        std::memcpy(
            reinterpret_cast<void*>(dest),
            data_, size_ * sizeof(value));
    data_ = nullptr;
}

} // detail
BOOST_JSON_NS_END

#endif
