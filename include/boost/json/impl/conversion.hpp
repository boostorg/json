//
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_CONVERSION_HPP
#define BOOST_JSON_IMPL_CONVERSION_HPP

#include <boost/json/fwd.hpp>
#include <boost/json/string_view.hpp>
#include <boost/describe/enumerators.hpp>
#include <boost/describe/members.hpp>
#include <boost/describe/bases.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/utility.hpp>
#include <boost/system/result.hpp>

#include <iterator>
#include <tuple>
#include <utility>

#ifndef BOOST_NO_CXX17_HDR_VARIANT
# include <variant>
#endif // BOOST_NO_CXX17_HDR_VARIANT

#ifdef BOOST_JSON_HAS_REFLECTION
# include <meta>
#endif // BOOST_JSON_HAS_REFLECTION

namespace boost {
namespace json {

class value_ref;

namespace detail {

#ifdef __cpp_lib_nonmember_container_access
using std::size;
#endif

template<std::size_t I, class T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

template<class T>
using iterator_type = decltype(std::begin(std::declval<T&>()));
template<class T>
using iterator_traits = std::iterator_traits< iterator_type<T> >;

template<class T>
using value_type = typename iterator_traits<T>::value_type;
template<class T>
using mapped_type = tuple_element_t< 1, value_type<T> >;

// had to make the metafunction always succeeding in order to make it work
// with msvc 14.0
template<class T>
using key_type_helper = tuple_element_t< 0, value_type<T> >;
template<class T>
using key_type = mp11::mp_eval_or<
    void,
    key_type_helper,
    T>;

template<class T>
using are_begin_and_end_same = std::is_same<
    iterator_type<T>,
    decltype(std::end(std::declval<T&>()))>;

// msvc 14.0 gets confused when std::is_same is used directly
template<class A, class B>
using is_same_msvc_140 = std::is_same<A, B>;
template<class T>
using is_its_own_value = is_same_msvc_140<value_type<T>, T>;

template<class T>
using not_its_own_value = mp11::mp_not< is_its_own_value<T> >;

template<class T>
using begin_iterator_category = typename std::iterator_traits<
    iterator_type<T>>::iterator_category;

template<class T>
using has_positive_tuple_size = mp11::mp_bool<
    (std::tuple_size<T>::value > 0) >;

template<class T>
using has_unique_keys = has_positive_tuple_size<decltype(
    std::declval<T&>().emplace(
        std::declval<value_type<T>>()))>;

template<class T>
using has_string_type = std::is_same<
    typename T::string_type, std::basic_string<typename T::value_type> >;

template<class T>
struct is_value_type_pair_helper : std::false_type
{ };
template<class T1, class T2>
struct is_value_type_pair_helper<std::pair<T1, T2>> : std::true_type
{ };
template<class T>
using is_value_type_pair = is_value_type_pair_helper<value_type<T>>;

template<class T>
using has_size_member_helper
    = std::is_convertible<decltype(std::declval<T&>().size()), std::size_t>;
template<class T>
using has_size_member = mp11::mp_valid_and_true<has_size_member_helper, T>;
template<class T>
using has_free_size_helper
    = std::is_convertible<
        decltype(size(std::declval<T const&>())),
        std::size_t>;
template<class T>
using has_free_size = mp11::mp_valid_and_true<has_free_size_helper, T>;
template<class T>
using size_implementation = mp11::mp_cond<
    has_size_member<T>, mp11::mp_int<3>,
    has_free_size<T>,   mp11::mp_int<2>,
    std::is_array<T>,   mp11::mp_int<1>,
    mp11::mp_true,      mp11::mp_int<0>>;

template<class T>
std::size_t
try_size(T&& cont, mp11::mp_int<3>)
{
    return cont.size();
}

template<class T>
std::size_t
try_size(T& cont, mp11::mp_int<2>)
{
    return size(cont);
}

template<class T, std::size_t N>
std::size_t
try_size(T(&)[N], mp11::mp_int<1>)
{
    return N;
}

template<class T>
std::size_t
try_size(T&, mp11::mp_int<0>)
{
    return 0;
}

template<class T>
using has_push_back_helper
    = decltype(std::declval<T&>().push_back(std::declval<value_type<T>>()));
template<class T>
using has_push_back = mp11::mp_valid<has_push_back_helper, T>;
template<class T>
using inserter_implementation = mp11::mp_cond<
    is_tuple_like<T>, mp11::mp_int<2>,
    has_push_back<T>, mp11::mp_int<1>,
    mp11::mp_true,    mp11::mp_int<0>>;

template<class T>
iterator_type<T>
inserter(
    T& target,
    mp11::mp_int<2>)
{
    return target.begin();
}

template<class T>
std::back_insert_iterator<T>
inserter(
    T& target,
    mp11::mp_int<1>)
{
    return std::back_inserter(target);
}

template<class T>
std::insert_iterator<T>
inserter(
    T& target,
    mp11::mp_int<0>)
{
    return std::inserter( target, target.end() );
}

using boolean_category = std::integral_constant<
    conversion_category, conversion_category::boolean>;

using integer_category = std::integral_constant<
    conversion_category, conversion_category::integer>;

using floating_point_category = std::integral_constant<
    conversion_category, conversion_category::floating_point>;

} // namespace detail


template<class T, class Ctx, class Enable>
struct use_category : unknown_category {};

namespace detail {

using value_from_conversion = mp11::mp_true;
using value_to_conversion = mp11::mp_false;

using user_category = std::integral_constant<
    conversion_category, conversion_category::user>;
using user_context_category = std::integral_constant<
    conversion_category, conversion_category::user_context>;
using user_full_context_category = std::integral_constant<
    conversion_category, conversion_category::user_full_context>;
using json_value_category = std::integral_constant<
    conversion_category, conversion_category::json_value>;
using json_object_category = std::integral_constant<
    conversion_category, conversion_category::json_object>;
using json_array_category = std::integral_constant<
    conversion_category, conversion_category::json_array>;
using json_string_category = std::integral_constant<
    conversion_category, conversion_category::json_string>;
using json_value_ref_category = std::integral_constant<
    conversion_category, conversion_category::json_value_ref>;

template< class Cat >
using is_user_conversion = mp11::mp_bool<
       Cat::value == conversion_category::user
    || Cat::value == conversion_category::user_context
    || Cat::value == conversion_category::user_full_context>;

template< class Cat >
using is_native_conversion = mp11::mp_bool<
       Cat::value == conversion_category::user
    || Cat::value == conversion_category::json_value
    || Cat::value == conversion_category::json_object
    || Cat::value == conversion_category::json_array
    || Cat::value == conversion_category::json_string
    || Cat::value == conversion_category::json_value_ref
    || Cat::value == conversion_category::boolean
    || Cat::value == conversion_category::integer
    || Cat::value == conversion_category::floating_point>;

template<class... Args>
using supports_tag_invoke = decltype(tag_invoke( std::declval<Args>()... ));

template<class T>
using has_user_conversion_from_impl = supports_tag_invoke<
    value_from_tag, value&, T&& >;
template<class T>
using has_user_conversion_to_impl = supports_tag_invoke<
    value_to_tag<T>, value const& >;
template<class T>
using has_nonthrowing_user_conversion_to_impl = supports_tag_invoke<
    try_value_to_tag<T>, value const& >;
template< class T, class Dir >
using has_user_conversion1 = mp11::mp_if<
    std::is_same<Dir, value_from_conversion>,
    mp11::mp_valid<has_user_conversion_from_impl, T>,
    mp11::mp_or<
        mp11::mp_valid<has_user_conversion_to_impl, T>,
        mp11::mp_valid<has_nonthrowing_user_conversion_to_impl, T>>>;

template< class Ctx, class T >
using has_context_conversion_from_impl = supports_tag_invoke<
    value_from_tag, value&, T&&, Ctx const& >;
template< class Ctx, class T >
using has_context_conversion_to_impl = supports_tag_invoke<
    value_to_tag<T>, value const&, Ctx const& >;
template< class Ctx, class T >
using has_nonthrowing_context_conversion_to_impl = supports_tag_invoke<
    try_value_to_tag<T>, value const&, Ctx const& >;
template< class Ctx, class T, class Dir >
using has_user_conversion2 = mp11::mp_if<
    std::is_same<Dir, value_from_conversion>,
    mp11::mp_valid<has_context_conversion_from_impl, Ctx, T>,
    mp11::mp_or<
        mp11::mp_valid<has_context_conversion_to_impl, Ctx, T>,
        mp11::mp_valid<has_nonthrowing_context_conversion_to_impl, Ctx, T>>>;

template< class Ctx, class T >
using has_full_context_conversion_from_impl = supports_tag_invoke<
    value_from_tag, value&, T&&, Ctx const&, Ctx const& >;
template< class Ctx, class T >
using has_full_context_conversion_to_impl = supports_tag_invoke<
    value_to_tag<T>, value const&, Ctx const&,  Ctx const& >;
template< class Ctx, class T >
using has_nonthrowing_full_context_conversion_to_impl = supports_tag_invoke<
    try_value_to_tag<T>, value const&, Ctx const&, Ctx const& >;
template< class Ctx, class T, class Dir >
using has_user_conversion3 = mp11::mp_if<
    std::is_same<Dir, value_from_conversion>,
    mp11::mp_valid<has_full_context_conversion_from_impl, Ctx, T>,
    mp11::mp_or<
        mp11::mp_valid<has_full_context_conversion_to_impl, Ctx, T>,
        mp11::mp_valid<
            has_nonthrowing_full_context_conversion_to_impl, Ctx, T>>>;

template< class T >
using described_non_public_members = describe::describe_members<
    T,
    describe::mod_private
        | describe::mod_protected
        | boost::describe::mod_inherited>;

#if defined(BOOST_MSVC) && BOOST_MSVC < 1920

template< class T >
struct described_member_t_impl;

template< class T, class C >
struct described_member_t_impl<T C::*>
{
    using type = T;
};

template< class T, class D >
using described_member_t = remove_cvref<
    typename described_member_t_impl<
        remove_cvref<decltype(D::pointer)> >::type>;

#else

template< class T, class D >
using described_member_t = remove_cvref<decltype(
    std::declval<T&>().* D::pointer )>;

#endif

template< class T >
using described_members = describe::describe_members<
    T, describe::mod_any_access | describe::mod_inherited>;

#ifdef BOOST_DESCRIBE_CXX14

constexpr
bool
compare_strings(char const* l, char const* r)
{
#if defined(_MSC_VER) && (_MSC_VER <= 1900) && !defined(__clang__)
    return *l == *r && ( (*l == 0) | compare_strings(l + 1, r + 1) );
#else
    do
    {
        if( *l != *r )
            return false;
        if( *l == 0 )
            return true;
        ++l;
        ++r;
    } while(true);
#endif
}

template< class L, class R >
struct equal_member_names
    : mp11::mp_bool< compare_strings(L::name, R::name) >
{};

template< class T >
using uniquely_named_members = mp11::mp_same<
    mp11::mp_unique_if< described_members<T>, equal_member_names >,
    described_members<T> >;

#else

// we only check this in C++14, but the template should exist nevertheless
template< class T >
using uniquely_named_members = std::true_type;

#endif // BOOST_DESCRIBE_CXX14

// user conversion (via tag_invoke)
template< class T, class Ctx, class Dir >
using tag_invoke_with_context_category = mp11::mp_cond<
    has_user_conversion3<Ctx, T, Dir>, user_full_context_category,
    has_user_conversion2<Ctx, T, Dir>, user_context_category>;

template< class T, class Dir >
using tag_invoke_category = mp11::mp_cond<
    has_user_conversion1<T, Dir>,      user_category>;

// native conversions (constructors and member functions of value)
template< class T >
using native_conversion_category = mp11::mp_cond<
    std::is_same<T, value_ref>, json_value_ref_category,
    std::is_same<T, value>,     json_value_category,
    std::is_same<T, array>,     json_array_category,
    std::is_same<T, object>,    json_object_category,
    std::is_same<T, string>,    json_string_category>;

template<class T>
struct deduced_category
    : mp11::mp_cond<
        std::is_same<T, bool>,     detail::boolean_category,
        std::is_integral<T>,       detail::integer_category,
        std::is_floating_point<T>, detail::floating_point_category,
        is_null_like<T>,           null_category,
        is_string_like<T>,         string_category,
        is_variant_like<T>,        variant_category,
        is_optional_like<T>,       optional_category,
        is_map_like<T>,            map_category,
        is_sequence_like<T>,       sequence_category,
        is_tuple_like<T>,          tuple_category,
        is_described_class<T>,     described_class_category,
        is_described_enum<T>,      described_enum_category,
        is_path_like<T>,           path_category,
        // failed to find a suitable implementation
        mp11::mp_true,             unknown_category>
{ };

struct no_context {};

#ifdef BOOST_JSON_HAS_REFLECTION

template<class T, class Ctx>
constexpr
conversion_category
annotated_category_impl()
{
    static constexpr auto annotations = std::define_static_array(
        std::meta::annotations_of_with_type(^^T, ^^conversion_category));
    template for (constexpr std::meta::info a_info: annotations)
    {
        return std::meta::extract<conversion_category>(a_info);
    }
    return conversion_category::unknown;
}

template <class T, class Ctx>
using annotated_category = std::integral_constant<
    conversion_category, annotated_category_impl<T, Ctx>()>;

#endif // BOOST_JSON_HAS_REFLECTION
       //
template <class T, class Ctx>
using use_category_helper = use_category<
    T, mp11::mp_if<std::is_same<Ctx, no_context>, void, Ctx>>;

template< class Dir >
struct all_custom_checks
{
    template <class T, class Ctx>
    using fn = mp11::mp_list<
        mp11::mp_defer<use_category_helper, T, Ctx>,
#ifdef BOOST_JSON_HAS_REFLECTION
        mp11::mp_defer<annotated_category, T, Ctx>,
#endif // BOOST_JSON_HAS_REFLECTION
        mp11::mp_defer<tag_invoke_with_context_category, T, Ctx, Dir>>;
};

template< class Dir >
struct all_fallback_checks
{
    template <class T>
    using fn = mp11::mp_list<
        mp11::mp_defer<use_category_helper, T, void>,
#ifdef BOOST_JSON_HAS_REFLECTION
        mp11::mp_defer<annotated_category, T, void>,
#endif // BOOST_JSON_HAS_REFLECTION
        mp11::mp_defer<tag_invoke_category, T, Dir>,
        mp11::mp_defer<native_conversion_category, T>,
        mp11::mp_defer<deduced_category, T>>;
};

template <class T, class Ctx>
using direct_custom_checks = mp11::mp_list<
    mp11::mp_defer<use_category_helper, T, Ctx>
#ifdef BOOST_JSON_HAS_REFLECTION
        ,
        mp11::mp_defer<annotated_category, T, Ctx>
#endif // BOOST_JSON_HAS_REFLECTION
       >;

template <class T>
using direct_fallback_checks = mp11::mp_list<
    mp11::mp_defer<use_category_helper, T, void>,
#ifdef BOOST_JSON_HAS_REFLECTION
        mp11::mp_defer<annotated_category, T, void>,
#endif // BOOST_JSON_HAS_REFLECTION
    mp11::mp_defer<deduced_category, T>>;

template <class>
using no_checks = mp11::mp_list<>;

template< class T >
using nested_type = typename T::type;

template< class T1, class T2 >
using get_conversion_category_helper = mp11::mp_eval_if_c<
    conversion_category::unknown != T1::value,
    T1,
    mp11::mp_eval_or_q, T1, mp11::mp_quote<nested_type>, T2>;

template<
    class T,
    class Ctx,
    template<class, class> class CustomChecks,
    template<class> class FallbackChecks>
struct get_conversion_category_impl
{
    using type = std::integral_constant<
        conversion_category,
        mp11::mp_fold<
            mp11::mp_append< CustomChecks<T, Ctx>, FallbackChecks<T> >,
            unknown_category,
            get_conversion_category_helper>::value>;
};

template<
    class T,
    class Ctx,
    template<class, class> class CustomChecks,
    template<class> class FallbackChecks>
using get_conversion_category = typename get_conversion_category_impl<
    T, Ctx, CustomChecks, FallbackChecks>::type;

template< class T >
using any_conversion_tag = mp11::mp_not< std::is_same<T, unknown_category> >;

template<
    class T,
    template<class, class> class CustomChecks,
    template<class> class FallbackChecks,
    class... Ctxs >
struct get_conversion_category_impl<
    T, std::tuple<Ctxs...>, CustomChecks, FallbackChecks>
{
    using ctxs = mp11::mp_list< remove_cvref<Ctxs>... >;
    using cats = mp11::mp_list<
        get_conversion_category<
            T, remove_cvref<Ctxs>, CustomChecks, no_checks>... >;

    using custom_index = mp11::mp_find_if< cats, any_conversion_tag >;
    using is_custom = mp11::mp_less< custom_index, mp11::mp_size<cats> >;

    using index = mp11::mp_if< is_custom, custom_index, mp11::mp_size_t<0> >;

    using fallback_cat = mp11::mp_fold<
        FallbackChecks<T>, unknown_category, get_conversion_category_helper>;

    using type = std::integral_constant<
        conversion_category,
        mp11::mp_eval_if_not<
            is_custom, fallback_cat, mp11::mp_at, cats, index>::value>;
};

template <class T, class Dir>
using can_convert = mp11::mp_not<
    std::is_same<
        get_conversion_category<
            T,
            no_context,
            all_custom_checks<Dir>::template fn,
            all_fallback_checks<Dir>::template fn>,
        unknown_category>>;

template< class T1, class T2 >
struct copy_cref_helper
{
    using type = remove_cvref<T2>;
};
template< class T1, class T2 >
using copy_cref = typename copy_cref_helper< T1, T2 >::type;

template< class T1, class T2 >
struct copy_cref_helper<T1 const, T2>
{
    using type = remove_cvref<T2> const;
};
template< class T1, class T2 >
struct copy_cref_helper<T1&, T2>
{
    using type = copy_cref<T1, T2>&;
};
template< class T1, class T2 >
struct copy_cref_helper<T1&&, T2>
{
    using type = copy_cref<T1, T2>&&;
};

template< class Rng, class Traits >
using forwarded_value_helper = mp11::mp_if<
    std::is_convertible<
        typename Traits::reference,
        copy_cref<Rng, typename Traits::value_type> >,
    copy_cref<Rng, typename Traits::value_type>,
    typename Traits::value_type >;

template< class Rng >
using forwarded_value = forwarded_value_helper<
    Rng, iterator_traits< Rng > >;

template< class Ctx, class T, class Dir >
struct supported_context
{
    using type = Ctx;

