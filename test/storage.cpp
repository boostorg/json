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

#include "test.hpp"

namespace boost {
namespace json {

class storage_test : public beast::unit_test::suite
{
public:
    struct throwing : storage
    {
        throwing()
        {
            throw std::exception{};
        }

        void*
        do_allocate(
            std::size_t,
            std::size_t) override
        {
            return nullptr;
        }

        void
        do_deallocate(
            void*,
            std::size_t,
            std::size_t) noexcept override
        {
        }

        bool
        do_is_equal(
            storage const&)
            const noexcept override
        {
            return true;
        }
    };

    void
    testMembers()
    {
        auto const dsp = default_storage();
        auto const usp =
            make_storage<unique_storage>();

        // ~storage_ptr()
        {
            // implied
        }

        // storage_ptr()
        {
            storage_ptr sp;
            BEAST_EXPECT(! sp);
        }

        // storage_ptr(storage_ptr&&)
        {
            storage_ptr sp1 = dsp;
            storage_ptr sp2(std::move(sp1));
            BEAST_EXPECT(! sp1);
            BEAST_EXPECT(*sp2 == *dsp);
        }

        // storage_ptr(storage_ptr const&)
        {
            storage_ptr sp1 = dsp;
            storage_ptr sp2(sp1);
            BEAST_EXPECT(sp1);
            BEAST_EXPECT(sp2);
            BEAST_EXPECT(sp1 == sp2);
        }

        // storage_ptr(basic_storage_ptr<U>&&)
        {
            basic_storage_ptr<unique_storage> sp1 =
                make_storage<unique_storage>();
            storage_ptr sp2(std::move(sp1));
            BEAST_EXPECT(! sp1);
            BEAST_EXPECT(sp2);
        }

        // storage_ptr(basic_storage_ptr<U> const&)
        {
            basic_storage_ptr<unique_storage> sp1 =
                make_storage<unique_storage>();
            storage_ptr sp2(sp1);
            BEAST_EXPECT(sp1);
            BEAST_EXPECT(sp2);
            BEAST_EXPECT(*sp1 == *sp2);
        }

        // storage_ptr(nullptr_t)
        {
            storage_ptr sp(nullptr);
            BEAST_EXPECT(! sp);
        }

        // operator=(storage_ptr&&)
        {
            storage_ptr sp1(dsp);
            storage_ptr sp2(usp);
            sp2 = std::move(sp1);
            BEAST_EXPECT(! sp1);
            BEAST_EXPECT(*sp2 == *dsp);
        }

        // operator=(storage_ptr const&)
        {
            storage_ptr sp1(dsp);
            storage_ptr sp2(usp);
            sp2 = sp1;
            BEAST_EXPECT(sp1);
            BEAST_EXPECT(*sp1 == *sp2);
        }

        // operator bool()
        {
            storage_ptr sp;
            BEAST_EXPECT(! sp);
            sp = dsp;
            BEAST_EXPECT(sp);
        }

        // get()
        {
            storage_ptr sp(dsp);
            BEAST_EXPECT(sp.get() == dsp.get());
        }

        // operator->()
        {
            storage_ptr sp(dsp);
            BEAST_EXPECT(sp.operator->() == dsp.get());
        }

        // operator*()
        {
            storage_ptr sp(dsp);
            BEAST_EXPECT(&sp.operator*() == dsp.get());
        }

        // exception in make_storage
        {
            BEAST_THROWS(
                make_storage<throwing>(),
                std::exception);
        }
    }

    void
    testRelational()
    {
        basic_storage_ptr<unique_storage> sp1 =
            make_storage<unique_storage>();
        storage_ptr sp2 = sp1;
        basic_storage_ptr<unique_storage> sp3 =
            make_storage<unique_storage>();
        storage_ptr sp4;
        BEAST_EXPECT(sp1 == sp2);
        BEAST_EXPECT(sp1 != sp3);
        BEAST_EXPECT(sp4 == nullptr);
        BEAST_EXPECT(sp3 != nullptr);
        BEAST_EXPECT(nullptr == sp4);
        BEAST_EXPECT(nullptr != sp3);
    }

    void
    testDefaultStorage()
    {
        // default_storage()
        {
            auto sp1 = default_storage();
            auto sp2 = default_storage();
            BEAST_EXPECT(*sp1 == *sp2);
        }
    }

    void
    run() override
    {
        testMembers();
        testRelational();
        testDefaultStorage();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,storage);

} // json
} // boost
