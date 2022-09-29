//
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

// test that header file is self-contained
#include <boost/json/conversion.hpp>
// test that header file is header-guarded properly
#include <boost/json/conversion.hpp>

#include "test.hpp"
#include "test_suite.hpp"

namespace
{

struct pseudo_string1
{
    operator boost::json::string_view();
};

struct pseudo_string2 : pseudo_string1
{ };

struct pseudo_sequence1
{
    int* begin();
    int* end();
};

struct pseudo_sequence2 : pseudo_sequence1
{ };

struct pseudo_tuple1
{ };

struct pseudo_tuple2
{ };

template<class Key>
struct pseudo_map1
{
    std::pair<Key, int>* begin();
    std::pair<Key, int>* end();

    std::pair< std::pair<Key, int>*, bool >
    emplace(std::pair<Key, int>);

};

template<class Key>
struct pseudo_multimap1
{
    std::pair<Key, int>* begin();
    std::pair<Key, int>* end();

    std::pair<Key, int>*
    emplace(std::pair<Key, int>);

};

struct my_null { };

} // namespace

namespace std
{

// some versions of libstdc++ forward-declare tuple_size as class
#if defined(__clang__) || ( defined(__GNUC__) && __GNUC__ >= 10 )
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmismatched-tags"
#endif
template<>
struct tuple_size<pseudo_tuple1> : std::integral_constant<std::size_t, 2>
{ };
#if defined(__clang__) || ( defined(__GNUC__) && __GNUC__ >= 10 )
# pragma GCC diagnostic pop
#endif

} // namespace std

BOOST_JSON_NS_BEGIN

template <>
struct is_string_like<pseudo_string2> : std::false_type
{ };

template <>
struct is_sequence_like<pseudo_sequence2> : std::false_type
{ };

template <>
struct is_tuple_like<pseudo_tuple2> : std::false_type
{ };

template <>
struct is_null_like<my_null> : std::true_type
{ };

class conversion_test
{
public:
    void
    run()
    {
        BOOST_STATIC_ASSERT( is_string_like<pseudo_string1>::value );
        BOOST_STATIC_ASSERT( !is_string_like<pseudo_string2>::value );

        BOOST_STATIC_ASSERT( is_sequence_like<pseudo_sequence1>::value );
        BOOST_STATIC_ASSERT( !is_sequence_like<pseudo_sequence2>::value );

        BOOST_STATIC_ASSERT( is_tuple_like<pseudo_tuple1>::value );
        BOOST_STATIC_ASSERT( !is_tuple_like<pseudo_tuple2>::value );

        BOOST_STATIC_ASSERT(
            is_map_like< pseudo_map1<pseudo_string1> >::value );
        BOOST_STATIC_ASSERT(
            !is_map_like< pseudo_map1<pseudo_string2> >::value );
        BOOST_STATIC_ASSERT(
            !is_map_like< pseudo_multimap1<pseudo_string1> >::value );

        BOOST_STATIC_ASSERT( is_null_like<std::nullptr_t>::value );
        BOOST_STATIC_ASSERT( is_null_like<my_null>::value );
    }
};

TEST_SUITE(conversion_test, "boost.json.conversion");

BOOST_JSON_NS_END
