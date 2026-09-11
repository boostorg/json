//
// Copyright (c) 2022 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_CONVERSION_HPP
#define BOOST_JSON_CONVERSION_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/fwd.hpp>
#include <boost/mp11/utility.hpp>

#include <type_traits>

namespace boost {
namespace json {

namespace detail {

template< class Ctx, class T, class Dir >
struct supported_context;

} // namespace detail

/** Conversion categories supported by the library.

    With the exception of `unknown` every enumerator of this enum represents a
    conversion category supported by this library. These categories are used
    to pick appropriate implementations of @ref value_to, @ref try_value_to,
    @ref value_from, @ref parse_into, and @ref serialize.

    `unknown` category is assigned to types for which the library could not
    deduce an appropriate category. Using the aforementioned functions with
    objects of such types will result in compilation failure.
 */
enum class conversion_category
{
    /// See @ref unknown_category
    unknown,
    /// See @ref null_category
    null,
    /// See @ref string_category
    string,
    /// See @ref variant_category
    variant,
    /// See @ref optional_category
    optional,
    /// See @ref map_category
    map,
    /// See @ref sequence_category
    sequence,
    /// See @ref tuple_category
    tuple,
    /// See @ref described_class_category
    described_class,
    /// See @ref described_enum_category
    described_enum,
    /// See @ref path_category
    path,

    // internal categories
#ifndef BOOST_JSON_DOCS
    boolean,
    integer,
    floating_point,
    json_value,
    json_object,
    json_array,
    json_string,
    json_value_ref,

