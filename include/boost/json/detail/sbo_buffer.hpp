//
// Copyright (c) 2023 Dmitry Arkhipov (grisumbras@yandex.ru)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//


#ifndef BOOST_JSON_DETAIL_SBO_BUFFER_HPP
#define BOOST_JSON_DETAIL_SBO_BUFFER_HPP

#include <boost/json/detail/config.hpp>
#include <string>
#include <array>

namespace boost {
namespace json {
namespace detail {

template< std::size_t N >
class sbo_buffer
{
    struct size_ptr_pair
    {
        std::size_t size;
        char* ptr;
    };
    BOOST_STATIC_ASSERT( N >= sizeof(size_ptr_pair) );

    union {
        std::array<char, N> buffer_;
        std::size_t capacity_;
    };
    char* data_ = buffer_.data();
    std::size_t size_ = 0;

    bool
    is_small() const noexcept
    {
        return data_ == buffer_.data();
    }

    void
    dispose()
    {
        if( is_small() )
            return;

        delete[] data_;
        buffer_ = {};
        data_ = buffer_.data();
    }

public:
    sbo_buffer()
        : buffer_()
    {}

    sbo_buffer(sbo_buffer&& other) noexcept
        : size_(other.size_)
    {
        if( other.is_small() )
        {
            buffer_ = other.buffer_;
            data_ = buffer_.data();
        }
        else
        {
            data_ = other.data_;
            other.data_ = other.buffer_.data();
        }
        BOOST_ASSERT( other.is_small() );
    }

    sbo_buffer&
    operator=(sbo_buffer&& other) noexcept
    {
        if( &other == this )
            return this;

        if( other.is_small() )
        {
            buffer_ = other.buffer_;
            data_ = buffer_.data();
        }
        else
        {
            data_ = other.data_;
            other.data_ = other.buffer_.data();
        }

        size_ = other.size_;
        other.size_ = 0;

        return *this;
    }

    ~sbo_buffer()
    {
        if( !is_small() )
            delete[] data_;
    }

    std::size_t
    capacity() const noexcept
    {
        return is_small() ? buffer_.size() : capacity_;
    }

    void
    reset() noexcept
    {
        dispose();
        clear();
    }

    void
    clear()
    {
        size_ = 0;
    }

    char*
    append(char const* ptr, std::size_t sz)
    {
        if( !sz )
            return data_;

        BOOST_ASSERT(ptr);
        std::size_t capacity = this->capacity();
        std::size_t size = size_;
        if( size + sz > capacity )
        {
            capacity = std::max(capacity + sz, capacity * 2);
            char* new_data = new char[capacity];
            std::memcpy(new_data, data_, size);

            dispose();
            data_ = new_data;
            capacity_ = capacity;
        }
        BOOST_ASSERT(data_);
        std::memcpy(data_ + size, ptr, sz);
        size_ = size + sz;

        return data_;
    }


    std::size_t
    size() noexcept
    {
        return size_;
    }
};

} // namespace detail
} // namespace json
} // namespace boost

#endif // BOOST_JSON_DETAIL_SBO_BUFFER_HPP
