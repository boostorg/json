//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_ARRAY_VIEW_HPP
#define BOOST_JSON_ARRAY_VIEW_HPP

#include <boost/json/detail/config.hpp>
#include <boost/json/conversion.hpp>
#include <boost/json/kind.hpp>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <boost/optional.hpp>

namespace boost {
namespace json {

#ifndef BOOST_JSON_DOCS
class array;
class value_view;
#endif

/** A view of an array of JSON values

    This is the type used to represent a JSON array as
    a modifiable container. The interface and performance
    characteristics are modeled after `std::vector<value>`.
\n
    Elements are stored contiguously, which means that
    they can be accessed not only through iterators, but
    also using offsets to regular pointers to elements. A
    pointer to an element of an @ref array may be passed to
    any function that expects a pointer to @ref value.
\n
    The storage of the array is handled automatically, being
    expanded and contracted as needed. Arrays usually occupy
    more space than array language constructs, because more
    memory is allocated to handle future growth. This way an
    array does not need to reallocate each time an element
    is inserted, but only when the additional memory is used
    up. The total amount of allocated memory can be queried
    using the @ref capacity function. Extra memory can be
    relinquished by calling @ref shrink_to_fit.
    \n

    Reallocations are usually costly operations in terms of
    performance. The @ref reserve function can be used to
    eliminate reallocations if the number of elements is
    known beforehand.
\n
    The complexity (efficiency) of common operations on
    arrays is as follows:

    @li Random access - constant *O(1)*.
    @li Insertion or removal of elements at the
        end - amortized constant *O(1)*.
    @li Insertion or removal of elements - linear in
        the distance to the end of the array *O(n)*.

    @par Allocators

    All elements stored in the container, and their
    children if any, will use the same memory resource
    that was used to construct the container.

    @par Thread Safety

    Non-const member functions may not be called
    concurrently with any other member functions.

    @par Satisfies
        <a href="https://en.cppreference.com/w/cpp/named_req/ContiguousContainer"><em>ContiguousContainer</em></a>,
        <a href="https://en.cppreference.com/w/cpp/named_req/ReversibleContainer"><em>ReversibleContainer</em></a>, and
        <a href="https://en.cppreference.com/w/cpp/named_req/SequenceContainer"><em>SequenceContainer</em></a>.
*/
class array_view
{
    struct adaptor
    {
        virtual bool            empty(const void * data)   const = 0;
        virtual std::size_t      size(const void * data)   const = 0;
        virtual std::size_t  max_size(const void * data)   const = 0;
        virtual std::size_t  capacity(const void * data)   const = 0;
        virtual value_view         at(const void * data, std::size_t key) const = 0;
        virtual value_view     get_at(const void * data, std::size_t key) const = 0;
        virtual value_view      front(const void * data) const = 0;
        virtual value_view       back(const void * data) const = 0;
        virtual boost::optional<value_view> if_contains(const void * data, std::size_t key) const = 0;

        virtual const void * begin(const void *data) const = 0;
        virtual const void * end  (const void *data) const = 0;
        virtual const void * next (const void* elem, std::ptrdiff_t  change) const = 0;
        virtual std::intptr_t distance(const void* itr1, const void * itr2) const = 0;
        virtual value_view dereference(const void *data, const void * elem) const = 0;
    };

    kind k_ = kind::array;  // must come first
    const void * data_;
    const adaptor * adaptor_;

    struct empty_adaptor;
    struct array_adaptor;
    static const empty_adaptor empty_impl_;
    static const array_adaptor array_impl_;

    template<typename Container>
    struct container_adaptor;

    template<typename T, std::size_t Size>
    struct raw_array_adaptor;

    template<typename Container>
    struct tuple_adaptor;
public:
    /// A random access const iterator to an element
    struct const_iterator
    {
        typedef void pointer;
        typedef void reference;
        typedef std::random_access_iterator_tag iterator_category;
        typedef value_view value_type;
        typedef std::ptrdiff_t difference_type;

