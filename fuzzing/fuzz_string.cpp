//
// Copyright 2026 Arthur SC Chan (arthur.chan@adalogics.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//
// Fuzzer for the string API: assignment, growth, insertion, replacement and
// erasure across the small-string/boundary transitions, plus string_view
// interoperability and string-to-value conversion.
//
// Input layout (NUL-separated fields):
//   <op bytes> NUL <payload bytes>
// Each op consumes two bytes: [selector][argument]. All lengths are reduced
// modulo small bounds so memory use stays bounded.
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

// a slice of the payload, bounded in offset and length by the fuzz input
string_view
slice(string_view payload, uint8_t arg)
{
    std::size_t pos = arg & 15;
    if(pos > payload.size())
        pos = payload.size();
    std::size_t len = 1u + ((arg >> 4) & 15);
    if(len > payload.size() - pos)
        len = payload.size() - pos;
    return payload.substr(pos, len);
}

std::size_t volatile sink_dump;

} // namespace

extern "C"
int
LLVMFuzzerTestOneInput(
    uint8_t const* data, std::size_t size)
{
    if(size < 2)
        return 0;

    uint8_t const* d = data;
    std::size_t s = size;

    string_view const ops = field(d, s);
    string_view const payload = field(d, s);

    std::size_t sink = 0;
    try
    {
        string str;
        str.assign(payload);

        std::size_t const nops =
            ops.size() / 2 < 24 ? ops.size() / 2 : 24;
        for(std::size_t i = 0; i < nops; ++i)
        {
            uint8_t const sel = ops[2 * i];
            uint8_t const arg = ops[2 * i + 1];
            std::size_t const len = str.size();
            switch(sel & 15)
            {
            case 0: str.assign(slice(payload, arg)); break;
            case 1: str.append(slice(payload, arg)); break;
            case 2:
                str.insert(len ? arg % len : 0, slice(payload, arg));
                break;
            case 3:
                if(len)
                    str.erase(arg % len, arg & 7u);
                break;
            case 4: str.resize(arg % 64u); break;
            case 5: str.reserve(arg % 128u); break;
            case 6: str.shrink_to_fit(); break;
            case 7: str.assign(arg & 7u, 'x'); break;
            case 8: str.append(arg & 7u, 'y'); break;
            case 9:
                str.replace(
                    len ? arg % len : 0, arg & 7u, slice(payload, arg));
                break;
            case 10:
            {
                string_view sv{str.data(), str.size()};
                if(sv.size())
                {
                    string_view const sub =
                        sv.substr(arg % sv.size(), arg & 31u);
                    sink += sub.size();
                    sink += static_cast<std::size_t>(
                        str.compare(sub) != 0);
                }
                break;
            }
            case 11:
            {
                string_view sv{str.data(), str.size()};
                sink += str.starts_with(sv) + str.ends_with(sv);
                break;
            }
            case 12:
            {
                value jv{str};
                sink += serialize(jv).size();
                break;
            }
            case 13:
            {
                string other;
                str.swap(other);
                str.swap(other);
                break;
            }
            case 14:
            {
                string moved = std::move(str);
                str = std::move(moved);
                break;
            }
            case 15:
            {
                string cp;
                cp = str;
                sink += cp.size();
                break;
            }
            }
        }
        sink += str.size();
    }
    catch(...)
    {
    }
    sink_dump = sink;
    return 0;
}