    user,
    user_context,
    user_full_context,
#endif
};

/** Customization point tag.

    This tag type is used by the function @ref value_from to select overloads
    of `tag_invoke`.

    @note This type is empty; it has no members.

    @see
        @ref value_from, @ref value_to, @ref value_to_tag,
        [tag_invoke: A general pattern for supporting customisable functions](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf).
*/
struct value_from_tag { };

/** Customization point tag type.

    This tag type is used by the function @ref value_to to select overloads of
    `tag_invoke`.

    @note This type is empty; it has no members.

    @see
        @ref value_from, @ref value_from_tag, @ref value_to,
        [tag_invoke: A general pattern for supporting customisable functions](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf).
*/
template<class T>
struct value_to_tag { };

/** Customization point tag type.

    This tag type is used by the function @ref try_value_to to select overloads
    of `tag_invoke`.

    @note This type is empty; it has no members.

    @see
        @ref value_to, @ref value_to_tag,
        [tag_invoke: A general pattern for supporting customisable functions](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1895r0.pdf).
*/
template<class T>
struct try_value_to_tag { };

/** Determine if `T` can be treated like a string during conversions.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to using @ref use_category and @ref string_category.

    Provides the member constant `value` that is equal to `true`, if `T` is
    convertible to @ref string_view. Otherwise, `value` is equal to `false`.

    Users can specialize the trait for their own types if they don't want them
    to be treated like strings. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_string_like<your::string> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @par Types satisfying the trait
    @ref string, @ref string_view, @ref std::string, @ref std::string_view.

    @see @ref value_from, @ref value_to.
*/
template<class T>
struct is_string_like;

/** Conversion category constant for strings.

    String types are represented as JSON strings.

    By default a type is considered a string if it is convertible to @ref
    string_view.

    Users can specialize the trait for their own types if they want them to be
    treated like strings. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct is_string_like<your::string> : string_category
    { };

    } }
    @endcode

    @par Matching Types
    @ref string_view, @ref std::string, @ref std::string_view.
*/
using string_category = std::integral_constant<
    conversion_category, conversion_category::string>;

/** Determine if `T` can be treated like `std::filesystem::path` during conversions.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to @ref use_category and @ref path_category.

    Given `t`, a glvalue of type `T`, if

    - given `It`, the type denoted by `decltype(std::begin(t))`,
      `std::iterator_traits<It>::iterator_category` is well-formed and denotes
      a type; and

    - `std::iterator_traits<It>::value_type` is `T`; and

    - `T::value_type` is well-formed and denotes a type; and

    - `T::string_type` is well-formed, denotes a type, and is an alias for
    `std::basic_string< T::value_type >`;

    then the trait provides the member constant `value` that is equal to
    `true`. Otherwise, `value` is equal to `false`.

    Users can specialize the trait for their own types if they don't want them
    to be treated like filesystem paths. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_path_like<your::path> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @par Types Satisfying the Trait
    @ref std::filesystem::path, @ref boost::filesystem::path.

    @see @ref value_from, @ref value_to.
*/
template<class T>
struct is_path_like;

/** Conversion category constant for filesystem paths.

    Paths are represented in JSON as strings.

    By default a type `T` is considered a path if

    - given `t`, a glvalue of type `T`; and

    - given `It`, the type denoted by `decltype(std::begin(t))`,
      `std::iterator_traits<It>::iterator_category` is well-formed and denotes
      a type; and

    - `std::iterator_traits<It>::value_type` is `T`; and

    - `T::value_type` is well-formed and denotes a type; and

    - `T::string_type` is well-formed, denotes a type, and is an alias for
    `std::basic_string< T::value_type >`.

    Users can specialize @ref use_category for their own types if they want
    them to be treated like filesystem paths. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::path> : path_category
    { };

    } }
    @endcode

    @par Matching Types
    @ref std::filesystem::path, @ref boost::filesystem::path.
*/
using path_category = std::integral_constant<
    conversion_category, conversion_category::path>;

/** Determine if `T` can be treated like a sequence during conversions.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to using @ref use_category and @ref sequence_category.

    Given `t`, a glvalue of type `T`, if

    - given `It`, the type denoted by `decltype(std::begin(t))`,
      `std::iterator_traits<It>::iterator_category` is well-formed and denotes
      a type; and

    - `decltype(std::end(t))` also denotes the type `It`; and

    - `std::iterator_traits<It>::value_type` is not `T`.

    then the trait provides the member constant `value` that is equal to
    `true`. Otherwise, `value` is equal to `false`.

    Users can specialize the trait for their own types if they don't want them
    to be treated like sequences. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_sequence_like<your::container> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode


    @par Types satisfying the trait
    Any {req_SequenceContainer}, array types.

    @see @ref value_from, @ref value_to
*/
template<class T>
struct is_sequence_like;

/** Conversion category constant for sequences.

    Sequences are represented in JSON as arrays.

    By default a type `T` is considered a sequence if

    - given `t`, a glvalue of type `T`; and

    - given `It`, the type denoted by `decltype(std::begin(t))`,
      `std::iterator_traits<It>::iterator_category` is well-formed and denotes
      a type; and

    - `decltype(std::end(t))` also denotes the type `It`; and

    - `std::iterator_traits<It>::value_type` is not `T`; and

    - `std::iterator_traits<It>::value_type` is not `T`.

    Users can specialize @ref use_category for their own types if they want
    them to be treated like sequences. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::container> : sequence_category
    { };

    } }
    @endcode

    @par Matching Types
    Any {req_SequenceContainer}, array types.
*/
using sequence_category = std::integral_constant<
    conversion_category, conversion_category::sequence>;

/** Determine if `T` can be treated like a 1-to-1 mapping during
    conversions.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to using @ref use_category and @ref map_category.

    Given `t`, a glvalue of type `T`, if

    - `is_sequence_like<T>::value` is `true`; and

    - given type `It` denoting `decltype(std::begin(t))`, and types `K` and
      `M`,  `std::iterator_traits<It>::value_type` denotes `std::pair<K, M>`;
      and

    - `std::is_string_like<K>::value` is `true`; and

    - given `v`, a glvalue of type `V`, and `E`, the type denoted by
      `decltype(t.emplace(v))`, `std::is_tuple_like<E>::value` is `true`;

    then the trait provides the member constant `value` that is equal to
    `true`. Otherwise, `value` is equal to `false`.

    Users can specialize the trait for their own types if they don't want them
    to be treated like mappings. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_map_like<your::map> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @note The restriction for `t.emplace()` return type ensures that the
    container does not accept duplicate keys.

    @par Types Satisfying The Trait
    @ref std::map, @ref std::unordered_map.

    @see @ref value_from, @ref value_to.
*/
template<class T>
struct is_map_like;

/** Conversion category constant for maps.

    Maps are represented in JSON as objects. Such representation limits this
    category to 1-to-1 maps (as opposed to 1-to-many e.g. @ref std::multimap)
    with string keys.

    By default a type `T` is considered a map if

    - given `t`, a glvalue of type `T`; and

    - given `It`, the type denoted by `decltype(std::begin(t))`,
      `std::iterator_traits<It>::iterator_category` is well-formed and denotes
      a type; and

    - `decltype(std::end(t))` also denotes the type `It`; and

    - `std::iterator_traits<It>::value_type` is not `T`; and

    - given types `K` and `M`, `std::iterator_traits<It>::value_type` denotes
      `std::pair<K, M>`; and

    - conversion category of K is @ref conversion_category::string; and

    - given `v`, a value of type `std::iterator_traits<It>::value_type`,
      `std::tuple_size< decltype(t.emplace(v)) >::value` is a positive number.

    Less formally, `T` should be a sequence of `std::pair`s with unique
    string-like keys.

    @note The restriction for `t.emplace()` return type ensures that the
    container does not allow duplicate keys.

    Users can specialize @ref use_category for their own types if they want
    them to be treated like maps. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct is_map_like<your::map> : map_category
    { };

    } }
    @endcode

    @par Matching Types
    @ref std::map, @ref std::unordered_map.
*/
using map_category = std::integral_constant<
    conversion_category, conversion_category::map>;

/** Determine if `T` can be treated like a tuple during conversions.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to using @ref use_category and @ref tuple_category.

    Provides the member constant `value` that is equal to `true`, if
    `std::tuple_size<T>::value` is a positive number. Otherwise, `value`
    is equal to `false`.

    Users can specialize the trait for their own types if they don't want them
    to be treated like tuples. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_tuple_like<your::tuple> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode


    @par Types satisfying the trait
    @ref std::tuple, @ref std::pair.

    @see @ref value_from, @ref value_to.
*/
template<class T>
struct is_tuple_like;

/** Conversion category constant for tuples.

    Tuples are represented in JSON as arrays.

    By default a type `T` is considered a tuple if `std::tuple_size<T>::value`
    is a positive number.

    Users can specialize @ref use_category for their own types if they want
    them to be treated like tuples. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::tuple> : tuple_category
    { };

    } }
    @endcode


    @par Matching Types
    @ref std::tuple, @ref std::pair.
*/
using tuple_category = std::integral_constant<
    conversion_category, conversion_category::tuple>;

/** Determine if `T` can be treated like null during conversions.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to using @ref use_category and @ref null_category.

    Primary template instantiations provide the member constant `value` that is
    equal to `false`. Users can specialize the trait for their own types if
    they **do** want them to be treated as nulls. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_null_like<your::null_type> : std::true_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @par Types Satisfying the Trait
    @ref std::nullptr_t.

    @see @ref value_from, @ref value_to
*/
template<class T>
struct is_null_like
    : std::false_type
{ };

/** Conversion category constant for null types.

    Null types are represented in JSON as the `null` literal.

    By default only the types @ref std::nullptr_t and @ref std::monostate are
    considered null types.

    Users can specialize @ref use_category for their own types if they want
    them to be treated as nulls. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::null> : null_category
    { };

    } }
    @endcode

    @par Matching Types
    @ref std::nullptr_t, @ref std::monostate.
*/
using null_category = std::integral_constant<
    conversion_category, conversion_category::null>;

/** Determine if `T` should be treated as a described class.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to @ref use_category and @ref described_class_category.

    Described classes are serialized as objects with an element for each
    described data member. Described bases are serialized in a flattened way,
    that is members of bases are serialized as direct elements of the object,
    and no nested objects are created for bases.

    A described class should not have non-public described members (including
    inherited members) or non-public non-empty described bases. Or more
    formally, given `L`, a class template of the form
    `template<class...> struct L {}`, if

    - `boost::describe::has_describe_members<T>::value` is `true`; and

    - `boost::describe::describe_members<T, boost::describe::mod_private |
      boost::describe::mod_protected | boost::describe::mod_inherited>` denotes
      `L<>`; and

    - `std::is_union<T>::value` is `false`;

    then the trait provides the member constant `value` that is equal to
    `true`. Otherwise, `value` is equal to `false`.

    @note Shadowed members are ignored both for requirements checking and for
    performing conversions.

    Users can specialize the trait for their own types if they don't want them
    to be treated as described classes. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_described_class<your::described_class> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    Users can also specialize the trait for their own types _with_ described
    non-public data members to enable this conversion implementation. Note that
    non-public bases are not supported regardless.

    @see [Boost.Describe](https://www.boost.org/doc/libs/latest/libs/describe/doc/html/describe.html).
*/
template<class T>
struct is_described_class;

/** Conversion category constant for described classes.

    Described classes are represented in JSON as objects with an element for
    each described data member. Described bases are flattened, that is members
    of bases are represented as direct elements of the object rather than
    elements of some subobjects.

    By default a described class should not have non-public described members
    (including inherited members) or non-public non-empty described bases. Or
    more formally a type `T` is considered a described class if

    - `boost::describe::has_describe_members<T>::value` is `true`; and

    - given `L`, a class template of the form `template<class...> struct L {}`,
     `boost::describe::describe_members<T, boost::describe::mod_private |
      boost::describe::mod_protected | boost::describe::mod_inherited>` denotes
      `L<>`; and

    - `std::is_union<T>::value` is `false`.

    @note Shadowed members are ignored both for requirements checking and for
    performing conversions.

    Users can specialize @ref use_category for their own types if they want
    them to be treated as described classes. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::described_class>
        : described_class_category
    { };

    } }
    @endcode

    Users can also specialize the trait for their own types _with_ described
    non-public data members to enable this conversion implementation. Note that
    non-public bases are not supported regardless.

    @see [Boost.Describe](https://www.boost.org/doc/libs/latest/libs/describe/doc/html/describe.html).
*/
using described_class_category = std::integral_constant<
    conversion_category, conversion_category::described_class>;

/** Determine if `T` should be treated as a described enum.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to @ref use_category and @ref described_enum_category.

    Described enums are serialized as strings when their value equals to a
    described enumerator, and as integers otherwise. The reverse operation
    does not convert numbers to enums values, though, and instead produces
    an error.

    If `boost::describe::has_describe_enumerators<T>::value` is `true`, then
    the trait provides the member constant `value` that is equal to `true`.
    Otherwise, `value` is equal to `false`.

    Users can specialize the trait for their own enums if they don't want them
    to be treated as described enums. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_described_enum<your::described_enum> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @see [Boost.Describe](https://www.boost.org/doc/libs/latest/libs/describe/doc/html/describe.html).
*/
template<class T>
struct is_described_enum;

/** Conversion category constant for described enums.

    Described enums are serialized as strings when their value equals to a
    described enumerator, and as integers otherwise. The reverse operation
    does not convert numbers to enums values, though, and instead produces
    an error.

    By default a type `T` is considered a described enum if
    `boost::describe::has_describe_enumerators<T>::value` is `true`.

    Users can specialize @ref use_category for their own enums if they want
    them to be treated as described enums. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::described_enum>
        : described_enum_category
    { };

    } }
    @endcode

    @see [Boost.Describe](https://www.boost.org/doc/libs/latest/libs/describe/doc/html/describe.html).
*/
using described_enum_category = std::integral_constant<
    conversion_category, conversion_category::described_enum>;

/** Determine if `T` should be treated as a variant.

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to using @ref use_category and @ref variant_category.

    Variants are serialized the same way their active alternative is
    serialized. The opposite conversion selects the first alternative for which
    conversion succeeds.

    Given `t`, a glvalue of type ` const T`, if `t.valueless_by_exception()` is
    well-formed, then the trait provides the member constant `value` that is
    equal to `true`. Otherwise, `value` is equal to `false`.

    Users can specialize the trait for their own types if they don't want them
    to be treated as variants. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_variant_like<your::variant> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @par Types Satisfying the Trait
    @ref std::variant, @ref boost::variant2::variant.
*/
template<class T>
struct is_variant_like;

/** Conversion category constant for variants.

    Variants are serialized the same way their active alternative is
    serialized. The opposite conversion selects the first alternative for which
    conversion succeeds.

    By default a type `T` is considered a variant if given `t`, a glvalue of
    type `const T`, `t.valueless_by_exception()` is well-formed.

    Users can specialize @ref use_category for their own types if they want
    them to be treated as variants. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::variant> : variant_category
    { };

    } }
    @endcode

    @par Mathcing Types
    @ref std::variant, @ref boost::variant2::variant.
*/
using variant_category = std::integral_constant<
    conversion_category, conversion_category::variant>;

/** Determine if `T` should be treated as an optional

    @warning This trait is deprecated and **will be removed in Boost 1.93.0.**
    Switch to using @ref use_category and @ref optional_category.

    Optionals are serialized as `null` if empty, or as the stored type
    otherwise.

    Given `t`, a glvalue of type `T`, if

    - `decltype( t.value() )` is well-formed and isn't a void type; and
    - `t.reset()` is well-formed;

    then the trait provides the member constant `value` that is equal to
    `true`. Otherwise, `value` is equal to `false`.

    Users can specialize the trait for their own types if they don't want them
    to be treated as optionals. For example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct is_optional_like<your::optional> : std::false_type
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @par Types Satisfying the Trait
    @ref std::optional, @ref boost::optional.
*/
template<class T>
struct is_optional_like;

/** Conversion category constant for optionals.

    Optionals are represented in JSON as `null` if not engaged (i.e. does not
    store a value), or as the stored type otherwise.

    By default a type `T` is considered an optional if

    - given `t`, a glvalue of type `T`, `decltype( t.value() )` is well-formed
      and isn't a void type; and
    - `t.reset()` is well-formed;

    Users can specialize @ref use_category for their own types if they want
    them to be treated as optionals. For example:

    @code
    namespace boost { namespace json {

    template <>
    struct use_category<your::optional> : optional_category
    { };

    } }
    @endcode

    @par Matching Types
    @ref std::optional, @ref boost::optional.
*/
using optional_category = std::integral_constant<
    conversion_category, conversion_category::optional>;

/** Unknown conversion category.

    This category is assigned by the library's conversion category deduction
    mechanism when it fails.
*/
using unknown_category = std::integral_constant<
    conversion_category, conversion_category::unknown>;

/** Overrides the category of conversion of a type.

    Specializations should provide the static member constant `value` that is
    equal to an enumerator of @ref conversion_category that represents the
    suitable conversion category of T in context `Ctx`.

    The primary template's `value` is equal to @ref
    conversion_category::unknown.

    Use `void` for `Ctx` (the default) if the category should apply regardless
    of context.

    The third template parameter can be used for conditional specializations.

    Usage example:

    @code
    namespace boost {
    namespace json {

    template <>
    struct conversion_category<your::string> : string_category
    { };

    } // namespace boost
    } // namespace json
    @endcode

    @see \<\<Custom Conversions>>, @ref value_from, @ref value_to.
*/
template<class T, class Ctx = void, class Enable = void>
struct use_category;

} // namespace json
} // namespace boost

#include <boost/json/impl/conversion.hpp>

#endif // BOOST_JSON_CONVERSION_HPP
