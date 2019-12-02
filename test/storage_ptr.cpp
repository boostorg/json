//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/storage_ptr.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

#include "test.hpp"

namespace boost {
namespace json {

class storage_ptr_test : public beast::unit_test::suite
{
public:
    struct not_storage
    {
        void*
        allocate(
            std::size_t,
            std::size_t)
        {
            return nullptr;
        }

        void
        deallocate(
            void*,
            std::size_t,
            std::size_t) noexcept
        {
        }
    };

    BOOST_STATIC_ASSERT(
        ! is_storage<not_storage>::value);

    struct throwing
    {
        static
        constexpr
        std::uint64_t
        id = 0;

        static
        constexpr
        bool
        need_free = true;

        throwing()
        {
            throw std::exception{};
        }

        void*
        allocate(
            std::size_t,
            std::size_t)
        {
            return nullptr;
        }

        void
        deallocate(
            void*,
            std::size_t,
            std::size_t) noexcept
        {
        }
    };

    void
    testMembers()
    {
        auto const dsp = storage_ptr{};
        auto const usp =
            make_storage<unique_storage>();

        // ~storage_ptr()
        {
            // implied
        }

        // storage_ptr()
        {
            storage_ptr sp;
            BEAST_EXPECT(sp.get());
        }

        // storage_ptr(storage_ptr&&)
        {
            storage_ptr sp1 = dsp;
            storage_ptr sp2(std::move(sp1));
            BEAST_EXPECT(sp1.get());
            BEAST_EXPECT(*sp2 == *dsp);
        }

        // storage_ptr(storage_ptr const&)
        {
            storage_ptr sp1 = dsp;
            storage_ptr sp2(sp1);
            BEAST_EXPECT(sp1 == sp2);
        }

        // operator=(storage_ptr&&)
        {
            storage_ptr sp1(dsp);
            storage_ptr sp2(usp);
            sp2 = std::move(sp1);
            BEAST_EXPECT(*sp2 == *dsp);
        }

        // operator=(storage_ptr const&)
        {
            storage_ptr sp1(dsp);
            storage_ptr sp2(usp);
            sp2 = sp1;
            BEAST_EXPECT(*sp1 == *sp2);
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
    run() override
    {
        testMembers();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,storage_ptr);

} // json
} // boost
