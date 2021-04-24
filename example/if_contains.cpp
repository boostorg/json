//
// Copyright (c) 2021 Maksymilian Palka (maksymilian.palka@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json.hpp>

using namespace boost::json;

const value *
if_contains(value const *jv, std::size_t index)
{
    if (jv && jv->is_array())
    {
        return jv->as_array().if_contains(index);
    }
    return nullptr;
}

const value *
if_contains(value const *jv, string_view key)
{
    if (jv && jv->is_object())
        return jv->as_object().if_contains(key);
    return nullptr;
}

template<class Arg0, class Arg1, class... Args>
const value *
if_contains(value const *jv, Arg0 const &arg0, Arg1 const &arg1, Args const &... args)
{
    return if_contains(if_contains(jv, arg0), arg1, args...);
}

int
main(int, char **)
{
    const auto jv = boost::json::parse(R"(
    {
        "k1": "v1",
        "nested": {
            "array": [{
                "array.1": "va1"
            },
            {
                "array.2": "va2",
                "array.2.o": {
                    "arr_obj": "arr_obj"
                }
            },
            "arr_str"
            ]
        }
    })");

    bool ok = if_contains(&jv, "k1");
    ok = ok && if_contains(&jv, "k2") == nullptr;

    ok = ok && if_contains(&jv, "nested");
    ok = ok && if_contains(&jv, "nested", "array");
    ok = ok && if_contains(&jv, "nested", "array2") == nullptr;

    ok = ok && if_contains(&jv, "nested", "array", 0);
    ok = ok && if_contains(&jv, "nested", "array", 0, "array.1");
    ok = ok && if_contains(&jv, "nested", "array", 1);
    ok = ok && if_contains(&jv, "nested", "array", 2);
    ok = ok && if_contains(&jv, "nested", "array", 3) == nullptr;
    ok = ok && if_contains(&jv, "nested", "array", 1, "array.2.o");
    ok = ok && if_contains(&jv, "nested", "array", 1, "array.2.o", "arr_obj");

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
