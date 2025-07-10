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

#include <boost/describe/enum.hpp>
#include <boost/describe/class.hpp>

#include "test.hpp"
#include "test_suite.hpp"

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-private-field"
#endif

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

struct described1 { int n1; };
BOOST_DESCRIBE_STRUCT(described1, (), (n1))

struct described2 : described1 { int n2; };
BOOST_DESCRIBE_STRUCT(described2, (described1), (n2))

struct described3 : private described1 { int n3; };
BOOST_DESCRIBE_STRUCT(described3, (described1), (n3))

struct described4 : protected described1 { };
BOOST_DESCRIBE_STRUCT(described4, (described1), ())

struct described5 : private described2, protected described3 { };
BOOST_DESCRIBE_STRUCT(described5, (), ())

struct described6
{
    int n;

private:
    int m;
};
BOOST_DESCRIBE_STRUCT(described6, (), (n))

struct described7
{
    int n;

private:
    int m;
    BOOST_DESCRIBE_CLASS(described7, (), (n), (), (m))
};

struct described8
{
    int n;

protected:
    int m;
    BOOST_DESCRIBE_CLASS(described8, (), (n), (m), ())
};

union described9
{
    int n;
};
BOOST_DESCRIBE_STRUCT(described9, (), (n))

struct base1 {};
BOOST_DESCRIBE_STRUCT(base1, (), ())

struct base2 : private base1 {};
BOOST_DESCRIBE_STRUCT(base2, (base1), ())

struct described10 : base2 {};
BOOST_DESCRIBE_STRUCT(described10, (base2), ())

enum class described_enum { e };
BOOST_DESCRIBE_ENUM(described_enum, e)

enum class undescribed_enum { };

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

namespace boost {
namespace json {

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

#ifdef BOOST_DESCRIBE_CXX14
        BOOST_STATIC_ASSERT( is_described_class<described1>::value );
        BOOST_STATIC_ASSERT( is_described_class<described2>::value );
        BOOST_STATIC_ASSERT( is_described_class<described5>::value );
        BOOST_STATIC_ASSERT( is_described_class<described6>::value );

        BOOST_STATIC_ASSERT( !is_described_class<my_null>::value );
        BOOST_STATIC_ASSERT( !is_described_class<described3>::value );
        BOOST_STATIC_ASSERT( !is_described_class<described4>::value );
        BOOST_STATIC_ASSERT( !is_described_class<described7>::value );
        BOOST_STATIC_ASSERT( !is_described_class<described8>::value );
        BOOST_STATIC_ASSERT( !is_described_class<described9>::value );
        // BOOST_STATIC_ASSERT( !is_described_class<described10>::value );

        BOOST_STATIC_ASSERT( is_described_enum<described_enum>::value );
        BOOST_STATIC_ASSERT( !is_described_enum<my_null>::value );
        BOOST_STATIC_ASSERT( !is_described_enum<described1>::value );
        BOOST_STATIC_ASSERT( !is_described_enum<undescribed_enum>::value );
#endif

        BOOST_STATIC_ASSERT(
            std::is_same<
                detail::forwarded_value< std::vector<int>& >,
                int& >::value );
        BOOST_STATIC_ASSERT(
            std::is_same<
                detail::forwarded_value< std::vector<int> const& >,
                int const& >::value );
        BOOST_STATIC_ASSERT(
            std::is_same<
                detail::forwarded_value< std::vector<int>&& >,
                int >::value );
        BOOST_STATIC_ASSERT(
            std::is_same<
                detail::forwarded_value< std::vector<bool>& >,
                bool >::value );
    }
};

TEST_SUITE(conversion_test, "boost.json.conversion");

} // namespace json
} // namespace boost
