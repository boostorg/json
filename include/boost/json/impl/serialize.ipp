//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZE_IPP
#define BOOST_JSON_IMPL_SERIALIZE_IPP

#include <boost/json/serialize.hpp>
#include <boost/json/serializer.hpp>
#include <boost/json/detail/serialize.hpp>
#include <ostream>

namespace boost {
namespace json {

std::string
serialize(
    value const& jv)
{
    unsigned char buf[256];
    serializer sr(
        storage_ptr(),
        buf,
        sizeof(buf));
    sr.reset(&jv);
    std::string s;
    detail::serialize_impl(s, sr);
    return s;
}

std::string
serialize(
    array const& arr)
{
    unsigned char buf[256];
    serializer sr(
        storage_ptr(),
        buf,
        sizeof(buf));
    std::string s;
    sr.reset(&arr);
    detail::serialize_impl(s, sr);
    return s;
}

std::string
serialize(
    object const& obj)
{
    unsigned char buf[256];
    serializer sr(
        storage_ptr(),
        buf,
        sizeof(buf));
    std::string s;
    sr.reset(&obj);
    detail::serialize_impl(s, sr);
    return s;
}

std::string
serialize(
    string const& str)
{
    unsigned char buf[256];
    serializer sr(
        storage_ptr(),
        buf,
        sizeof(buf));
    std::string s;
    sr.reset(&str);
    detail::serialize_impl(s, sr);
    return s;
}

// this is here for key_value_pair::key()
std::string
serialize(
    string_view sv)
{
    unsigned char buf[256];
    serializer sr(
        storage_ptr(),
        buf,
        sizeof(buf));
    std::string s;
    sr.reset(sv);
    detail::serialize_impl(s, sr);
    return s;
}

//----------------------------------------------------------

//[example_operator_lt__lt_
// Serialize a value into an output stream

std::ostream&
operator<<( std::ostream& os, value const& jv )
{
    // Create a serializer
    serializer sr;

    // Set the serializer up for our value
    sr.reset( &jv );

    // Loop until all output is produced.
    while( ! sr.done() )
    {
        // Use a local buffer to avoid allocation.
        char buf[ BOOST_JSON_STACK_BUFFER_SIZE ];

        // Fill our buffer with serialized characters and write it to the output stream.
        os << sr.read( buf );
    }

    return os;
}
//]

static
void
to_ostream(
    std::ostream& os,
    serializer& sr)
{
    while(! sr.done())
    {
        char buf[BOOST_JSON_STACK_BUFFER_SIZE];
        auto s = sr.read(buf);
        os.write(s.data(), s.size());
    }
}

std::ostream&
operator<<(
    std::ostream& os,
    array const& arr)
{
    serializer sr;
    sr.reset(&arr);
    to_ostream(os, sr);
    return os;
}

std::ostream&
operator<<(
    std::ostream& os,
    object const& obj)
{
    serializer sr;
    sr.reset(&obj);
    to_ostream(os, sr);
    return os;
}

std::ostream&
operator<<(
    std::ostream& os,
    string const& str)
{
    serializer sr;
    sr.reset(&str);
    to_ostream(os, sr);
    return os;
}

} // namespace json
} // namespace boost

#endif
