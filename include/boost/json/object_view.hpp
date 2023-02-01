//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_OBJECT_VIEW_HPP
#define BOOST_JSON_OBJECT_VIEW_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/kind.hpp>
#include <boost/optional.hpp>
#include <boost/describe/class.hpp>
#include <boost/json/storage_ptr.hpp>
#include <boost/json/string_view.hpp>
#include <boost/json/detail/object.hpp>
#include <boost/json/detail/value.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <type_traits>
#include <utility>

namespace boost {
namespace json {

class key_value_view_pair;

class value_view;

/** A dynamically sized associative container of JSON key/value pairs.

    This is an associative container whose elements
    are key/value pairs with unique keys.
\n
    The elements are stored contiguously; iterators are
    ordinary pointers, allowing random access pointer
    arithmetic for retrieving elements.
    In addition, the container maintains an internal
    index to speed up find operations, reducing the
    average complexity for most lookups and insertions.
\n
    Reallocations are usually costly operations in terms of
    performance, as elements are copied and the internal
    index must be rebuilt. The @ref reserve function can
    be used to eliminate reallocations if the number of
    elements is known beforehand.

    @par Allocators

    All elements stored in the container, and their
    children if any, will use the same memory resource that
    was used to construct the container.

    @par Thread Safety

    Non-const member functions may not be called
    concurrently with any other member functions.

    @par Satisfies
        <a href="https://en.cppreference.com/w/cpp/named_req/ContiguousContainer"><em>ContiguousContainer</em></a>,
        <a href="https://en.cppreference.com/w/cpp/named_req/ReversibleContainer"><em>ReversibleContainer</em></a>, and
        <a href="https://en.cppreference.com/w/cpp/named_req/SequenceContainer"><em>SequenceContainer</em></a>.
*/
class object_view
{
    struct adaptor
    {
        virtual bool            empty(const void * data)   const = 0;
        virtual std::size_t      size(const void * data)   const = 0;
        virtual std::size_t  max_size(const void * data)   const = 0;
        virtual std::size_t  capacity(const void * data)   const = 0;
        virtual value_view         at(const void * data, string_view key) const = 0;
        virtual bool         contains(const void*, string_view key) const = 0;
        virtual std::size_t     count(const void*, string_view key) const = 0;

        virtual boost::optional<value_view> if_contains(const void * data, string_view key) const = 0;

        virtual const void * find (const void *data, string_view key) const = 0;

        virtual const void * begin(const void *data) const = 0;
        virtual const void * end  (const void *data) const = 0;
        virtual const void * next (const void* elem, std::ptrdiff_t change) const = 0;
        virtual std::intptr_t distance(const void* itr1, const void * itr2) const = 0;
        virtual key_value_view_pair dereference(const void * data, const void * elem) const = 0;
    };

    kind k_ = kind::object;     // must come first
    const void * data_;
    const adaptor * adaptor_;

    struct empty_adaptor;
    struct object_adaptor;
    static const empty_adaptor empty_impl_;
    static const object_adaptor object_impl_;

    template<typename Map>
    struct map_adaptor;

    template<typename Map>
    struct describe_adaptor;

#if defined(BOOST_DESCRIBE_CXX14)
    template<typename Class, typename Member>
    constexpr static auto member_convertible_to_value_view_impl(Member (Class::*)) -> const std::decay_t<Member> &;
    template<typename Member>
    using member_convertible_to_value_view = std::is_convertible<decltype(member_convertible_to_value_view_impl(Member::pointer)), value_view>;
#endif
public:
    /** The type of _Allocator_ returned by @ref get_allocator

        This type is a @ref polymorphic_allocator.
    */
#ifdef BOOST_JSON_DOCS
    using allocator_type = __see_below__;
#else
    using allocator_type = polymorphic_allocator<value>;
#endif

    /** The type of keys.

        The function @ref string::max_size returns the
        maximum allowed size of strings used as keys.
    */
    using key_type = string_view;

    /// The type of mapped values
    using mapped_type = value_view;

    /// The element type
    using value_type = key_value_view_pair;

    /// The type used to represent unsigned integers
    using size_type = std::size_t;

    /// The type used to represent signed integers
    using difference_type = std::ptrdiff_t;

    /// A reference to an element
    using reference = value_type&;

    /// A const reference to an element
    using const_reference = value_type const&;