        const_iterator & operator+=(std::size_t pos)
        {
            elem = adaptor_->next(elem, static_cast<std::intptr_t>(pos));
            return *this;
        }
        const_iterator & operator-=(std::size_t pos)
        {
            elem = adaptor_->next( elem, -static_cast<std::intptr_t>(pos));
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

        BOOST_JSON_DECL value_view operator*() const;
        BOOST_JSON_DECL value_view operator[](std::size_t pos) const;

        constexpr const_iterator() = default;
        constexpr const_iterator(const const_iterator & ) = default;
        constexpr const_iterator(const_iterator &&) = default;
    private:
        constexpr const_iterator(const void * data,  const void * elem, const adaptor * adaptor_)
                                    : data(data), elem(elem), adaptor_(adaptor_) {}
        const void * data{nullptr}, * elem{nullptr};
        const adaptor * adaptor_{nullptr};
        friend class array_view;
    };

    /** The type of <em>Allocator</em> returned by @ref get_allocator

        This type is a @ref polymorphic_allocator.
    */

    /// The type used to represent unsigned integers
    using size_type = std::size_t;

    /// The type of each element
    using value_type = value_view;

    /// The type used to represent signed integers
    using difference_type = std::ptrdiff_t;

    /// A reference to an element
    using reference = value_view&;

    /// A const reference to an element
    using const_reference = value_view const&;

    /// A pointer to an element
    using pointer = value_view*;

    /// A const pointer to an element
    using const_pointer = value_view const*;


    /// A reverse random access const iterator to an element
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
    ~array_view() noexcept = default;

    //------------------------------------------------------

    /** Constructor.

        The constructed array is empty with zero
        capacity, using the default memory resource.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    BOOST_JSON_DECL
    array_view() noexcept;

    /** Copy constructor.

        The array is constructed with a copy of the
        contents of `other`, using `other`'s memory resource.

        @par Complexity
        Linear in `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The array to copy
    */
    array_view(array_view const& other) = default;

    /** Move constructor.

        The array is constructed by acquiring ownership of
        the contents of `other` and shared ownership of
        `other`'s memory resource.

        @note

        After construction, the moved-from array behaves
        as if newly constructed with its current storage
        pointer.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param other The container to move
    */
    array_view(array_view && other) noexcept = default;
    //------------------------------------------------------

    /** Copy assignment.

        The contents of the array are replaced with an
        element-wise copy of `other`.

        @par Complexity
        Linear in @ref size() plus `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The array to copy.
    */
    array_view&
    operator=(array_view const& other) noexcept = default;

    /** Move assignment.

        The contents of the array are replaced with the
        contents of `other` using move semantics:

        @li If `*other.storage() == *sp`, ownership of
        the underlying memory is transferred in constant
        time, with no possibility of exceptions.
        After assignment, the moved-from array behaves
        as if newly constructed with its current storage
        pointer.

        @li If `*other.storage() != *sp`, an
        element-wise copy is performed, which may throw.
        In this case, the moved-from array is not
        changed.

        @par Complexity
        Constant, or linear in
        `this->size()` plus `other.size()`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param other The array to move.
    */
    array_view&
      operator=(array_view&& other) noexcept = default;

    BOOST_JSON_DECL
    array_view(const json::array & arr) noexcept;

    template<typename T, std::size_t Size>
    array_view(const T (&arr)[Size]) noexcept;

    template<typename Range >
    array_view(const Range & r,
               typename std::enable_if<
                       std::is_constructible<value_view, decltype(*r.data())>::value
                   && !std::is_convertible<Range, string_view>::value,
                       decltype(r.size())
               >::type = 0u) noexcept;

    template<typename Tuple >
    array_view(const Tuple & r,
               typename std::enable_if<
                   is_tuple_like<Tuple>::value &&
                   mp11::mp_all_of_q<Tuple, mp11::mp_bind<std::is_constructible, value_view, mp11::_1>>::value>::type * = nullptr) noexcept;

    //------------------------------------------------------

    BOOST_JSON_DECL
    value_view
    at(std::size_t pos) const;
    /* @} */

