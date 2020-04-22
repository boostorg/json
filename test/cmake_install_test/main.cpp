// Copyright 2018 Peter Dimov
// Copyright 2020 Richard Hodges
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#include <boost/json.hpp>
#include <iostream>

int main()
{
    boost::json::error_code err = boost::json::error::expected_colon;
    std::cout << err.message() << std::endl;
}