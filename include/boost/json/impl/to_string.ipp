//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_TO_STRING_IPP
#define BOOST_JSON_IMPL_TO_STRING_IPP

#include <boost/json/to_string.hpp>
#include <boost/json/serializer.hpp>
#include <ostream>

BOOST_JSON_NS_BEGIN

string
to_string(
    json::value const& jv)
{
    string s;
    serializer sr(jv);
    while(! sr.done())
    {
        if(s.size() >= s.capacity())
            s.reserve(s.capacity() + 1);
        s.grow(static_cast<
            string::size_type>(
            sr.read(s.data() + s.size(),
                s.capacity() - s.size()).size()));
    }
    return s;
}

//[example_operator_lt__lt_
// Serialize a value into an output stream

std::ostream&
operator<<( std::ostream& os, value const& jv )
{
    // Create a serializer that is set to output our value.
    serializer sr( jv );

    // Loop until all output is produced.
    while( ! sr.done() )
    {
        // Use a local buffer.
        char buf[4000];

        // Try to fill up the local buffer.
        auto const bytes_written = sr.read( buf ).size();

        // Write the valid portion of the buffer to the output stream.
        os.write( buf, bytes_written );
    }

    return os;
}
//]

BOOST_JSON_NS_END

#endif
