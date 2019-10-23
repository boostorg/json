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

#include <utility>

namespace boost {
namespace json {

template<class Storage, class... Args>
basic_storage_ptr<Storage>
make_storage(Args&&... args)
{
    return basic_storage_ptr<Storage>(
        new Storage(std::forward<Args>(args)...));
}

} // json
} // boost

#endif
