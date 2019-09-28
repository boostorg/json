//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/storage.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>
#include <boost/json/detail/storage_adaptor.hpp>

#include "test_storage.hpp"

namespace boost {
namespace json {

class storage_test : public beast::unit_test::suite
{
public:
    void
    testStoragePtr()
    {
        auto p = new detail::storage_adaptor<
            std::allocator<char>>({});

        // default ctor
        {
            storage_ptr s;
            BEAST_EXPECT(s == nullptr);
        }

        // move ctor
        {
            p->addref();
            storage_ptr s1(p);
            storage_ptr s2(std::move(s1));
            BEAST_EXPECT(s1 == nullptr);
            BEAST_EXPECT(s2 == p);
        }

        // copy ctor
        {
            p->addref();
            storage_ptr s1(p);
            storage_ptr s2(s1);
            BEAST_EXPECT(s1 == p);
            BEAST_EXPECT(s2 == p);
            BEAST_EXPECT(s1 == s2);
        }

        // move assign
        {
            p->addref();
            storage_ptr s1(p);
            storage_ptr s2;
            s2 = std::move(s1);
            BEAST_EXPECT(s1 == nullptr);
            BEAST_EXPECT(s2 == p);
        }

        // copy assign
        {
            p->addref();
            storage_ptr s1(p);
            storage_ptr s2;
            s2 = s1;
            BEAST_EXPECT(s1 == p);
            BEAST_EXPECT(s2 == p);
            BEAST_EXPECT(s1 == s2);
        }

        // construction
        {
            p->addref();
            storage_ptr sp(p);
            BEAST_EXPECT(sp == p);
        }

        // release
        {
            p->addref();
            storage_ptr sp(
                storage_ptr{p}.release());
            BEAST_EXPECT(sp == p);
        }

        // bool, get, ->, *
        {
            p->addref();
            storage_ptr sp(p);
            BEAST_EXPECT(sp);
            BEAST_EXPECT(sp.get() == p);
            BEAST_EXPECT(sp.operator->() == p);
            BEAST_EXPECT(&sp.operator*() == p);
            sp = nullptr;
            BEAST_EXPECT(! sp);
        }

        // converting assignment
        {
            p->addref();
            storage_ptr sp;
            sp = p;
            BEAST_EXPECT(sp == p);
            sp = nullptr;
            BEAST_EXPECT(! sp);
        }

        // equality
        {
            unique_storage us1;
            unique_storage us2;
            storage_ptr spu1(&us1);
            storage_ptr spu2(&us2);
            storage_ptr sp = make_storage_ptr(
                std::allocator<void>{});
            storage_ptr spd =
                default_storage();
            BEAST_EXPECT(us1 != us2);
            BEAST_EXPECT(us1 != *spd);
            BEAST_EXPECT(us2 != *spd);
            BEAST_EXPECT(us1 != *sp);
            BEAST_EXPECT(us2 != *sp);
            BEAST_EXPECT(
                *spd == *default_storage());
            BEAST_EXPECT(
                *sp != *default_storage());
        }

        p->release();
    }

    void run() override
    {
        testStoragePtr();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,storage);

} // json
} // boost
