//
// Copyright 2026 Arthur SC Chan (arthur.chan@adalogics.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//
// Fuzzer for the JSON Pointer API: value::find_pointer, value::try_at_pointer,
// value::set_at_pointer and value::try_set_at_pointer, including the
// set_pointer_options matrix.
//
// Input layout (NUL-separated fields):
//   <op bytes> NUL <json document> NUL <pointer string>
// A JSON document never contains a raw NUL byte, so the split is
// unambiguous. All growth through set_pointer_options is bounded
// (max_created_elements <= 8) so memory use stays bounded.
//

#include <boost/json.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>

using namespace boost::json;

namespace
{

string_view
field(
    uint8_t const*& data,
    std::size_t& size)
{
    uint8_t const* nul =
        static_cast<uint8_t const*>(std::memchr(data, 0, size));
    if(! nul)
    {
        string_view sv{
            reinterpret_cast<char const*>(data), size};
        data += size;
        size = 0;
        return sv;
    }
    string_view sv{
        reinterpret_cast<char const*>(data),
        static_cast<std::size_t>(nul - data)};
    size -= static_cast<std::size_t>(nul - data) + 1;
    data = nul + 1;
    return sv;
}

// sink is written through a volatile pointer so the optimiser cannot
// discard the serialisation work done on the side results.
std::size_t volatile sink_dump;

} // namespace

extern "C"
int
LLVMFuzzerTestOneInput(
    uint8_t const* data, std::size_t size)
{
    if(size < 4)
        return 0;

    uint8_t const opt_bits = data[0];
    uint8_t const val_bits = data[1];
    uint8_t const op_sel   = data[2];

    data += 3;
    size -= 3;

    string_view const jtext = field(data, size);
    string_view const ptr   = field(data, size);

    set_pointer_options opts;
    opts.create_arrays        = !! (opt_bits & 1);
    opts.create_objects       = !! (opt_bits & 2);
    opts.replace_any_scalar   = !! (opt_bits & 4);
    opts.max_created_elements = 1u + (opt_bits >> 5);

    boost::system::error_code ec;
    value jv = parse(jtext, ec);

    std::size_t sink = 0;
    try
    {
        // navigation only
        value const* found = jv.find_pointer(ptr, ec);
        if(found)
            sink += serialize(*found).size();

        // read access through the result-based API
        if(auto r = jv.try_at_pointer(ptr))
            sink += serialize(*r).size();

        // the replacement value, kept small and fuzz-selected
        value new_val;
        switch(val_bits & 7)
        {
        case 0: new_val = nullptr; break;
        case 1: new_val = std::int64_t(-42 - val_bits); break;
        case 2: new_val = std::uint64_t(val_bits); break;
        case 3: new_val = double(val_bits) / 8.0; break;
        case 4: new_val = string_view("fz", 2); break;
        case 5: new_val = array{nullptr, std::int64_t(1)}; break;
        case 6: new_val = object{{"k", nullptr}}; break;
        default: new_val = !! (val_bits & 8); break;
        }

        if(op_sel & 1)
        {
            value* set = jv.set_at_pointer(ptr, new_val, ec, opts);
            if(set)
                sink += serialize(*set).size();
        }
        if(op_sel & 2)
        {
            if(auto r = jv.try_set_at_pointer(
                ptr, !! (val_bits & 16), opts))
                sink += serialize(*r).size();
        }
        if(op_sel & 4)
        {
            // creation of intermediate containers out of scalars
            if(auto r = jv.try_set_at_pointer(
                ptr, string_view("p", 1), opts))
                sink += serialize(*r).size();
        }

        // walk the modified document again
        found = jv.find_pointer(ptr, ec);
        if(found)
            sink += serialize(*found).size();

        // whole-document serialisation exercises rehash/realloc paths
        sink += serialize(jv).size();
    }
    catch(...)
    {
    }
    sink_dump = sink;
    return 0;
}
