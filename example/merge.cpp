//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <cstdlib>
#include <iostream>

namespace boost {
namespace json {

/** Merge the elements from one object into another

    For each key in `src` this function attempts
    to move the entire key/value pair into `dest`
    if the key is not already present. Each
    element successfully moved is erased from
    `src`, making this operation destructive
    to `src`.

    @par Complexity
    Linear in `src.size()` best case,
    quadratic in `dest.size()+src.size()` worst case.

    @par Exception Safety
    Basic guarantee.
    Calls to allocate may throw.

    @param dest The object to merge into

    @param src The object to remove elements from
*/
object&
merge_move(
    object& dest,
    object&& src)
{
    dest.reserve(
        dest.size() + src.size());
    auto it = src.end();
    auto const begin = src.begin();
    while(it != begin)
    {
        --it;
        auto result =
            dest.insert(pilfer(*it));
        if(result.second)
            src.erase(it);
    }
    return dest;
}

/** Merge the elements from one object into another

    For each key in `src` this function attempts
    to copy the entire key/value pair into `dest`
    if the key is not already present.

    @par Complexity
    Linear in `src.size()` best case,
    quadratic in `dest.size()+src.size()` worst case.

    @par Exception Safety
    Basic guarantee.
    Calls to allocate may throw.

    @param dest The object to merge into

    @param src The object to remove elements from
*/
object&
merge_copy(
    object& dest,
    object const& src)
{
    dest.reserve(
        dest.size() + src.size());
    auto it = src.end();
    auto const begin = src.begin();
    while(it != begin)
    {
        --it;
        auto result =
            dest.insert(*it);
    }
    return dest;
}

} // json
} // boost

using boost::json::object;

static
object
make_B()
{
    object obj;
    obj["B"] = {{"property1", 1}};
    return obj;
}

static
object
make_C()
{
    object obj;
    obj["C"] = {
        {"property1", 1},
        {"property2", {1, 2, 3}}};
    return obj;
}

int
main(int, char**)
{
    object obj;
    obj["A"] = { "x", "y", "z" };
    merge_move(obj, make_B());
    merge_move(obj, make_C());

    std::cout << obj;
}
