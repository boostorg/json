//
// Copyright (c) 2021 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

//
// An example that compares the performance of json::parse and
// json::parse_into on canada.json
//

#include <boost/json.hpp>

#if !defined(BOOST_DESCRIBE_CXX14)

#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE( "This example requires C++14" )

int main() {}

#else

#include <boost/describe.hpp>

#include <chrono>
#include <fstream>
#include <iostream>

#include "canada.hpp"

using namespace std::chrono_literals;
using canada = boost::json::canada::data;

int main()
{
    std::ifstream is( "canada.json" );
    std::string json( std::istreambuf_iterator<char>( is ), std::istreambuf_iterator<char>{} );

    std::cout << "canada.json: " << json.size() << " bytes\n";

    {
        auto tp1 = std::chrono::steady_clock::now();

        boost::json::value jv = boost::json::parse( json );

        auto tp2 = std::chrono::steady_clock::now();
        std::cout << "boost::json::parse: " << (tp2 - tp1) / 1ms << " ms\n";
    }

    {
        auto tp1 = std::chrono::steady_clock::now();

        canada w;

        boost::system::error_code ec;
        boost::json::parse_into( w, json, ec );

        if( ec.failed() )
        {
            std::cout << "Error: " << ec.what() << std::endl;
        }

        auto tp2 = std::chrono::steady_clock::now();
        std::cout << "parse_into<canada>: " << (tp2 - tp1) / 1ms << " ms\n";
    }
}

#endif
