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

#include <boost/core/detail/static_assert.hpp>
#include <boost/describe/enum.hpp>
#include <boost/describe/class.hpp>

#include <boost/optional.hpp>

#include <array>
#include <map>
#include <unordered_map>
#include <vector>

#ifndef BOOST_NO_CXX17_HDR_FILESYSTEM
# include <filesystem>
#endif // BOOST_NO_CXX17_HDR_FILESYSTEM

#ifndef BOOST_NO_CXX17_HDR_OPTIONAL
# include <optional>
#endif // BOOST_NO_CXX17_HDR_OPTIONAL

#ifndef BOOST_NO_CXX17_HDR_VARIANT
# include <variant>
#endif // BOOST_NO_CXX17_HDR_VARIANT

#include "test_suite.hpp"

#ifdef __clang__
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused-private-field"
#endif

struct ctx0 {};
struct ctx1 {};

struct my_null1 { };
struct my_null2 { };
struct my_null3 { };

struct custom1 { };
struct custom2 { };
struct custom3 { };
struct custom4 { };
struct custom5 { };
struct custom6 { };
struct custom7 { };
struct custom8 { };
struct custom9 { };
struct custom10 { };
struct custom11 { };
struct custom12 { };
struct custom13 { };

void
tag_invoke(boost::json::value_from_tag, boost::json::value&, custom1 const&);
void
tag_invoke(boost::json::value_from_tag, boost::json::value&, custom10 const&);

void
tag_invoke(
    boost::json::value_from_tag,
    boost::json::value&,
    custom4 const&,
    ctx1 const&);
void
tag_invoke(
    boost::json::value_from_tag,
    boost::json::value&,
    custom11 const&,
    ctx1 const&);
void
tag_invoke(
    boost::json::value_from_tag,
    boost::json::value&,
    custom12 const&,
    ctx1 const&);
void
tag_invoke(
    boost::json::value_from_tag,
    boost::json::value&,
    custom13 const&,
    ctx0 const&);

template<class Ctx>
void
tag_invoke(
    boost::json::value_from_tag,
    boost::json::value&,
    custom7 const&,
    ctx1 const&,
    Ctx const&);

custom2
tag_invoke(boost::json::value_to_tag<custom2>, boost::json::value const&);
custom10
tag_invoke(boost::json::value_to_tag<custom10>, boost::json::value const&);

custom5
tag_invoke(
    boost::json::value_to_tag<custom5>,
    boost::json::value const&,
    ctx1 const&);
custom11
tag_invoke(
    boost::json::value_to_tag<custom11>,
    boost::json::value const&,
    ctx1 const&);
custom12
tag_invoke(
    boost::json::value_to_tag<custom12>,
    boost::json::value const&,
    ctx1 const&);
custom13
tag_invoke(
    boost::json::value_to_tag<custom13>,
    boost::json::value const&,
    ctx0 const&);

template<class Ctx>
custom8
tag_invoke(
    boost::json::value_to_tag<custom8>,
    boost::json::value const&,
    ctx1 const&,
    Ctx const&);

boost::system::result<custom3>
tag_invoke(boost::json::try_value_to_tag<custom3>, boost::json::value const&);

boost::system::result<custom6>
tag_invoke(
    boost::json::try_value_to_tag<custom6>,
    boost::json::value const&,
    ctx1 const&);

template<class Ctx>
boost::system::result<custom9>
tag_invoke(
    boost::json::try_value_to_tag<custom9>,
    boost::json::value const&,
    ctx1 const&,
    Ctx const&);

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

struct pseudo_sequence3 : pseudo_sequence1
{ };

struct pseudo_sequence4 : pseudo_sequence1
{ };

struct pseudo_sequence5 : pseudo_sequence1
{ };

void
tag_invoke(
    boost::json::value_from_tag, boost::json::value&, pseudo_sequence5 const&);
pseudo_sequence5
tag_invoke(
    boost::json::value_to_tag<pseudo_sequence5>, boost::json::value const&);

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

template <> struct use_category<my_null1> : null_category { };
template <> struct use_category<my_null2, ctx1> : null_category { };
template <> struct use_category<custom10> : null_category { };
template <> struct use_category<custom11> : null_category { };
template <> struct use_category<custom12, ctx1> : null_category { };
template <> struct use_category<custom13, ctx1> : null_category { };

template <>
struct is_null_like<my_null3> : std::true_type
{ };

template <>
struct is_string_like<pseudo_string2> : std::false_type
{ };

