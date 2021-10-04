//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_PARSE_INTO_HPP
#define BOOST_JSON_DETAIL_PARSE_INTO_HPP

#include <boost/json/error.hpp>
#include <boost/mp11.hpp>
#include <boost/describe.hpp>

BOOST_JSON_NS_BEGIN

namespace detail {

#define BOOST_JSON_FAIL(ec, e) static constexpr auto loc = BOOST_CURRENT_LOCATION; ec.assign( e, &loc )

template<error E> class handler_error_base
{
public:

    handler_error_base() = default;

    handler_error_base( handler_error_base const& ) = delete;
    handler_error_base& operator=( handler_error_base const& ) = delete;

public:

    bool on_object_begin( error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_object_end( std::size_t, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_array_begin( error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_array_end( std::size_t, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_key_part( string_view, std::size_t, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_key( string_view, std::size_t, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_string_part( string_view, std::size_t, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_string( string_view, std::size_t, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_number_part( string_view, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_int64( std::int64_t, string_view, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_uint64( std::uint64_t, string_view, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_double( double, string_view, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_bool( bool, error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
    bool on_null( error_code& ec ) { BOOST_JSON_FAIL( ec, E ); return false; }
};

// integral_handler

template<class V, class P> class integral_handler: public handler_error_base<error::expected_integer>
{
private:

    V * value_;
    P * parent_;

public:

    integral_handler( integral_handler const& ) = delete;
    integral_handler& operator=( integral_handler const& ) = delete;

public:

    integral_handler( V* v, P* p ): value_( v ), parent_( p )
    {
    }

    bool on_number_part( string_view, error_code& )
    {
        return true;
    }

    bool on_int64( std::int64_t v, string_view, error_code& ec )
    {
        if( v < std::numeric_limits<V>::min() || v > std::numeric_limits<V>::max() )
        {
            BOOST_JSON_FAIL( ec, error::number_out_of_range );
            return false;
        }

        *value_ = static_cast<V>( v );

        parent_->signal_value();
        return true;
    }

    bool on_uint64( std::uint64_t v, string_view, error_code& ec )
    { 
        if( v > std::numeric_limits<V>::max() )
        {
            BOOST_JSON_FAIL( ec, error::number_out_of_range );
            return false;
        }

        *value_ = static_cast<V>( v );

        parent_->signal_value();
        return true;
    }

    bool on_array_end( std::size_t, error_code& )
    {
        parent_->signal_end();
        return true;
    }
};

// floating_point_handler

template<class V, class P> class floating_point_handler: public handler_error_base<error::expected_number>
{
private:

    V * value_;
    P * parent_;

public:

    floating_point_handler( floating_point_handler const& ) = delete;
    floating_point_handler& operator=( floating_point_handler const& ) = delete;

public:

    floating_point_handler( V* v, P* p ): value_( v ), parent_( p )
    {
    }

    bool on_number_part( string_view, error_code& )
    {
        return true;
    }

    bool on_int64( std::int64_t v, string_view, error_code& )
    {
        *value_ = static_cast<V>( v );

        parent_->signal_value();
        return true;
    }

    bool on_uint64( std::uint64_t v, string_view, error_code& )
    { 
        *value_ = static_cast<V>( v );

        parent_->signal_value();
        return true;
    }

    bool on_double( double v, string_view, error_code& )
    { 
        *value_ = static_cast<V>( v );

        parent_->signal_value();
        return true;
    }

    bool on_array_end( std::size_t, error_code& )
    {
        parent_->signal_end();
        return true;
    }
};

// string_handler

template<class V, class P> class string_handler: public handler_error_base<error::expected_string>
{
private:

    V * value_;
    P * parent_;

public:

    string_handler( V* v, P* p ): value_( v ), parent_( p )
    {
    }

    bool on_string_part( string_view sv, std::size_t, error_code& )
    {
        value_->append( sv.begin(), sv.end() );
        return true;
    }

    bool on_string( string_view sv, std::size_t, error_code& )
    {
        value_->append( sv.begin(), sv.end() );

        parent_->signal_value();
        return true;
    }

    bool on_array_end( std::size_t, error_code& )
    {
        parent_->signal_end();
        return true;
    }
};

// bool_handler

template<class V, class P> class bool_handler: public handler_error_base<error::expected_bool>
{
private:

    V * value_;
    P * parent_;

public:

    bool_handler( bool_handler const& ) = delete;
    bool_handler& operator=( bool_handler const& ) = delete;

public:

    bool_handler( V* v, P* p ): value_( v ), parent_( p )
    {
    }

    bool on_bool( bool v, error_code& )
    {
        *value_ = v;

        parent_->signal_value();
        return true;
    }

    bool on_array_end( std::size_t, error_code& )
    {
        parent_->signal_end();
        return true;
    }
};

// forward declarations

template<class V, class P> class sequence_handler;
template<class V, class P> class map_handler;
template<class V, class P> class tuple_handler;
template<class V, class P> class described_struct_handler;

template<class V> struct unknown_type_handler
{
    static_assert( sizeof(V) == 0, "This type is not supported" );
};

// is_string

template<class T> struct is_string: std::is_same<T, std::string>
{
};

// is_map

template<class T, class E = void> struct is_map: std::false_type
{
};

template<class T> struct is_map<T, decltype( std::declval<T&>().emplace( std::declval<std::string>(), std::declval<typename T::mapped_type>() ), (void)0 )>: std::true_type
{
};

// is_sequence

template<class T, class E = void> struct is_sequence: std::false_type
{
};

template<class T> struct is_sequence<T, decltype( std::declval<T&>().push_back( std::declval<typename T::value_type>() ), (void)0 )>: std::true_type
{
};

// is_tuple

template<class T, class E = void> struct is_tuple: std::false_type
{
};

template<class T> struct is_tuple<T, decltype( (void)std::tuple_size<T>::value )>: std::true_type
{
};

// is_described_struct

template<class T, class E = void> struct is_described_struct: std::false_type
{
};

template<class T> struct is_described_struct<T, decltype((void)boost::describe::describe_members<T, boost::describe::mod_any_access>())>: std::true_type
{
};

// get_handler

template<class V, class P> using get_handler = boost::mp11::mp_cond<

    std::is_same<V, bool>, bool_handler<V, P>,
    std::is_integral<V>, integral_handler<V, P>,
    std::is_floating_point<V>, floating_point_handler<V, P>,
    is_string<V>, string_handler<V, P>,
    is_map<V>, map_handler<V, P>,
    is_sequence<V>, sequence_handler<V, P>,
    is_tuple<V>, tuple_handler<V, P>,
    is_described_struct<V>, described_struct_handler<V, P>,
    boost::mp11::mp_true, unknown_type_handler<V>
>;

// sequence_handler

template<class V, class P> class sequence_handler
{
private:

    V * value_;
    P * parent_;

    using value_type = typename V::value_type;
    using inner_handler_type = get_handler<value_type, sequence_handler>;

    value_type next_value_ = {};

    inner_handler_type inner_;
    bool inner_active_ = false;

public:

    sequence_handler( sequence_handler const& ) = delete;
    sequence_handler& operator=( sequence_handler const& ) = delete;

public:

    sequence_handler( V* v, P* p ): value_( v ), parent_( p ), inner_( &next_value_, this )
    {
    }

    void signal_value()
    {
        value_->push_back( std::move( next_value_ ) );
        next_value_ = {};
    }

    void signal_end()
    {
        inner_active_ = false;
        parent_->signal_value();
    }

#define BOOST_JSON_INVOKE_INNER(f) if( !inner_active_ ) { \
    BOOST_JSON_FAIL( ec, error::expected_array ); return false; } else return inner_.f

    bool on_object_begin( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_object_begin( ec ) );
    }

    bool on_object_end( std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_object_end( n, ec ) );
    }
    
    bool on_array_begin( error_code& ec )
    {
        if( inner_active_ )
        {
            return inner_.on_array_begin( ec );
        }
        else
        {
            inner_active_ = true;
            return true;
        }
    }

    bool on_array_end( std::size_t n, error_code& ec )
    {
        if( inner_active_ )
        {
            return inner_.on_array_end( n, ec );
        }
        else
        {
            parent_->signal_end();
            return true;
        }
    }

    bool on_key_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_key_part( sv, n, ec ) );
    }

    bool on_key( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_key( sv, n, ec ) );
    }

    bool on_string_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string_part( sv, n, ec ) );
    }
    
    bool on_string( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string( sv, n, ec ) );
    }

    bool on_number_part( string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_number_part( sv, ec ) );
    }
    
    bool on_int64( std::int64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_int64( v, sv, ec ) );
    }
    
    bool on_uint64( std::uint64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_uint64( v, sv, ec ) );
    }
    
    bool on_double( double v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_double( v, sv, ec ) );
    }
    
    bool on_bool( bool v, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_bool( v, ec ) );
    }
    
