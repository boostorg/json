//
// Copyright (c) 2021 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <chrono>

struct coordinate
{
    double x{}, y{}, z{};
    std::string name;
};

BOOST_DESCRIBE_STRUCT(coordinate, (), (x, y, z, name))

struct coordinates
{
    std::vector<coordinate> coordinates;
    std::string info;
};

BOOST_DESCRIBE_STRUCT(coordinates, (), (coordinates, info))

template<class T,
    class D1 = boost::describe::describe_members<T,
        boost::describe::mod_public | boost::describe::mod_protected>,
    class D2 = boost::describe::describe_members<T, boost::describe::mod_private>,
    class En = std::enable_if_t<boost::mp11::mp_empty<D2>::value> >

    void tag_invoke( boost::json::value_from_tag const&, boost::json::value& v, T const & t )
{
    auto& obj = v.emplace_object();

    boost::mp11::mp_for_each<D1>([&](auto D){

        obj[ D.name ] = boost::json::value_from( t.*D.pointer );

    });
}

template<class T,
    class Bd = boost::describe::describe_bases<T, boost::describe::mod_any_access>,
    class Md = boost::describe::describe_members<T, boost::describe::mod_any_access>>
    bool operator==( T const& t1, T const& t2 )
{
    bool r = true;

    boost::mp11::mp_for_each<Bd>([&](auto D){

        using B = typename decltype(D)::type;
        r = r && (B const&)t1 == (B const&)t2;

    });

    boost::mp11::mp_for_each<Md>([&](auto D){

        r = r && t1.*D.pointer == t2.*D.pointer;

    });

    return r;
}

struct accumulator
{
    using value_type = coordinate;

    std::size_t len = 0;

    double x = 0;
    double y = 0;
    double z = 0;

    void push_back( coordinate const& v )
    {
        x += v.x;
        y += v.y;
        z += v.z;

        ++len;
    }
};

struct coordinates2
{
    accumulator coordinates;
    std::string info;
};

BOOST_DESCRIBE_STRUCT(coordinates2, (), (coordinates, info))

using namespace std::chrono_literals;

int main()
{

    // using T = std::map<std::string, int>;
    // T t1{ { "one", 1 }, { "two", 2 }, { "three", 3 } };

    // using T = std::vector< std::vector<int> >;
    // T t1{ {}, {1}, {2, 3}, {4, 5, 6} };

    // using T = std::map<std::string, std::vector<int>>;
    // T t1{ { "one", {1} }, { "two", {2, 2} }, { "three", {3, 3, 3} } };

    // using T = std::map<std::string, std::pair<int, int>>;
    // T t1{ { "one", { 1, 2 } }, { "two", { 3, 4 } } };

    // using T = std::vector<coordinate>;
    // T t1{ { 1, 1, 1, "1" }, { 2, 2, 2, "2" } };
/*
    using T = coordinates;
    T t1{};
    std::string json = boost::json::serialize( boost::json::value_from( t1 ) );
    std::cout << "json: " << json << std::endl;
    T t2;
    boost::json::error_code ec;
    parse_into( t2, json, ec );
    if( ec.failed() )
    {
        std::cout << "Error: " << ec.what();
        return -1;
    }
    std::string json2 = boost::json::serialize( boost::json::value_from( t2 ) );
    std::cout << "json2: " << json2 << std::endl;
    if( t1 != t2 )
    {
        std::cout << "Mismatch!" << std::endl;
    }
    else
    {
        std::cout << "Match!" << std::endl;
    }
*/

    std::ifstream is( "/tmp/1.json" );
    std::string json( std::istreambuf_iterator<char>( is ), std::istreambuf_iterator<char>{} );

    std::cout << "1.json: " << json.size() << " bytes\n";

    {
        auto tp1 = std::chrono::steady_clock::now();

        boost::json::value jv = boost::json::parse( json );

        auto tp2 = std::chrono::steady_clock::now();
        std::cout << "boost::json::parse: " << (tp2 - tp1) / 1ms << " ms\n";

        auto x = 0.0, y = 0.0, z = 0.0;
        auto len = 0;

        auto &obj = jv.get_object();

        for( auto& v: obj["coordinates"].get_array() )
        {
            ++len;
            auto& coord = v.get_object();
            x += coord["x"].get_double();
            y += coord["y"].get_double();
            z += coord["z"].get_double();
        }

        x /= len;
        y /= len;
        z /= len;

        auto tp3 = std::chrono::steady_clock::now();
        std::cout << "  x: " << x << ", y: " << y << ", z: " << z << ": " << (tp3 - tp2) / 1ms << " ms\n";
    }

    {
        auto tp1 = std::chrono::steady_clock::now();

        coordinates w;

        boost::json::error_code ec;
        boost::json::parse_into( w, json, ec );

        if( ec.failed() )
        {
            std::cout << "Error: " << ec.what() << std::endl;
        }

        auto tp2 = std::chrono::steady_clock::now();
        std::cout << "parse_into coordinates: " << (tp2 - tp1) / 1ms << " ms\n";

        auto x = 0.0, y = 0.0, z = 0.0;
        auto len = 0;

        for( auto const& v: w.coordinates )
        {
            x += v.x;
            y += v.y;
            z += v.z;

            ++len;
        }

        x /= len;
        y /= len;
        z /= len;

        auto tp3 = std::chrono::steady_clock::now();
        std::cout << "  x: " << x << ", y: " << y << ", z: " << z << ": " << (tp3 - tp2) / 1ms << " ms\n";
    }

    {
        auto tp1 = std::chrono::steady_clock::now();

        coordinates2 w;

        boost::json::error_code ec;
        boost::json::parse_into( w, json, ec );

        if( ec.failed() )
        {
            std::cout << "Error: " << ec.what() << std::endl;
        }

        auto tp2 = std::chrono::steady_clock::now();
        std::cout << "parse_into coordinates2: " << (tp2 - tp1) / 1ms << " ms\n";

        double x = w.coordinates.z / w.coordinates.len;
        double y = w.coordinates.y / w.coordinates.len;
        double z = w.coordinates.z / w.coordinates.len;

        auto tp3 = std::chrono::steady_clock::now();
        std::cout << "  x: " << x << ", y: " << y << ", z: " << z << ": " << (tp3 - tp2) / 1ms << " ms\n";
    }
}