template <>
struct is_sequence_like<pseudo_sequence2> : std::false_type
{ };

template <> struct use_category<pseudo_sequence3> : map_category { };
template <> struct use_category<pseudo_sequence4, ctx1> : map_category { };

template <>
struct is_tuple_like<pseudo_tuple2> : std::false_type
{ };

template< class T, class Ctx = void >
using get_category1 = detail::get_conversion_category<
    T,
    Ctx,
    detail::all_custom_checks<detail::value_from_conversion>::fn,
    detail::all_fallback_checks<detail::value_from_conversion>::fn>;

template< class T, class Ctx = void >
using get_category2 = detail::get_conversion_category<
    T,
    Ctx,
    detail::all_custom_checks<detail::value_to_conversion>::fn,
    detail::all_fallback_checks<detail::value_to_conversion>::fn>;

template< class T, class Ctx = void >
using get_category3 = detail::get_conversion_category<
    T, Ctx, detail::direct_custom_checks, detail::direct_fallback_checks>;

class conversion_test
{
    template<class C, class Cat>
    static constexpr
    bool
    is_category()
    {
        return std::is_same<C, Cat>::value;
    }

    template<class C, class T, class Ctx, class Dir>
    static constexpr
    bool
    check_selected_context()
    {
        using Sup = detail::supported_context<Ctx, T, Dir>;
        using Get = decltype(Sup::get( std::declval<Ctx const&>() ));
        using R = detail::remove_cvref<Get>;
        return std::is_same<C, R>::value;
    }

