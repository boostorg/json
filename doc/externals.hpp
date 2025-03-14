#pragma once

namespace boost {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/optional/doc/html/boost_optional/reference/header__boost_optional_optional_hpp_/header_optional_optional_values.html#reference_operator_template
struct optional {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/assert/doc/html/assert.html#source_location
struct source_location {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/container_hash/doc/html/hash.html#ref_hasht
template<class T>
void
hash(T const&);

namespace container {
namespace pmr {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/doc/html/doxygen/boost_container_header_reference/classboost_1_1container_1_1pmr_1_1polymorphic__allocator.html
template <class T>
struct polymorphic_allocator {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/doc/html/doxygen/boost_container_header_reference/classboost_1_1container_1_1pmr_1_1memory__resource.html
struct memory_resource {};

} // namespace pmr
} // namespace container

namespace filesystem {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/filesystem/doc/reference.html#class-path
struct path {};

} // namespace filesystem

namespace system {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/system/doc/html/system.html#ref_error_code
struct error_code {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/system/doc/html/system.html#ref_error_category
struct error_category {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/system/doc/html/system.html#ref_error_condition
struct error_condition {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/system/doc/html/system.html#ref_system_error
struct system_error {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/system/doc/html/system.html#ref_boostsystemresult_hpp
struct result {};

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/system/doc/html/system.html#ref_generic_category
void generic_category();

} // namespace system

namespace variant2 {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/latest/libs/variant2/doc/html/variant2.html#ref_variant
struct variant {};

} // namespace variant2
} // namespace boost

namespace std {

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/iterator/reverse_iterator
template <class T>
struct reverse_iterator {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/size_t
struct size_t {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/integer
struct uint64_t {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/integer
struct int64_t {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/nullptr_t
struct nullptr_t {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/ptrdiff_t
struct ptrdiff_t {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/utility/initializer_list
template <class T>
struct initializer_list {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/error/error_code
struct error_code {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/utility/pair
struct pair {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/byte
struct byte {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/io/basic_ostream
struct ostream {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/io/basic_istream
struct istream {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/io/basic_ios
struct ios
{
    /// !EXTERNAL!
    ///
    /// @see https://en.cppreference.com/w/cpp/io/basic_ios/exceptions
    void exceptions();
};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/io/ios_base
struct ios_base
{
    /// !EXTERNAL!
    ///
    /// @see https://en.cppreference.com/w/cpp/io/ios_base/iostate.html
    enum iostate
    {
        /// !EXTERNAL!
        ///
        /// @see https://en.cppreference.com/w/cpp/io/ios_base/iostate.html
        failbit,
    };

    /// !EXTERNAL!
    ///
    /// @see https://en.cppreference.com/w/cpp/io/ios_base/fmtflags.html
    enum fmtflags
    {
        /// !EXTERNAL!
        ///
        /// @see https://en.cppreference.com/w/cpp/io/ios_base/fmtflags.html
        skipws,
    };
};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/string/basic_string
struct string {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/string/basic_string_view
struct string_view {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/memory/new/bad_alloc
struct bad_alloc {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/container/map
struct map {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/container/unordered_map
struct unordered_map {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/container/vector
struct vector {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/utility/tuple
struct tuple {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/utility/variant
struct variant {};

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/utility/optional
struct optional {};

namespace filesystem {

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/filesystem/path
struct path {};

} // namespace filesystem
} // namespace std