    /// A pointer to an element
    using pointer = value_type*;

    /// A const pointer to an element
    using const_pointer = value_type const*;

    /// A const random access iterator to an element
    struct const_iterator
    {
        typedef void pointer;
        typedef void reference;
        typedef std::random_access_iterator_tag iterator_category;
        typedef key_value_view_pair value_type;
        typedef std::ptrdiff_t difference_type;

        const_iterator & operator+=(std::size_t pos)
        {
            elem = adaptor_->next(elem, static_cast<std::intptr_t>(pos));
            return *this;
        }
        const_iterator & operator-=(std::size_t pos)
        {
            elem = adaptor_->next(elem, -static_cast<std::intptr_t>(pos));
            return *this;
        }
        const_iterator operator+(std::size_t pos) const
        {
            return {data, adaptor_->next(elem, static_cast<std::intptr_t>(pos)), adaptor_};
        }
        const_iterator operator-(std::size_t pos) const
        {
            return {data, adaptor_->next(elem, -static_cast<std::intptr_t>(pos)), adaptor_};
        }

        std::ptrdiff_t operator+(const const_iterator & itr) const
        {
            return adaptor_->distance(elem, itr.elem);
        }
        std::ptrdiff_t operator-(const const_iterator & itr) const
        {
            return adaptor_->distance(itr.elem, elem);
        }

        friend bool operator==(const const_iterator & lhs, const const_iterator &rhs)
        {
            return (lhs.adaptor_ == rhs.adaptor_) && lhs.elem == rhs.elem;
        }
        friend bool operator!=(const const_iterator & lhs, const const_iterator &rhs)
        {
            return (lhs.adaptor_ != rhs.adaptor_) || lhs.elem != rhs.elem;
        }
        friend bool operator>=(const const_iterator & lhs, const const_iterator &rhs)
        {
            if (lhs.adaptor_ >= rhs.adaptor_)
                return true;
            else if (lhs.adaptor_ == rhs.adaptor_)
                return lhs.elem >= rhs.elem;
            else
                return false;
        }
        friend bool operator<=(const const_iterator & lhs, const const_iterator &rhs)
        {
            if (lhs.adaptor_ <= rhs.adaptor_)
                return true;
            else if (lhs.adaptor_ == rhs.adaptor_)
                return lhs.elem <= rhs.elem;
            else
                return false;
        }
        friend bool operator> (const const_iterator & lhs, const const_iterator &rhs)
        {
            if (lhs.adaptor_ > rhs.adaptor_)
                return true;
            else if (lhs.adaptor_ == rhs.adaptor_)
                return lhs.elem > rhs.elem;
            else
                return false;
        }
        friend bool operator< (const const_iterator & lhs, const const_iterator &rhs)
        {
            if (lhs.adaptor_ < rhs.adaptor_)
                return true;
            else if (lhs.adaptor_ == rhs.adaptor_)
                return lhs.elem < rhs.elem;
            else
                return false;
        }

        const_iterator & operator++()
        {
            elem = adaptor_->next(elem, 1);
            return *this;
        }
        const_iterator & operator--()
        {
            elem = adaptor_->next(elem, -1);
            return *this;
        }
        const_iterator operator++(int)
        {
            auto el = elem;
            elem = adaptor_->next(elem, 1);
            return {data, el, adaptor_};
        }
        const_iterator operator--(int)
        {
            auto el = elem;
            elem = adaptor_->next(elem, -1);
            return {data, el, adaptor_};
        }

        BOOST_JSON_DECL key_value_view_pair operator*() const;
        BOOST_JSON_DECL key_value_view_pair operator[](std::size_t pos) const;

        constexpr const_iterator() = default;
        constexpr const_iterator(const const_iterator & ) = default;
        constexpr const_iterator(const_iterator &&) = default;
    private:
        constexpr const_iterator(const void * data, const void * elem, const adaptor * adaptor_)
                : data(data), elem(elem), adaptor_(adaptor_) {}
        const void * data{nullptr}, * elem{nullptr};
        const adaptor * adaptor_{nullptr};
        friend class object_view;
    };
    /// A const reverse random access iterator to an element
    using const_reverse_iterator =
        std::reverse_iterator<const_iterator>;

    //------------------------------------------------------