    template< class T, class Cat >
    void test_deduced()
    {
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<T>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<T, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<T, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<T, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<T, std::tuple<ctx0, ctx1>>, Cat>() ));
    }

    template< class T, class Cat >
    void test_native()
    {
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<T, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<T, std::tuple<ctx0, ctx1>>, Cat>() ));
    }

    void test_variant2_deduced()
    {
        using Cat = variant_category;
        using variant = variant2::variant<int>;

        // These two fail, because Variant2 has tag_invoke overloads
        // BOOST_CORE_STATIC_ASSERT(
        //     is_category<get_category1<variant>, Cat>());
        // BOOST_CORE_STATIC_ASSERT(
        //     is_category<get_category2<variant>, Cat>());
        BOOST_CORE_STATIC_ASSERT((
                is_category<get_category3<variant>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<variant, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<variant, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<variant, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<variant, std::tuple<ctx0, ctx1>>, Cat>() ));
    }

    void
    test_customised()
    {
        test_deduced<my_null1, null_category>();

        using Cat = null_category;

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<my_null2>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<my_null2>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<my_null2>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<my_null2, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<my_null2, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<my_null2, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<my_null2, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<my_null2, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<my_null2, ctx1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<my_null2, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<my_null2, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<my_null2, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<my_null2, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<my_null2, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<my_null2, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<my_null2, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<my_null2, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<my_null2, std::tuple<ctx0, ctx1>>, Cat>() ));
    }

    void
    test_tag_invoke()
    {
        using Cat = detail::user_category;

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom1, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom1, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom1, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom1, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom1, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom1, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom1, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom1, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom1, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<custom1, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category2<custom1, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom1, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom1,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom1,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom2>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom2>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom2>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom2, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom2, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom2, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom2, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom2, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom2, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom2, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom2, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom2, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category1<custom2, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom2, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom2, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom2,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom2,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom3>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom3>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom3>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom3, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom3, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom3, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom3, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom3, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom3, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom3, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom3, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom3, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category1<custom3, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom3, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom3, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom3,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom3,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));
    }

    void
    test_tag_invoke_context()
    {
        using Cat = detail::user_context_category;

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom4>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom4>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom4>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom4, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom4, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom4, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom4, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom4, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom4, ctx1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom4, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom4, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom4, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom4, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom4, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom4, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<custom4, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category2<custom4, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom4, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx1,
                custom4,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom4,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom5>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom5>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom5>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom5, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom5, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom5, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom5, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom5, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom5, ctx1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom5, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom5, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom5, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom5, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom5, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom5, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category1<custom5, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom5, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom5, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom5,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx1,
                custom5,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom6>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom6>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom6>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom6, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom6, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom6, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom6, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom6, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom6, ctx1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom6, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom6, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom6, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom6, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom6, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom6, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category1<custom6, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom6, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom6, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom6,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx1,
                custom6,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));
    }

    void
    test_tag_invoke_full_context()
    {
        using Cat = detail::user_full_context_category;

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom7>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom7>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom7>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom7, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom7, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom7, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom7, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom7, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom7, ctx1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom7, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom7, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom7, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom7, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom7, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom7, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<custom7, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category2<custom7, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom7, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx1,
                custom7,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom7,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom8>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom8>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom8>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom8, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom8, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom8, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom8, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom8, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom8, ctx1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom8, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom8, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom8, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom8, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom8, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom8, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category1<custom8, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom8, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom8, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom8,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx1,
                custom8,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom9>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom9>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom9>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom9, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom9, ctx0>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom9, ctx0>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom9, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom9, ctx1>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom9, ctx1>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom9, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom9, std::tuple<>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom9, std::tuple<>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category1<custom9, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category2<custom9, std::tuple<ctx0>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<get_category3<custom9, std::tuple<ctx0>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category1<custom9, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom9, std::tuple<ctx0, ctx1>>, Cat>() ));
        BOOST_CORE_STATIC_ASSERT((
            !is_category<
                get_category3<custom9, std::tuple<ctx0, ctx1>>, Cat>() ));

        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx0,
                custom9,
                std::tuple<ctx0, ctx1>,
                detail::value_from_conversion>() ));
        BOOST_CORE_STATIC_ASSERT((
            check_selected_context<
                ctx1,
                custom9,
                std::tuple<ctx0, ctx1>,
                detail::value_to_conversion>() ));
    }

    void
    test_complex_customisations()
    {
        // the type is deduced as a sequence, but use_category makes it a map
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<pseudo_sequence3>, map_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<pseudo_sequence3>, map_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<pseudo_sequence3>, map_category>() ));

        // the type is deduced as a sequence, but use_category makes it a map
        // in context ctx1
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<pseudo_sequence4>, sequence_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<pseudo_sequence4>, sequence_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<pseudo_sequence4>, sequence_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<pseudo_sequence4, ctx1>, map_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<pseudo_sequence4, ctx1>, map_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<pseudo_sequence4, ctx1>, map_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<pseudo_sequence4, std::tuple<ctx0, ctx1>>,
                map_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<pseudo_sequence4, std::tuple<ctx0, ctx1>>,
                map_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<pseudo_sequence4, std::tuple<ctx0, ctx1>>,
                map_category>() ));

        // the type is deduced as a sequence, but there's a tag_invoke overload
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<pseudo_sequence5>, detail::user_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<pseudo_sequence5>, detail::user_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<pseudo_sequence5>, sequence_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<pseudo_sequence5, ctx1>,
                detail::user_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<pseudo_sequence5, ctx1>,
                detail::user_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<pseudo_sequence5, ctx1>,
                sequence_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<pseudo_sequence5, std::tuple<ctx0, ctx1>>,
                detail::user_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<pseudo_sequence5, std::tuple<ctx0, ctx1>>,
                detail::user_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<pseudo_sequence5, std::tuple<ctx0, ctx1>>,
                sequence_category>() ));

        // the type has a tag_invoke overload, but also has use_category
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom10>, null_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom10>, null_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<custom10>, null_category>() ));

        // the type has a tag_invoke overload in context ctx1, and
        // also has use_category (context-less)
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<custom11, ctx1>,
                detail::user_context_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom11, ctx1>,
                detail::user_context_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<custom11, ctx1>, null_category>() ));

        // the type has both a tag_invoke overload and use_category
        // in context ctx1
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category1<custom12, ctx1>, null_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category2<custom12, ctx1>, null_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<get_category3<custom12, ctx1>, null_category>() ));

        // the type has a tag_invoke overload in ctx0 and use_category
        // in context ctx1
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category1<custom13, std::tuple<ctx0, ctx1>>,
                detail::user_context_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category2<custom13, std::tuple<ctx0, ctx1>>,
                detail::user_context_category>() ));
        BOOST_CORE_STATIC_ASSERT((
            is_category<
                get_category3<custom13, std::tuple<ctx0, ctx1>>,
                null_category>() ));
    }