    bool on_null( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_null( ec ) );
    }

#undef BOOST_JSON_INVOKE_INNER
};

// map_handler

template<class V, class P> class map_handler
{
private:

    V * value_;
    P * parent_;

    using mapped_type = typename V::mapped_type;
    using inner_handler_type = get_handler<mapped_type, map_handler>;

    std::string key_;
    mapped_type next_value_ = {};

    inner_handler_type inner_;
    bool inner_active_ = false;

public:

    map_handler( map_handler const& ) = delete;
    map_handler& operator=( map_handler const& ) = delete;

public:

    map_handler( V* v, P* p ): value_( v ), parent_( p ), inner_( &next_value_, this )
    {
    }

    void signal_value()
    {
        value_->emplace( std::move( key_ ), std::move( next_value_ ) );

        key_ = {};
        next_value_ = {};

        inner_active_ = false;
    }

    void signal_end()
    {
        inner_active_ = false;
        parent_->signal_value();
    }

#define BOOST_JSON_INVOKE_INNER(f) if( !inner_active_ ) { \
    BOOST_JSON_FAIL( ec, error::expected_object ); return false; } else return inner_.f

    bool on_object_begin( error_code& ec )
    {
        if( inner_active_ )
        {
            return inner_.on_object_begin( ec );
        }

        return true;
    }

    bool on_object_end( std::size_t n, error_code& ec )
    {
        if( inner_active_ )
        {
            return inner_.on_object_end( n, ec );
        }

        parent_->signal_value();
        return true;
    }
    
    bool on_array_begin( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_array_begin( ec ) );
    }

    bool on_array_end( std::size_t n, error_code& ec )
    {
        if( inner_active_ )
        {
            return inner_.on_array_end( n, ec );
        }

        parent_->signal_end();
        return true;
    }

    bool on_key_part( string_view sv, std::size_t n, error_code& ec )
    {
        if( inner_active_ )
        {
            return inner_.on_key_part( sv, n, ec );
        }

        key_.append( sv.data(), sv.size() );
        return true;
    }

    bool on_key( string_view sv, std::size_t n, error_code& ec )
    {
        if( inner_active_ )
        {
            return inner_.on_key( sv, n, ec );
        }

        key_.append( sv.data(), sv.size() );

        inner_active_ = true;
        return true;
    }

    bool on_string_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string_part( sv, n, ec ) );
    }
    
    bool on_string( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string( sv, n, ec ) );
    }

    bool on_number_part( string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_number_part( sv, ec ) );
    }
    
    bool on_int64( std::int64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_int64( v, sv, ec ) );
    }
    
    bool on_uint64( std::uint64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_uint64( v, sv, ec ) );
    }
    
    bool on_double( double v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_double( v, sv, ec ) );
    }
    
    bool on_bool( bool v, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_bool( v, ec ) );
    }
    
    bool on_null( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_null( ec ) );
    }

