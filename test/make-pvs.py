# Build test vector variable

import os

def do_files(directory):
    for root, directories, files in os.walk(directory):
        for filename in files:
            filepath = os.path.join(root, filename)
            with open(filepath, 'r') as file:
                data = file.read();
                data = data.replace('\"', "\\\"");
                print("        { '" + filename[0:1] + "', \"" + filename[2:-5] + "\", R\"json(" + data + ")json\" },");

print("""
//
// Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef PARSE_VECTORS
#define PARSE_VECTORS

#include <boost/utility/string_view.hpp>
#include <cstdlib>
#include <type_traits>

struct parse_vectors
{
    struct item
    {
        char result;
        ::boost::string_view name;
        ::boost::string_view text;
    };

    using iterator = item const*;

    iterator begin() const noexcept
    {
        return first_;
    }

    iterator end() const noexcept
    {
        return last_;
    }

    std::size_t
    size() const noexcept
    {
        return last_ - first_;
    }

    inline parse_vectors() noexcept;

private:
    iterator first_;
    iterator last_;
};

parse_vectors::
parse_vectors() noexcept
{
    static item const list[] = {""");

do_files("parse-vectors");

print("""        { ' ', "", "" }
    };
    first_ = &list[0];
    last_ = &list[std::extent<
        decltype(list)>::value - 1];
}

#endif""");