public:
    void
    run()
    {
        test_deduced<bool, detail::boolean_category>();
        test_deduced<int, detail::integer_category>();
        test_deduced<unsigned, detail::integer_category>();
        test_deduced<long, detail::integer_category>();
        test_deduced<double, detail::floating_point_category>();
        test_deduced<float, detail::floating_point_category>();
        test_deduced<std::nullptr_t, null_category>();
        test_deduced<std::string, string_category>();
        test_deduced<string_view, string_category>();
        test_deduced<pseudo_string1, string_category>();
        test_variant2_deduced();
        test_deduced<boost::optional<int>, optional_category>();
        test_deduced<std::map<std::string, int>, map_category>();
        test_deduced<std::unordered_map<std::string, int>, map_category>();
        test_deduced<pseudo_map1<pseudo_string1>, map_category>();
        test_deduced<std::vector<int>, sequence_category>();
        test_deduced<std::array<int, 4>, sequence_category>();
        test_deduced<pseudo_sequence1, sequence_category>();
        test_deduced<std::multimap<std::string, int>, sequence_category>();
        test_deduced<std::tuple<int>, tuple_category>();
        test_deduced<std::tuple<int, double>, tuple_category>();
        test_deduced<std::pair<int, int>, tuple_category>();
        test_deduced<pseudo_tuple1, tuple_category>();
#ifndef BOOST_NO_CXX17_HDR_VARIANT
        test_deduced<std::monostate, null_category>();
        test_deduced<std::variant<int>, variant_category>();
#endif // BOOST_NO_CXX17_HDR_VARIANT
#ifndef BOOST_NO_CXX17_HDR_OPTIONAL
        test_deduced<std::optional<int>, optional_category>();
#endif // BOOST_NO_CXX17_HDR_OPTIONAL
#ifdef BOOST_DESCRIBE_CXX14
        test_deduced<described1, described_class_category>();
        test_deduced<described2, described_class_category>();
        test_deduced<described5, described_class_category>();
        test_deduced<described6, described_class_category>();
        test_deduced<described_enum, described_enum_category>();
#endif // BOOST_DESCRIBE_CXX14
#ifndef BOOST_NO_CXX17_HDR_FILESYSTEM
        test_deduced<std::filesystem::path, path_category>();
#endif // BOOST_NO_CXX17_HDR_FILESYSTEM

        test_native<value_ref, detail::json_value_ref_category>();
        test_native<value, detail::json_value_category>();
        test_native<array, detail::json_array_category>();
        test_native<object, detail::json_object_category>();
        test_native<string, detail::json_string_category>();

        test_customised();
        test_tag_invoke();
        test_tag_invoke_context();
        test_tag_invoke_full_context();
        test_complex_customisations();

        BOOST_CORE_STATIC_ASSERT( !is_string_like<pseudo_string2>::value );

        BOOST_CORE_STATIC_ASSERT( is_sequence_like<pseudo_sequence1>::value );

        BOOST_CORE_STATIC_ASSERT( !is_tuple_like<pseudo_tuple2>::value );

        BOOST_CORE_STATIC_ASSERT(
            !is_map_like< pseudo_map1<pseudo_string2> >::value );
        BOOST_CORE_STATIC_ASSERT(
            !is_map_like< pseudo_multimap1<pseudo_string1> >::value );

#ifdef BOOST_DESCRIBE_CXX14
        BOOST_CORE_STATIC_ASSERT( !is_described_class<described3>::value );
        BOOST_CORE_STATIC_ASSERT( !is_described_class<described4>::value );
        BOOST_CORE_STATIC_ASSERT( !is_described_class<described7>::value );
        BOOST_CORE_STATIC_ASSERT( !is_described_class<described8>::value );
        BOOST_CORE_STATIC_ASSERT( !is_described_class<described9>::value );

        BOOST_CORE_STATIC_ASSERT( !is_described_enum<my_null3>::value );
        BOOST_CORE_STATIC_ASSERT( !is_described_enum<described1>::value );
        BOOST_CORE_STATIC_ASSERT( !is_described_enum<undescribed_enum>::value);
#endif

        BOOST_CORE_STATIC_ASSERT((
            std::is_same<
                detail::forwarded_value< std::vector<int>& >,
                int& >::value ));
        BOOST_CORE_STATIC_ASSERT((
            std::is_same<
                detail::forwarded_value< std::vector<int> const& >,
                int const& >::value ));
        BOOST_CORE_STATIC_ASSERT((
            std::is_same<
                detail::forwarded_value< std::vector<int>&& >,
                int >::value ));
        BOOST_CORE_STATIC_ASSERT((
            std::is_same<
                detail::forwarded_value< std::vector<bool>& >,
                bool >::value ));
    }
};

TEST_SUITE(conversion_test, "boost.json.conversion");

} // namespace json
} // namespace boost