#undef BOOST_JSON_INVOKE_INNER
};

// into_handler tuple

template<std::size_t I, class T> struct handler_tuple_element
{
    T t_;
};

template<class S, class... T> struct handler_tuple_impl;

template<std::size_t... I, class... T> struct handler_tuple_impl<boost::mp11::index_sequence<I...>, T...>: handler_tuple_element<I, T>...
{
};

template<class P, class... V> struct handler_tuple: public handler_tuple_impl<boost::mp11::index_sequence_for<V...>, get_handler<V, P>...>
{
    using base_type = handler_tuple_impl<boost::mp11::index_sequence_for<V...>, get_handler<V, P>...>;

    template<class... A> handler_tuple( A... a ): base_type{ { { a.first, a.second } }... }
    {
    }

    handler_tuple( handler_tuple const& ) = delete;
    handler_tuple& operator=( handler_tuple const& ) = delete;
};

template<std::size_t I, class T> T& get( handler_tuple_element<I, T>& e )
{
    return e.t_;
}

// tuple_handler

template<class P, class T> struct tuple_inner_handlers;

template<class P, template<class...> class L, class... V> struct tuple_inner_handlers<P, L<V...>>
{
    handler_tuple<P, V...> handlers_;

    template<std::size_t... I> tuple_inner_handlers( L<V...>* pv, P* pp, boost::mp11::index_sequence<I...> ): handlers_( std::make_pair( &get<I>(*pv), pp )... )
    {
    }
};