    /** Destructor.

        The destructor for each element is called if needed,
        any used memory is deallocated, and shared ownership
        of the @ref memory_resource is released.

        @par Complexity
        Constant, or linear in @ref size().

        @par Exception Safety
        No-throw guarantee.
    */
    ~object_view() noexcept = default;

    //------------------------------------------------------

    /** Default constructor.

        The constructed object is empty with zero
        capacity, using the default memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    object_view() noexcept;

    /** Construct a view from an existing object

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    object_view(const object & obj) noexcept;

    /** Move constructor.

        The object is constructed by acquiring ownership of
        the contents of `other` and shared ownership
        of `other`'s memory resource.

        @note

        After construction, the moved-from object behaves
        as if newly constructed with its current memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The object to move.
    */
    object_view(object_view&& other) noexcept = default;
    /** Copy constructor.

        The object is constructed with a copy of the
        contents of `other`, using `other`'s memory resource.

        @par Complexity
        Linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to copy.
    */
    object_view(object_view const& other) = default;

#if defined(BOOST_DESCRIBE_CXX14)
    template<typename T>
    object_view(const T & en,
                typename std::enable_if<
                        is_described_class<T>::value /*&&
                        mp11::mp_all_of<describe::describe_members<T, describe::mod_public | describe::mod_inherited>,
                                        member_convertible_to_value_view>::value*/
                        >::type * = nullptr) ;
#endif

    template<typename T>
    object_view(const T & mp,
                typename std::enable_if<
                       is_map_like<T>::value
                    && (sizeof(typename T::const_iterator) == sizeof(void*))
                    && std::is_convertible<typename T::mapped_type, value_view>::value>::type * = nullptr) ;


    //------------------------------------------------------
    //
    // Assignment
    //
    //------------------------------------------------------

    /** Copy assignment.

        The contents of the object are replaced with an
        element-wise copy of `other`.

        @par Complexity
        Linear in @ref size() plus `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The object to copy.
    */
    BOOST_JSON_DECL
    object_view&
    operator=(object_view const& other) = default;

    //------------------------------------------------------
    //
    // Iterators
    //
    //------------------------------------------------------

    /** Return a const iterator to the first element.

        If the container is empty, @ref end() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    begin() const noexcept
    {
        return const_iterator{data_, adaptor_->begin(data_),adaptor_};
    }

    /** Return a const iterator to the first element.

        If the container is empty, @ref cend() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    cbegin() const noexcept
    {
        return const_iterator{data_, adaptor_->begin(data_),adaptor_};
    }

    /** Return a const iterator to the element following the last element.

        The element acts as a placeholder; attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    end() const noexcept
    {
        return const_iterator{data_, adaptor_->end(data_),adaptor_};
    }

    /** Return a const iterator to the element following the last element.

        The element acts as a placeholder; attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_iterator
    cend() const noexcept
    {
        return const_iterator{data_, adaptor_->end(data_),adaptor_};
    }

    /** Return a const reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the
        last element of the non-reversed container.
        If the container is empty, @ref rend() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    /** Return a const reverse iterator to the first element of the reversed container.

        The pointed-to element corresponds to the
        last element of the non-reversed container.
        If the container is empty, @ref crend() is returned.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }

    /** Return a const reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    /** Return a const reverse iterator to the element following the last element of the reversed container.

        The pointed-to element corresponds to the element
        preceding the first element of the non-reversed container.
        This element acts as a placeholder, attempting
        to access it results in undefined behavior.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    const_reverse_iterator
    crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }

    //------------------------------------------------------
    //
    // Capacity
    //
    //------------------------------------------------------

    /** Return whether there are no elements.

        Returns `true` if there are no elements in
        the container, i.e. @ref size() returns 0.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    bool
    empty() const noexcept
    {
      return adaptor_->empty(data_);
    }

    /** Return the number of elements.

        This returns the number of elements in the container.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    std::size_t
    size() const noexcept
    {
      return adaptor_->size(data_);
    }

    /** Return the maximum number of elements any object can hold

        The maximum is an implementation-defined number dependent
        on system or library implementation. This value is a
        theoretical limit; at runtime, the actual maximum size
        may be less due to resource limits.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    std::size_t
    max_size() const noexcept
    {
        return adaptor_->max_size(data_);
    }

    /** Return the number of elements that can be held in currently allocated memory

        This number may be larger than the value returned
        by @ref size().

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    std::size_t
    capacity() const noexcept
    {
        return adaptor_->capacity(data_);
    }

    //------------------------------------------------------
    //
    // Lookup
    //
    //------------------------------------------------------

    /** Access the specified element, with bounds checking.

        Returns a reference to the mapped value of the element
        that matches `key`, otherwise throws.

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        Strong guarantee.

        @return A reference to the mapped value.

        @param key The key of the element to find.

        @throw std::out_of_range if no such element exists.
    */
    /* @{ */
    BOOST_JSON_DECL
    value_view
    at(string_view key) const;

