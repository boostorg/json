//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_SERIALIZE_IPP
#define BOOST_JSON_IMPL_SERIALIZE_IPP

#include <boost/json/serialize.hpp>
#include <boost/json/serializer.hpp>
#include <ostream>

BOOST_JSON_NS_BEGIN

static
void
serialize_impl(
    string& s,
    serializer& sr)
{
    // serialize to a small buffer
    // to avoid most reallocations
    char buf[16384];
    string_view sv;
    sv = sr.read(buf);
    if(sr.done())
    {
        // fast path
        s.append(sv);
        return;
    }
    s.reserve(sv.size() * 2);
    s.append(sv);
    do
    {
        sv = sr.read(
            s.data() + s.size(),
            s.capacity() - s.size());
        s.grow(sv.size());
    }
    while(! sr.done());
}

string
serialize(
    value const& jv,
    storage_ptr sp)
{
    string s(std::move(sp));
    serializer sr;
    sr.reset(&jv);
    serialize_impl(s, sr);
    return s;
}

string
serialize(
    array const& arr,
    storage_ptr sp)
{
    string s(std::move(sp));
    serializer sr;
    sr.reset(&arr);
    serialize_impl(s, sr);
    return s;
}

string
serialize(
    object const& obj,
    storage_ptr sp)
{
    string s(std::move(sp));
    serializer sr;
    sr.reset(&obj);
    serialize_impl(s, sr);
    return s;
}

string
serialize(
    string const& str,
    storage_ptr sp)
{
    string s(std::move(sp));
    serializer sr;
    sr.reset(&str);
    serialize_impl(s, sr);
    return s;
}

string
serialize(
    string_view sv,
    storage_ptr sp)
{
    string s(std::move(sp));
    serializer sr;
    sr.reset(sv);
    serialize_impl(s, sr);
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
        char buf[ 4000 ];

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
    char buf[16384];
    while(! sr.done())
    {
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

BOOST_JSON_NS_END

#endif