template<class T, class P> class tuple_handler
{
private:

    T * value_;
    P * parent_;

    std::string key_;

    tuple_inner_handlers<tuple_handler, T> inner_;
    int inner_active_ = -1;

public:

    tuple_handler( tuple_handler const& ) = delete;
    tuple_handler& operator=( tuple_handler const& ) = delete;

public:

    tuple_handler( T* v, P* p ): value_( v ), parent_( p ), inner_( v, this, boost::mp11::make_index_sequence< std::tuple_size<T>::value >() )
    {
    }

    void signal_value()
    {
        ++inner_active_;
    }

    void signal_end()
    {
        inner_active_ = -1;
        parent_->signal_value();
    }

#define BOOST_JSON_INVOKE_INNER(fn) \
    if( inner_active_ < 0 ) \
    { \
        BOOST_JSON_FAIL( ec, error::expected_array ); \
        return false; \
    } \
    constexpr std::size_t N = std::tuple_size<T>::value; \
    if( inner_active_ >= N ) \
    { \
        BOOST_JSON_FAIL( ec, error::invalid_array_size ); \
        return false; \
    } \
    return boost::mp11::mp_with_index<N>( inner_active_, [&](auto I){ \
        return get<I>( inner_.handlers_ ).fn; \
    });

    //---

    bool on_object_begin( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_object_begin( ec ) );
    }

    bool on_object_end( std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_object_end( n, ec ) );
    }
    
    bool on_array_begin( error_code& ec )
    {
        if( inner_active_ < 0 )
        {
            inner_active_ = 0;
            return true;
        }

        constexpr std::size_t N = std::tuple_size<T>::value;

        if( inner_active_ >= N )
        {
            inner_active_ = 0;
            return true;
        }

        return boost::mp11::mp_with_index<N>( inner_active_, [&](auto I){

            return get<I>( inner_.handlers_ ).on_array_begin( ec );

        });
    }

    bool on_array_end( std::size_t n, error_code& ec )
    {
        if( inner_active_ < 0 )
        {
            parent_->signal_end();
            return true;
        }

        constexpr std::size_t N = std::tuple_size<T>::value;

        if( inner_active_ >= N )
        {
            parent_->signal_value();
            return true;
        }

        return boost::mp11::mp_with_index<N>( inner_active_, [&](auto I){

            return get<I>( inner_.handlers_ ).on_array_end( n, ec );

        });
    }

    bool on_key_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_key_part( sv, n, ec ) );
    }

    bool on_key( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_key( sv, n, ec ) );
    }

    bool on_string_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string_part( sv, n, ec ) );
    }
    
    bool on_string( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string( sv, n, ec ) );
    }

    bool on_number_part( string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_number_part( sv, ec ) );
    }
    
    bool on_int64( std::int64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_int64( v, sv, ec ) );
    }
    
    bool on_uint64( std::uint64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_uint64( v, sv, ec ) );
    }
    
    bool on_double( double v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_double( v, sv, ec ) );
    }
    
    bool on_bool( bool v, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_bool( v, ec ) );
    }
    
    bool on_null( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_null( ec ) );
    }

#undef BOOST_JSON_INVOKE_INNER
};

// described_struct_handler

template<class T, class D> using struct_member_type = std::remove_reference_t< decltype( std::declval<T&>().*D::pointer ) >;

template<class P, class T, class L> struct struct_inner_handlers;

template<class P, class T, template<class...> class L, class... D> struct struct_inner_handlers<P, T, L<D...>>
{
    handler_tuple<P, struct_member_type<T, D>...> handlers_;

    struct_inner_handlers( T* pv, P* pp ): handlers_( std::make_pair( &(pv->*D::pointer), pp )... )
    {
    }
};