    /* @} */

    /** Count the number of elements with a specific key

        This function returns the count of the number of
        elements match `key`. The only possible return values
        are 0 and 1.

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.
    */
    std::size_t
    count(string_view key) const noexcept
    {
        return adaptor_->count(data_, key);
    }

    /** Find an element with a specific key

        This function returns a constant iterator to
        the element matching `key` if it exists,
        otherwise returns @ref end().

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.
    */
    const_iterator
    find(string_view key) const noexcept
    {
        return {data_, adaptor_->find(data_, key), adaptor_};
    }

    /** Return `true` if the key is found

        This function returns `true` if a key with the
        specified string is found.

        @par Effects
        @code
        return this->find(key) != this->end();
        @endcode

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.

        @see @ref find
    */
    bool
    contains(string_view key) const noexcept
    {
        return adaptor_->contains(data_, key);
    }

    /** Return a pointer to the value if the key is found, or null

        This function searches for a value with the given
        key, and returns a pointer to it if found. Otherwise
        it returns null.

        @par Example
        @code
        if( auto p = obj.if_contains( "key" ) )
            std::cout << *p;
        @endcode

        @par Complexity
        Constant on average, worst case linear in @ref size().

        @par Exception Safety
        No-throw guarantee.

        @param key The key of the element to find.

        @see @ref find
    */
    BOOST_JSON_DECL
    boost::optional<value_view>
    if_contains(string_view key) const noexcept;

    /** Return `true` if two objects are equal.

        Objects are equal when their sizes are the same,
        and when for each key in `lhs` there is a matching
        key in `rhs` with the same value.

        @par Complexity
        Constant, or linear (worst case quadratic) in `lhs.size()`.

        @par Exception Safety
        No-throw guarantee.
    */
    // inline friend speeds up overload resolution
    friend
    bool
    operator==(
        object_view const& lhs,
        object_view const& rhs) noexcept
    {
        return lhs.equal(rhs);
    }

    /** Return `true` if two objects are not equal.

        Objects are equal when their sizes are the same,
        and when for each key in `lhs` there is a matching
        key in `rhs` with the same value.

        @par Complexity
        Constant, or linear (worst case quadratic) in `lhs.size()`.

        @par Exception Safety
        No-throw guarantee.
    */
    // inline friend speeds up overload resolution
    friend
    bool
    operator!=(
        object_view const& lhs,
        object_view const& rhs) noexcept
    {
        return ! (lhs == rhs);
    }

    /** Serialize @ref object to an output stream.

        This function serializes an `object` as JSON into the output stream.

        @return Reference to `os`.

        @par Complexity
        Constant or linear in the size of `obj`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param os The output stream to serialize to.

        @param obj The value to serialize.
    */
    BOOST_JSON_DECL
    friend
    std::ostream&
    operator<<(
        std::ostream& os,
        object_view const& obj);
private:
    BOOST_JSON_DECL
    bool
    equal(object_view const& other) const noexcept;
};

} // namespace json
} // namespace boost

#ifndef BOOST_JSON_DOCS
// boost::hash trait
namespace boost
{
namespace container_hash
{

template< class T > struct is_unordered_range;

template<>
struct is_unordered_range< json::object_view >
    : std::true_type
{};

} // namespace container_hash
} // namespace boost

// std::hash specialization
namespace std {
template <>
struct hash< ::boost::json::object_view > {
    BOOST_JSON_DECL
    std::size_t
    operator()(::boost::json::object_view const& jo) const noexcept;
};
} // std
#endif


// Must be included here for this file to stand alone
#include <boost/json/value_view.hpp>

// includes are at the bottom of <boost/json/value.hpp>

#endif
