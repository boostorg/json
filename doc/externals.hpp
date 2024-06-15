#pragma once


namespace boost {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/release/libs/assert/doc/html/assert.html#source_location
using source_location = void;

namespace container {
namespace pmr {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/release/doc/html/boost/container/pmr/polymorphic_allocator.html
template <class T>
using polymorphic_allocator = void;

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/1_85_0/doc/html/boost/container/pmr/memory_resource.html
using memory_resource = void;

} // namespace pmr
} // namespace container

namespace system {

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/release/libs/system/doc/html/system.html#ref_error_code
using error_code = void;

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/release/libs/system/doc/html/system.html#ref_error_category
using error_category = void;

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/release/libs/system/doc/html/system.html#ref_error_condition
using error_condition = void;

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/release/libs/system/doc/html/system.html#ref_system_error
using system_error = void;

/// !EXTERNAL!
///
/// @see https://www.boost.org/doc/libs/release/libs/system/doc/html/system.html#ref_boostsystemresult_hpp
using result = void;

} // namespace system
} // namespace boost

namespace std {

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/iterator/reverse_iterator
template <class T>
using reverse_iterator = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/size_t
using size_t = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/integer
using uint64_t = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/integer
using int64_t = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/nullptr_t
using nullptr_t = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/ptrdiff_t
using ptrdiff_t = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/utility/initializer_list
template <class T>
using initializer_list= void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/error/error_code
using error_code = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/utility/pair
using pair = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/types/byte
using byte = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/io/basic_ostream
using ostream = void;

/// !EXTERNAL!
///
/// @see https://en.cppreference.com/w/cpp/io/basic_istream
using istream = void;

} // namespace std