template<class V, class P> class described_struct_handler
{
private:

    V * value_;
    P * parent_;

    std::string key_;

    using Dm = boost::describe::describe_members<V, boost::describe::mod_public>;

    struct_inner_handlers<described_struct_handler, V, Dm> inner_;
    int inner_active_ = -1;

public:

    described_struct_handler( described_struct_handler const& ) = delete;
    described_struct_handler& operator=( described_struct_handler const& ) = delete;

public:

    described_struct_handler( V* v, P* p ): value_( v ), parent_( p ), inner_( v, this )
    {
    }

    void signal_value()
    {
        key_ = {};
        inner_active_ = -1;
    }

    void signal_end()
    {
        key_ = {};
        inner_active_ = -1;

        parent_->signal_value();
    }

#define BOOST_JSON_INVOKE_INNER(fn) \
    if( inner_active_ < 0 ) \
    { \
        BOOST_JSON_FAIL( ec, error::expected_object ); \
        return false; \
    } \
    return boost::mp11::mp_with_index<boost::mp11::mp_size<Dm>>( inner_active_, [&](auto I){ \
        return get<I>( inner_.handlers_ ).fn; \
    });

    //---

    bool on_object_begin( error_code& ec )
    {
        if( inner_active_ < 0 )
        {
            return true;
        }

        BOOST_JSON_INVOKE_INNER( on_object_begin( ec ) );
    }

    bool on_object_end( std::size_t n, error_code& ec )
    {
        if( inner_active_ < 0 )
        {
            parent_->signal_value();
            return true;
        }

        BOOST_JSON_INVOKE_INNER( on_object_end( n, ec ) );
    }
    
    bool on_array_begin( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_array_begin( ec ) );
    }

    bool on_array_end( std::size_t n, error_code& ec )
    {
        if( inner_active_ < 0 )
        {
            parent_->signal_end();
            return true;
        }

        BOOST_JSON_INVOKE_INNER( on_array_end( n, ec ) );
    }

    bool on_key_part( string_view sv, std::size_t n, error_code& ec )
    {
        if( inner_active_ < 0 )
        {
            key_.append( sv.data(), sv.size() );
            return true;
        }

        BOOST_JSON_INVOKE_INNER( on_key_part( sv, n, ec ) );
    }

    bool on_key( string_view sv, std::size_t n, error_code& ec )
    {
        if( inner_active_ >= 0 )
        {
            BOOST_JSON_INVOKE_INNER( on_key( sv, n, ec ) );
        }

        key_.append( sv.data(), sv.size() );

        int i = 0;

        boost::mp11::mp_for_each<Dm>([&](auto D){

            if( key_ == D.name )
            {
                inner_active_ = i;
            }

            ++i;
        });

        if( inner_active_ < 0 )
        {
            BOOST_JSON_FAIL( ec, error::invalid_member_name );
            return false;
        }

        return true;
    }

    bool on_string_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string_part( sv, n, ec ) );
    }
    
    bool on_string( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string( sv, n, ec ) );
    }

    bool on_number_part( string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_number_part( sv, ec ) );
    }
    
    bool on_int64( std::int64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_int64( v, sv, ec ) );
    }
    
    bool on_uint64( std::uint64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_uint64( v, sv, ec ) );
    }
    
    bool on_double( double v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_double( v, sv, ec ) );
    }
    
    bool on_bool( bool v, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_bool( v, ec ) );
    }
    
    bool on_null( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_null( ec ) );
    }

#undef BOOST_JSON_INVOKE_INNER
};

// into_handler

template<class V> class into_handler
{
private:

    using inner_handler_type = get_handler<V, into_handler>;

    inner_handler_type inner_;
    bool inner_active_ = true;

public:

    into_handler( into_handler const& ) = delete;
    into_handler& operator=( into_handler const& ) = delete;

public:

    constexpr static std::size_t max_object_size = std::size_t(-1);
    constexpr static std::size_t max_array_size = std::size_t(-1);
    constexpr static std::size_t max_key_size = std::size_t(-1);
    constexpr static std::size_t max_string_size = std::size_t(-1);

public:

    explicit into_handler( V* v ): inner_( v, this )
    {
    }

    void signal_value()
    {
    }

    void signal_end()
    {
    }

    bool on_document_begin( error_code& )
    {
        return true;
    }

    bool on_document_end( error_code& )
    {
        inner_active_ = false;
        return true;
    }

#define BOOST_JSON_INVOKE_INNER(f) if( !inner_active_ ) { BOOST_JSON_FAIL( ec, error::extra_data ); return false; } else return inner_.f

    bool on_object_begin( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_object_begin( ec ) );
    }

    bool on_object_end( std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_object_end( n, ec ) );
    }
    
    bool on_array_begin( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_array_begin( ec ) );
    }
    
    bool on_array_end( std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_array_end( n, ec ) );
    }

    bool on_key_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_key_part( sv, n, ec ) );
    }

    bool on_key( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_key( sv, n, ec ) );
    }

    bool on_string_part( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string_part( sv, n, ec ) );
    }
    
    bool on_string( string_view sv, std::size_t n, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_string( sv, n, ec ) );
    }

    bool on_number_part( string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_number_part( sv, ec ) );
    }
    
    bool on_int64( std::int64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_int64( v, sv, ec ) );
    }
    
    bool on_uint64( std::uint64_t v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_uint64( v, sv, ec ) );
    }
    
    bool on_double( double v, string_view sv, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_double( v, sv, ec ) );
    }
    
    bool on_bool( bool v, error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_bool( v, ec ) );
    }
    
    bool on_null( error_code& ec )
    {
        BOOST_JSON_INVOKE_INNER( on_null( ec ) );
    }
    
    bool on_comment_part(string_view, error_code&)
    {
        return true;
    }
    
    bool on_comment(string_view, error_code&)
    {
        return true;
    }

#undef BOOST_JSON_INVOKE_INNER
};

} // detail

BOOST_JSON_NS_END

#endif