    /** Access an element.

        Returns a reference to the element specified at
        location `pos`. No bounds checking is performed.

        @par Precondition
        `pos < size()`

        @par Complexity
        Constant.

        @param pos A zero-based index
    */
    /* @{ */
    BOOST_JSON_DECL
    value_view
    operator[](std::size_t pos) const noexcept;
    /* @} */

    /** Access the first element.

        Returns a reference to the first element.

        @par Precondition
        `not empty()`

        @par Complexity
        Constant.
    */
    /* @{ */
    BOOST_JSON_DECL
    value_view
    front() const noexcept;
    /* @} */

    /** Access the last element.

        Returns a reference to the last element.

        @par Precondition
        `not empty()`

        @par Complexity
        Constant.
    */
    /* @{ */
    BOOST_JSON_DECL
    value_view
    back() const noexcept;
    /* @} */

    /** Return a pointer to an element, or nullptr if the index is invalid

        This function returns a pointer to the element
        at index `pos` when the index is less then the size
        of the container. Otherwise it returns null.

        @par Example
        @code
        if( auto p = arr.if_contains( 1 ) )
            std::cout << *p;
        @endcode

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.

        @param pos The index of the element to return.
    */
    BOOST_JSON_DECL
    const boost::optional<value_view>
    if_contains(std::size_t pos) const noexcept;

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
        The element acts as a placeholder; attempting
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
        The element acts as a placeholder; attempting
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

    /** Return the number of elements in the array.

        This returns the number of elements in the array.
        The value returned may be different from the number
        returned from @ref capacity.

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

    /** Return the maximum number of elements any array can hold.

        The maximum is an implementation-defined number.
        This value is a theoretical limit; at runtime,
        the actual maximum size may be less due to
        resource limits.

        @par Complexity
        Constant.

        @par Exception Safety
        No-throw guarantee.
    */
    inline
    std::size_t
    max_size() noexcept
    {
        return adaptor_->max_size(data_);
    }

    /** Return the number of elements that can be held in currently allocated memory.

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

    /** Check if the array has no elements.

        Returns `true` if there are no elements in the
        array, i.e. @ref size() returns 0.

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

    /** Return `true` if two arrays are equal.

        Arrays are equal when their sizes are
        the same, and they are element-for-element
        equal in order.

        @par Effects
        `return std::equal( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );`

        @par Complexity
        Constant or linear in `lhs.size()`.

        @par Exception Safety
        No-throw guarantee.
    */
    // inline friend speeds up overload resolution
    friend
    bool
    operator==(
        array_view const& lhs,
        array_view const& rhs) noexcept
    {
        return lhs.equal(rhs);
    }

    /** Return `true` if two arrays are not equal.

        Arrays are equal when their sizes are
        the same, and they are element-for-element
        equal in order.

        @par Effects
        `return ! std::equal( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );`

        @par Complexity
        Constant or linear in `lhs.size()`.

        @par Exception Safety
        No-throw guarantee.
    */
    // inline friend speeds up overload resolution
    friend
    bool
    operator!=(
        array_view const& lhs,
        array_view const& rhs) noexcept
    {
        return ! (lhs == rhs);
    }

    /** Serialize @ref array to an output stream.

        This function serializes an `array`` as JSON into the output stream.

        @return Reference to `os`.

        @par Complexity
        Constant or linear in the size of `arr`.

        @par Exception Safety
        Strong guarantee.
        Calls to `memory_resource::allocate` may throw.

        @param os The output stream to serialize to.

        @param arr The value to serialize.
    */
    BOOST_JSON_DECL
    friend
    std::ostream&
    operator<<(
        std::ostream& os,
        array_view const& arr);

private:
    BOOST_JSON_DECL
    bool
    equal(array_view const& other) const noexcept;
};

} // namespace json
} // namespace boost

// std::hash specialization
#ifndef BOOST_JSON_DOCS
namespace std {
template <>
struct hash< ::boost::json::array_view > {
    BOOST_JSON_DECL
    std::size_t
    operator()(::boost::json::array_view const& ja) const noexcept;
};
} // std
#endif

// Must be included here for this file to stand alone
#include <boost/json/value_view.hpp>

// includes are at the bottom of <boost/json/value.hpp>


#endif
