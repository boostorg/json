//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

// Test that header file is self-contained.
#include <boost/json/memory_resource.hpp>

#include <boost/json/monotonic_resource.hpp>
#include <boost/json/static_resource.hpp>
#include <boost/json/value.hpp>

#ifndef BOOST_JSON_STANDALONE
#include <boost/container/pmr/vector.hpp>
#endif
#include <vector>

#include "test_suite.hpp"

#ifdef assert
#undef assert
#endif
#define assert BOOST_TEST

BOOST_JSON_NS_BEGIN

namespace snippets1 {

    //--------------------------------------

    namespace std {
    template<class T>
    using allocator = ::std::allocator<T>;
    } // std

    //[snippet_background_1

    namespace std {

    template< class T, class Allocator = std::allocator< T > >
    class vector;

    } // namespace std

    //]

    //--------------------------------------

    //[snippet_background_2

    namespace std {

    template< class T, class Allocator >
    class vector
    {
    public:
        explicit vector( Allocator const& alloc );

        //...

    //]

    };

    template<class T, class A>
    vector<T,A>::vector(A const&){}

    }

    //--------------------------------------

    //[snippet_background_3

    namespace std {
    namespace pmr {

    class memory_resource
    {
    public:
        virtual ~memory_resource();

        void* allocate  ( size_t bytes, size_t alignment );
        void  deallocate( void* p, size_t bytes, size_t alignment );
        bool  is_equal  ( const memory_resource& other ) const;

    protected:
        virtual void* do_allocate  ( size_t bytes, size_t alignment ) = 0;
        virtual void  do_deallocate( void* p, size_t bytes, size_t alignment ) = 0;
        virtual bool  do_is_equal  ( memory_resource const& other ) const noexcept = 0;
    };

    } // namespace pmr
    } // namespace std

    //]

    //--------------------------------------

    //[snippet_background_4

    namespace std {
    namespace pmr {

    template< class T >
    using vector = std::vector< T, polymorphic_allocator< T > >;

    } // namespace pmr
    } // namespace std

    //]

    //--------------------------------------

    struct my_resource : memory_resource
    {
        void* do_allocate  ( size_t, size_t ) override { return 0; }
        void  do_deallocate( void*, size_t, size_t ) override {}
        bool  do_is_equal  ( memory_resource const& ) const noexcept override { return true; }
    };

    //[snippet_background_7

    namespace my_library {

    std::pmr::vector<char> get_chars1()
    {
        // This leaks memory because `v` does not own the memory resource
        std::pmr::vector<char> v( new my_resource );

        return v;
    }

    } // my_library

    //]

    //--------------------------------------

    //[snippet_background_8

    namespace my_library {

    std::pmr::vector<char> get_chars2()
    {
        // Declare a local memory resource
        my_resource mr;

        // Construct a vector that uses our resource
        std::pmr::vector<char> v( &mr );

        // Undefined behavior, `mr` goes out of scope!
        return v;
    }

    } // my_library

    //]

    //--------------------------------------

} // snippets

namespace snippets2 {
#ifdef BOOST_JSON_STANDALONE
template<class T>
using vector = std::pmr::vector<T>;
#else
template<class T>
using vector = boost::container::pmr::vector<T>;
#endif
} // snippets2

