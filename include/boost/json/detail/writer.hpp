//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2020 Krystian Stasiowski (sdkrystian@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_DETAIL_WRITER_HPP
#define BOOST_JSON_DETAIL_WRITER_HPP

#include <boost/json/string_view.hpp>
#include <boost/json/detail/stack.hpp>

namespace boost {
namespace json {
namespace detail {

struct writer
{
    char* dest_;
    char const* end_;

public:
    using resume_fn =
        bool(*)(writer&);

    detail::stack stack;
    char temp_[29];

    writer() = default;

    writer(
        storage_ptr sp,
        unsigned char* buf,
        std::size_t buf_size) noexcept
        : stack(
            std::move(sp),
            buf,
            buf_size)
    {
    }

    char*
    data() noexcept
    {
        return dest_;
    }

    // provide an output buffer
    void
    prepare(
        char* dest,
        std::size_t size)
    {
        dest_ = dest;
        end_ = dest + size;
    }

    // return true if there is no space
    bool
    empty() const noexcept
    {
        return dest_ == end_;
    }

    // return the amount of space available
    std::size_t
    available() const noexcept
    {
        return static_cast<
            std::size_t>(end_ - dest_);
    }

    // return true if there is space
    bool
    append(char c) noexcept
    {
        if(! empty())
        {
            *dest_++ = c;
            return true;
        }
        return false;
    }

    // append one char
    void
    append_unsafe(
        char c) noexcept
    {
        BOOST_ASSERT(! empty());
        *dest_++ = c;
    }

    // append chars in s, unchecked
    void
    append_unsafe(
        char const* s,
        std::size_t n) noexcept
    {
        BOOST_ASSERT(available() >= n);
        std::memcpy(dest_, s, n);
        dest_ += n;
    }

    // push a resume function
    void
    push_resume(resume_fn fn)
    {
        stack.push(fn);
    }

    // pop and invoke a resume function
    // or return true if stack is empty
    bool
    do_resume()
    {
        if(! stack.empty())
        {
            resume_fn fn;
            stack.pop(fn);
            return fn(*this);
        }
        return true;
    }
};

} // detail
} // json
} // boost

#endif
