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
#include <ostream>

BOOST_JSON_NS_BEGIN

static
void
serialize_impl(
    std::string& s,
    serializer& sr,
    const serialize_options & opt)
{
    // serialize to a small buffer to avoid
    // the first few allocations in std::string
    char buf[BOOST_JSON_STACK_BUFFER_SIZE];
    string_view sv;
    sv = sr.read(buf, opt);
    if(sr.done())
    {
        // fast path
        s.append(
            sv.data(), sv.size());
        return;
    }
    std::size_t len = sv.size();
    s.reserve(len * 2);
    s.resize(s.capacity());
    BOOST_ASSERT(
        s.size() >= len * 2);
    std::memcpy(&s[0],
        sv.data(), sv.size());
    for(;;)
    {
        sv = sr.read(
            &s[0] + len,
            s.size() - len, 
            opt);
        len += sv.size();
        if(sr.done())
            break;
        s.reserve(
            s.capacity() + 1);
        s.resize(
            s.capacity());
    }
    s.resize(len);
}

std::string
serialize(
    value const& jv,
    const serialize_options & opt)
{
    std::string s;
    serializer sr;
    sr.reset(&jv);
    serialize_impl(s, sr, opt);
    return s;
}

std::string
serialize(
    array const& arr,
    const serialize_options & opt)
{
    std::string s;
    serializer sr;
    sr.reset(&arr);
    serialize_impl(s, sr, opt);
    return s;
}

std::string
serialize(
    object const& obj,
    const serialize_options & opt)
{
    std::string s;
    serializer sr;
    sr.reset(&obj);
    serialize_impl(s, sr, opt);
    return s;
}

std::string
serialize(
    string const& str,
    const serialize_options & opt)
{
    std::string s;
    serializer sr;
    sr.reset(&str);
    serialize_impl(s, sr, opt);
    return s;
}

// this is here for key_value_pair::key()
std::string
serialize(
    string_view sv,
    const serialize_options & opt)
{
    std::string s;
    serializer sr;
    sr.reset(sv);
    serialize_impl(s, sr, opt);
    return s;
}

//----------------------------------------------------------


static
void
serialize_impl(
    std::string& s,
    serializer& sr,
    error_code & ec)
{
  // serialize to a small buffer to avoid
  // the first few allocations in std::string
  char buf[BOOST_JSON_STACK_BUFFER_SIZE];
  string_view sv;
  sv = sr.read(buf, ec);
  if(sr.done())
  {
    // fast path
    s.append(
        sv.data(), sv.size());
    return;
  }
  std::size_t len = sv.size();
  s.reserve(len * 2);
  s.resize(s.capacity());
      BOOST_ASSERT(
      s.size() >= len * 2);
  std::memcpy(&s[0],
              sv.data(), sv.size());
  for(;;)
  {
    sv = sr.read(
        &s[0] + len,
        s.size() - len,
        ec);
    len += sv.size();
    if(sr.done())
      break;
    s.reserve(
        s.capacity() + 1);
    s.resize(
        s.capacity());
  }
  s.resize(len);
}

std::string
serialize(
    value const& jv,
    error_code & ec)
{
  std::string s;
  serializer sr;
  sr.reset(&jv);
  serialize_impl(s, sr, ec);
  return s;
}

std::string
serialize(
    array const& arr,
    error_code & ec)
{
  std::string s;
  serializer sr;
  sr.reset(&arr);
  serialize_impl(s, sr, ec);
  return s;
}

std::string
serialize(
    object const& obj,
    error_code & ec)
{
  std::string s;
  serializer sr;
  sr.reset(&obj);
  serialize_impl(s, sr, ec);
  return s;
}

std::string
serialize(
    string const& str,
    error_code & ec)
{
  std::string s;
  serializer sr;
  sr.reset(&str);
  serialize_impl(s, sr, ec);
  return s;
}

// this is here for key_value_pair::key()
std::string
serialize(
    string_view sv,
    error_code & ec)
{
  std::string s;
  serializer sr;
  sr.reset(sv);
  serialize_impl(s, sr, ec);
  return s;
}


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

BOOST_JSON_NS_END

#endif
