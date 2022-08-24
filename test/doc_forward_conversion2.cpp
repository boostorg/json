//
// Copyright (c) 2021 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

namespace boost {
namespace json {

class value;

template< class T >
struct try_value_to_tag;

template< class T1, class T2 >
struct result_for;

template< class T >
typename result_for<T, value>::type
try_value_to( const value& jv );

}
}

#include <string>
#include <system_error>

namespace thirdparty {

struct customer
{
    std::uint64_t id;
    std::string name;
    bool late;

    customer() = default;

    customer( std::uint64_t i, const std::string& n, bool l )
        : id( i ), name( n ), late( l ) { }
};


template< class T, class JsonObject >
typename boost::json::result_for<T, boost::json::value>::type
get_element( const JsonObject& obj, char const* key )
{
    namespace json = boost::json;
    using Result = typename json::result_for<T, json::value>::type;

    auto const at_elem = obj.find(key);
    if( at_elem == obj.end() )
        return Result(
            Result::in_place_error,
            make_error_code(std::errc::invalid_argument));

    return boost::json::try_value_to< T >( at_elem->value() );
}

template< class JsonValue >
typename boost::json::result_for<customer, JsonValue>::type
tag_invoke(
    const boost::json::try_value_to_tag<customer>&,
    const JsonValue& jv)
{
    auto obj = jv.if_object();
    if( !obj )
        return make_error_code(std::errc::invalid_argument);

    auto id = get_element<std::uint64_t>( *obj, "id" );
    if( !id )
        return id.error();

    auto name = get_element<std::string>( *obj, "name" );
    if( !name )
        return name.error();

    auto late = get_element<bool>( *obj, "late" );
    if( !late )
        return late.error();

    return customer(*id, std::move(*name), *late);
}

}


#include <boost/json/value_to.hpp>

#include "test_suite.hpp"


BOOST_JSON_NS_BEGIN

class doc_forward_conversion2
{
public:
    void
    run()
    {
        value const jv{ { "id", 1 }, { "name", "Carl" }, { "late", true } };
        auto const c = try_value_to<thirdparty::customer>( jv ).value();
        BOOST_TEST( c.id == 1 );
        BOOST_TEST( c.name == "Carl" );
        BOOST_TEST( c.late );
    }
};

TEST_SUITE(doc_forward_conversion2, "boost.json.doc_forward_conversion2");

BOOST_JSON_NS_END
