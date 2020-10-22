//
// Copyright (c) 2020 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

//[example_hash

/*
    This example demonstrates how to provide specializations of
    boost::hash and std::hash for a json value which provides compatibility
    with the behaviour of operator== so that json values can be safely stored in
    an unordered_map. Note that for small maps and large objects, the hashing function
    will dominate. But for large maps indexed by small json keys this could be useful.
*/

#include <boost/json.hpp>
#include <boost/functional/hash.hpp>
#include <iostream>
#include <unordered_map>

namespace boost {

// forward declarations

template <>
struct hash< json::array >
{
    inline
    std::size_t
    operator()(json::array const &arg) const;
};

template <>
struct hash< json::object >
{
    inline
    std::size_t
    operator()(json::object const &arg) const;
};

template <>
struct hash< json::string >
{
    inline
    std::size_t
    operator()(json::string const &arg) const;
};

template <>
struct hash< json::value >
{
    inline
    std::size_t
    operator()(json::value const &arg) const;
};

template <>
struct hash< std::nullptr_t >
{
    inline
    std::size_t
    operator()(std::nullptr_t) const
    {
        return 0;
    }
};

std::size_t
hash< json::value >::operator()(json::value const &arg) const
{
    using namespace boost::json;
    return json::visit([](auto&& item){
        return hash<std::decay_t<decltype(item)>>()(item);
    }, arg);
}

std::size_t hash< json::array >::operator()(json::array const &arg) const
{
    std::size_t seed = 0;
    for (auto &x : arg)
        hash_combine(seed, x);
    return seed;
}

std::size_t hash< json::object >::operator()(json::object const &arg) const
{
    std::size_t                                   seed = 0;
    thread_local std::vector< json::string_view > keys;
    keys.clear();
    for (auto &item : arg)
        keys.push_back(item.key());
    std::sort(keys.begin(), keys.end());
    for (auto &key : keys)
        hash_combine(seed, arg.at(key));
    return seed;
}

std::size_t hash< json::string >::operator()(json::string const &arg) const
{
    return hash_range(arg.begin(), arg.end());
}

} // namespace boost

namespace json = boost::json;

namespace std
{
    template<>
    struct hash<json::value>
    {
        std::size_t
        operator()(json::value const& v) const
        {
            return boost::hash<json::value>()(v);
        }
    };
}

int
main(int argc, char** argv)
{

    auto key1 = json::value{ {"name", "bob"}, {"age", 29} };
    auto key2 = json::value{ {"age", 29}, {"name", "bob"} }; // note: different order
    auto key3 = json::value{ {"name", "alice"}, {"age", 29} };

    auto equal_hash = [](json::value const& l, json::value const& r)
    {
        return std::hash<json::value>()(l) == std::hash<json::value>()(r);
    };

    std::cout << "key 1 equals key 2? " <<
        std::boolalpha << (key1 == key2) << '\n';

    std::cout << "key 1 hash equals key 2 hash? " <<
        std::boolalpha << equal_hash(key1, key2) << '\n';


    std::cout << "key 1 equals key 3? " <<
              std::boolalpha << (key1 == key3) << '\n';

    // note that the hashes could theoretically be equal and this would be
    // correct. However, it is somewhat unlikely.

    std::cout << "key 1 hash equals key 3 hash? " <<
              std::boolalpha << equal_hash(key1, key3) << '\n';

    std::cout << '\n';

    std::unordered_map<json::value, std::string> comments;
    comments.emplace(key1, "a diligent worker");
    comments.emplace(key2, "lazy and bad tempered");
    comments.emplace(key3, "a fine engineer");

    for (auto& item : comments)
        std::cout << item.first << " seems " << item.second << '\n';

    return EXIT_SUCCESS;
}

//]
