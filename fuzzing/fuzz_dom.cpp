//
// Copyright 2026 Arthur SC Chan (arthur.chan@adalogics.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//
// Fuzzer for the DOM mutation APIs: array/object mutators and value kind
// conversions, followed by serialisation and visitation.
//
// Input layout (NUL-separated fields):
//   <op bytes> NUL <json document>
// Each op consumes two bytes: [selector][argument]. All sizes and positions
// are reduced modulo small bounds so memory use stays bounded.
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

// small fixed keys so that ops collide predictably with parsed keys
string_view
key_for(uint8_t arg)
{
    static char const keys[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    return string_view(keys + (arg & 7), 1);
}

std::size_t volatile sink_dump;

void
apply_op(
    value& root,
    uint8_t sel,
    uint8_t arg,
    std::size_t& sink)
{
    array* arr = root.is_array() ? &root.get_array() : nullptr;
    object* obj = root.is_object() ? &root.get_object() : nullptr;
    switch(sel & 15)
    {
    case 0:
        if(arr)
            arr->emplace_back(std::int64_t(arg));
        break;
    case 1:
        if(arr)
        {
            if(arg & 1)
                arr->push_back(object{{"a", std::int64_t(arg)}});
            else
                arr->push_back(array{std::int64_t(arg)});
        }
        break;
    case 2:
        if(arr)
            arr->resize(arg % 8u);
        break;
    case 3:
        if(arr)
            arr->reserve(arg % 16u);
        break;
    case 4:
        if(arr && arr->size())
            arr->erase(arr->begin() + (arg % arr->size()));
        break;
    case 5:
        if(arr && arr->size())
            arr->insert(
                arr->begin() + (arg % arr->size()), double(arg));
        break;
    case 6:
        if(obj)
            obj->emplace(key_for(arg), std::int64_t(arg));
        break;
    case 7:
        if(obj)
            obj->insert_or_assign(key_for(arg), string_view("v", 1));
        break;
    case 8:
        if(obj)
            obj->erase(key_for(arg));
        break;
    case 9:
        if(obj)
            obj->reserve(arg % 12u);
        break;
    case 10:
        if(obj)
        {
            auto const it = obj->find(key_for(arg));
            if(it != obj->end())
                sink += serialize(it->value()).size();
        }
        break;
    case 11:
        // kind conversions on the root value
        switch(arg & 3)
        {
        case 0: root.emplace_null(); break;
        case 1: root.emplace_bool(); break;
        case 2: root.emplace_int64(); break;
        case 3: root.emplace_string(); break;
        }
        break;
    case 12:
    {
        value tmp = root;
        root.swap(tmp);
        sink += serialize(tmp).size();
        break;
    }
    case 13:
    {
        value cp(root);
        sink += serialize(cp).size();
        break;
    }
    case 14:
        visit([&sink](auto const&){ ++sink; }, root);
        break;
    case 15:
    {
        // promote the nested container so object and array mutators
        // are both reachable on the same input
        if(root.is_array())
        {
            object o;
            o.emplace("x", std::move(root));
            root = std::move(o);
        }
        else if(root.is_object())
        {
            array a;
            a.push_back(std::move(root));
            root = std::move(a);
        }
        break;
    }
    }
}

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
    string_view const jtext = field(d, s);

    boost::system::error_code ec;
    value root = parse(jtext, ec);
    if(! root.is_array() && ! root.is_object())
        root = array{std::move(root)};

    std::size_t sink = 0;
    try
    {
        std::size_t const nops =
            ops.size() / 2 < 32 ? ops.size() / 2 : 32;
        for(std::size_t i = 0; i < nops; ++i)
            apply_op(root, ops[2 * i], ops[2 * i + 1], sink);

        sink += serialize(root).size();
    }
    catch(...)
    {
    }
    sink_dump = sink;
    return 0;
}
