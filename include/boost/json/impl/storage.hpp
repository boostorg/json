//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_HPP
#define BOOST_JSON_IMPL_STORAGE_HPP

#include <boost/json/detail/storage_adaptor.hpp>

namespace boost {
namespace beast {
namespace json {

template<class Allocator>
storage_ptr
make_storage_ptr(Allocator const& a)
{
    return storage_ptr(
        new detail::storage_adaptor<Allocator>(a));
}

} // json
} // beast
} // boost

#endif
