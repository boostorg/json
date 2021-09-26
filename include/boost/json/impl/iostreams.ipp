//
// Copyright (c) 2021 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_IOSTREAMS_IPP
#define BOOST_JSON_IMPL_IOSTREAMS_IPP

#include <boost/json/iostreams.hpp>

BOOST_JSON_NS_BEGIN

streambuf::streambuf(
    parse_options const& opt,
    storage_ptr sp,
    char* buffer,
    std::size_t buffer_size)
    : p_(
        sp,
        opt,
        reinterpret_cast<unsigned char*>(buffer),
        buffer_size - (std::min)(std::size_t(1024), buffer_size / 4))
{
    std::size_t streambuf_size = (std::min)(
        std::size_t(1024), buffer_size / 4);
    if( streambuf_size )
    {
        buffer += buffer_size - streambuf_size;
    }
    else
    {
        mem_res_ = sp.get();
        buffer = static_cast<char*>(
            sp->allocate(1024, alignof(char)));
    }
    setp(buffer, buffer + streambuf_size);
}

streambuf::~streambuf() noexcept
{
    if( mem_res_ )
        mem_res_->deallocate(pbase(), 1024, alignof(char));
}

void
streambuf::reset_parser(storage_ptr sp) noexcept
{
    p_.reset(std::move(sp));
}

json::value
streambuf::value(error_code& ec)
{
    if( !ec_ )
    {
        overflow();
        p_.finish(ec_);
    }

    ec = std::move(ec_);
    return ec
        ? nullptr
        : p_.release();
}

json::value
streambuf::value()
{
    error_code ec;
    auto jv = value(ec);
    if( ec )
        detail::throw_system_error(
            ec,
            BOOST_JSON_SOURCE_POS);
    return jv;
}

streambuf::int_type
streambuf::overflow(int_type ch)
{
    p_.write(pbase(), pptr() - pbase(), ec_);
    setp(pbase(), epptr());

    if( traits_type::not_eof(ch) )
    {
        *pptr() = traits_type::to_char_type(ch);
        pbump(1);
    }

    return ec_
        ? traits_type::eof()
        : '0'; // not EOF
}

std::streamsize
streambuf::xsputn(
    char const* s,
    std::streamsize count)
{
    if( ec_ )
        return 0;

    std::streamsize available = pptr() - epptr();
    if( available > count )
    {
        std::memcpy(pbase(), s, count);
        pbump(count);
        return count;
    }

    if( pbase() != pptr() )
    {
        overflow();
        if( ec_ )
            return 0;
    }

    return p_.write(s, count, ec_);
}

value
parse(
    std::istream& is,
    error_code& ec,
    storage_ptr sp,
    parse_options const& opt)
{
    if( !is )
    {
        ec = std::make_error_code(std::io_errc::stream);
        return nullptr;
    }

    char temp[
        BOOST_JSON_STACK_BUFFER_SIZE];
    streambuf p(opt, std::move(sp), temp, BOOST_JSON_STACK_BUFFER_SIZE);

    is >> &p;
    if( !is )
    {
        ec = std::make_error_code(std::io_errc::stream);
        return nullptr;
    }
    return p.value(ec);
}

value
parse(
    std::streambuf* sb,
    error_code& ec,
    storage_ptr sp,
    parse_options const& opt)
{
    std::istream is(sb);
    return parse(is, ec, sp, opt);
}

value
parse(
    std::istream& is,
    storage_ptr sp,
    parse_options const& opt)
{
    error_code ec;
    auto jv = parse(
        is, ec, std::move(sp), opt);
    if( ec )
        detail::throw_system_error(
            ec,
            BOOST_JSON_SOURCE_POS);
    return jv;
}

BOOST_JSON_NS_END

#endif // BOOST_JSON_IMPL_IOSTREAMS_IPP