class memory_resource_test
{
public:
    void
    testSnippets()
    {
        {
            using namespace snippets2;

            struct T {};

            //[snippet_background_5

            // A type of memory resource
            monotonic_resource mr;

            // Construct a vector using the monotonic buffer resource
            vector< T > v1(( polymorphic_allocator< T >(&mr) ));

            // Or this way, since construction from memory_resource* is implicit:
            vector< T > v2( &mr );

            //]
        }

        //----------------------------------

        {
            using namespace snippets2;

            //[snippet_background_6

            {
                // A type of memory resource which uses a stack buffer
                char temp[4096];
                static_resource mr( temp, sizeof(temp) );

                // Construct a vector using the static buffer resource
                vector< value > v( &mr );

                // The vector will allocate from `temp` first, and then the heap.
            }

            //]
        }

        //----------------------------------

        {
            //[snippet_uses_allocator_1

            // We want to use this resource for all the containers
            monotonic_resource mr;

            // Declare a vector of JSON values
            std::vector< value, polymorphic_allocator< value > > v( &mr );

            // The polymorphic allocator will use our resource
            assert( v.get_allocator().resource() == &mr );

            // Add a string to the vector
            v.emplace_back( "boost" );

            // The vector propagates the memory resource to the string
            assert( v[0].storage().get() == &mr );

            //]
        }
        {
            //[snippet_uses_allocator_2

            // This vector will use the default memory resource
            std::vector< value, polymorphic_allocator < value > > v;

            // This value will same memory resource as the vector
            value jv( v.get_allocator() );

            // However, ownership is not transferred,
            assert( ! jv.storage().is_counted() );

            // and deallocate is never null
            assert( ! jv.storage().is_deallocate_trivial() );

            //]
        }
    }

    void
    testBoostPmr()
    {
#ifndef BOOST_JSON_STANDALONE
        using allocator_type =
            polymorphic_allocator<value>;

        // pass polymorphic_allocator
        // where storage_ptr is expected
        {
            monotonic_resource mr;
            value  jv( allocator_type{&mr} );
            object  o( allocator_type{&mr} );
            array   a( allocator_type{&mr} );
            string  s( allocator_type{&mr} );
        }
        {
            monotonic_resource mr;
            allocator_type a(&mr);

            boost::container::pmr::vector<value> v1(a);
            v1.resize(3);
            BOOST_TEST(v1[1].storage().get() == &mr);

            std::vector<value, allocator_type> v2(3, {}, a);
            BOOST_TEST(v2[1].storage().get() == &mr);
        }
#endif
    }

    void
    testStdPmr()
    {
#ifdef BOOST_JSON_STANDALONE
        using allocator_type =
            std::pmr::polymorphic_allocator<value>;

        // pass polymorphic_allocator
        // where storage_ptr is expected
        {
            value  jv( allocator_type{} );
            object  o( allocator_type{} );
            array   a( allocator_type{} );
            string  s( allocator_type{} );
        }
        {
            monotonic_resource mr;
            allocator_type a(&mr);

            std::vector<value, allocator_type> v2(3, {}, a);
            BOOST_TEST(v2[1].storage().get() == &mr);
        }
#endif
    }

    // These are here instead of the type-specific
    // test TUs, so that we only need to link to
    // Boost.Container from one file.
    void
    testPmr()
    {
        // array
        {
            // get_allocator
            {
                monotonic_resource mr;
                array a(&mr);
                BOOST_TEST(a.get_allocator().resource() == &mr);
            }
            {
                object a(make_counted_resource<
                    monotonic_resource>());
                BOOST_TEST_THROWS(
                    a.get_allocator(),
                    std::invalid_argument);
            }
        }
        // object
        {
            // get_allocator
            {
                monotonic_resource mr;
                object o(&mr);
                BOOST_TEST(o.get_allocator().resource() == &mr);
            }
            {
                object o(make_counted_resource<
                    monotonic_resource>());
                BOOST_TEST_THROWS(
                    o.get_allocator(),
                    std::invalid_argument);
            }
        }
        // string
        {
            // get_allocator
            {
                monotonic_resource mr;
                string s(&mr);
                BOOST_TEST(s.get_allocator().resource() == &mr);
            }
            {
                object s(make_counted_resource<
                    monotonic_resource>());
                BOOST_TEST_THROWS(
                    s.get_allocator(),
                    std::invalid_argument);
            }
        }
    }

    void
    run()
    {
        testSnippets();
        testBoostPmr();
        testStdPmr();
        testPmr();
    }
};

TEST_SUITE(memory_resource_test, "boost.json.memory_resource");

BOOST_JSON_NS_END
