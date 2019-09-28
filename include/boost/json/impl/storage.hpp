//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_IMPL_STORAGE_HPP
#define BOOST_JSON_IMPL_STORAGE_HPP

#include <boost/json/detail/storage_adaptor.hpp>
#include <utility>

namespace boost {
namespace json {

template<class Storage, class... Args>
storage_ptr
make_storage_ptr(Args&&... args)
{
    return std::make_shared<Storage>(
        std::forward<Args>(args)...);
}

template<class Allocator>
storage_ptr
make_storage_adaptor(Allocator const& a)
{
    return std::make_shared<
        detail::storage_adaptor<Allocator>>(a);
}

} // json
} // boost

#endif