    static
    type const&
    get( Ctx const& ctx ) noexcept
    {
        return ctx;
    }
};

template< class T, class Dir, class... Ctxs >
struct supported_context< std::tuple<Ctxs...>, T, Dir >
{
    using Ctx = std::tuple<Ctxs...>;
    using impl = get_conversion_category_impl<
        T,
        Ctx,
        all_custom_checks<Dir>::template fn,
        all_fallback_checks<Dir>::template fn>;
    using index = typename impl::index;
    using next_supported = supported_context<
        mp11::mp_at< typename impl::ctxs, index >, T, Dir >;
    using type = typename next_supported::type;

    static
    type const&
    get( Ctx const& ctx ) noexcept
    {
        return next_supported::get( std::get<index::value>( ctx ) );
    }
};

template< class T >
using value_result_type = typename std::decay<
    decltype( std::declval<T&>().value() )>::type;

template< class T >
using can_reset = decltype( std::declval<T&>().reset() );

template< class T >
using has_valueless_by_exception =
    decltype( std::declval<T const&>().valueless_by_exception() );

} // namespace detail

template <class T>
struct result_for<T, value>
{
    using type = system::result< detail::remove_cvref<T> >;
};

template<class T>
struct is_string_like
    : std::is_convertible<T, string_view>
{ };

template<class T>
struct is_path_like
    : mp11::mp_all<
        mp11::mp_valid_and_true<detail::is_its_own_value, T>,
        mp11::mp_valid_and_true<detail::has_string_type, T>>
{ };
template<class T>
struct is_sequence_like
    : mp11::mp_all<
        mp11::mp_valid_and_true<detail::are_begin_and_end_same, T>,
        mp11::mp_valid_and_true<detail::not_its_own_value, T>,
        mp11::mp_valid<detail::begin_iterator_category, T>>
{ };

template<class T>
struct is_map_like
    : mp11::mp_all<
        is_sequence_like<T>,
        mp11::mp_valid_and_true<detail::is_value_type_pair, T>,
        is_string_like<detail::key_type<T>>,
        mp11::mp_valid_and_true<detail::has_unique_keys, T>>
{ };

template<class T>
struct is_tuple_like
    : mp11::mp_valid_and_true<detail::has_positive_tuple_size, T>
{ };

template<>
struct is_null_like<std::nullptr_t>
    : std::true_type
{ };

#ifndef BOOST_NO_CXX17_HDR_VARIANT
template<>
struct is_null_like<std::monostate>
    : std::true_type
{ };
#endif // BOOST_NO_CXX17_HDR_VARIANT

template<class T>
struct is_described_class
    : mp11::mp_and<
        describe::has_describe_members<T>,
        mp11::mp_not< std::is_union<T> >,
        mp11::mp_empty<
            mp11::mp_eval_or<
                mp11::mp_list<>, detail::described_non_public_members, T>>>
{ };

template<class T>
struct is_described_enum
    : describe::has_describe_enumerators<T>
{ };

template<class T>
struct is_variant_like : mp11::mp_valid<detail::has_valueless_by_exception, T>
{ };

template<class T>
struct is_optional_like
    : mp11::mp_and<
        mp11::mp_not<std::is_void<
            mp11::mp_eval_or<void, detail::value_result_type, T>>>,
        mp11::mp_valid<detail::can_reset, T>>
{ };

} // namespace json
} // namespace boost

#endif // BOOST_JSON_IMPL_CONVERSION_HPP
