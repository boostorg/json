//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

#ifndef BOOST_JSON_EXAMPLE_FILE_HPP
#define BOOST_JSON_EXAMPLE_FILE_HPP

#include <boost/json/config.hpp>
#include <cstdio>

class file
{
    FILE* f_ = nullptr;

public:
    ~file()
    {
        if(f_)
            std::fclose(f_);
    }

    file() = default;

    file( file&& other ) noexcept
        : f_(other.f_)
    {
        other.f_ = nullptr;
    }

    file( char const* path, char const* mode )
    {
        open( path, mode );
    }

    file&
    operator=(file&& other) noexcept
    {
        close();
        f_ = other.f_;
        other.f_ = nullptr;
        return *this;
    }

    void
    close()
    {
        if(f_)
        {
            std::fclose(f_);
            f_ = nullptr;
        }
    }

    void
    open(
        char const* path,
        char const* mode,
        boost::json::error_code& ec)
    {
        close();
        f_ = std::fopen( path, mode );
        if( ! f_ )
        {
            ec.assign( errno, boost::json::generic_category() );
            return;
        }
    }

    void
    open(
        char const* path,
        char const* mode)
    {
        boost::json::error_code ec;
        open(path, mode, ec);
        if(ec)
            throw boost::json::system_error(ec);
    }

    bool
    eof() const noexcept
    {
        return std::feof( f_ ) != 0;
    }

    std::size_t
    read( char* data, std::size_t size, boost::json::error_code& ec)
    {
        auto const nread = std::fread( data, 1, size, f_ );
        if( std::ferror(f_) )
            ec.assign( errno, boost::json::generic_category() );
        return nread;
    }

    std::size_t
    read( char* data, std::size_t size )
    {
        boost::json::error_code ec;
        auto const nread = read( data, size, ec );
        if(ec)
            throw boost::json::system_error(ec);
        return nread;
    }
};

#endif
