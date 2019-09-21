//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/json
//

// Test that header file is self-contained.
#include <boost/json/iterator.hpp>

#include <boost/beast/_experimental/unit_test/suite.hpp>

namespace boost {
namespace json {

class iterator_test : public beast::unit_test::suite
{
public:
    void
    testIterator()
    {
        value jv = {
          {"pi", 3.141},
          {"happy", true},
          {"name", "Niels"},
          {"nothing", nullptr},
          {"answer", {
            {"everything", 42}
          }},
#if 0
          {"list", {1, 0, 2}},
          {"object", {
            {"currency", "USD"},
            {"value", 42.99}
          }}
#endif
        };

        auto& arr = jv.as_object().
            emplace("arr", kind::array).first->second.as_array();
        arr.emplace_back(1);
        arr.emplace_back(2);
        arr.emplace_back(3);

        const_iterator it(jv);
        while(it != end)
        {
            auto const& e = *it;
            log << std::string(e.depth*4, ' ');
            if(! e.end)
            {
                if(! e.key.empty())
                    log << "\""  << e.key << "\" : ";
                switch(e.value.kind())
                {
                case kind::object:
                    log << "{";
                    break;
                case kind::array:
                    log << "[";
                    break;
                case kind::string:
                    log << '\"' << e.value.as_string() << "\"";
                    if(! e.last)
                        log << ",";
                    break;
                case kind::number:
                    log << e.value.as_number();
                    if(! e.last)
                        log << ",";
                    break;
                case kind::boolean:
                    log << (e.value.as_bool() ?
                        "true" : "false");
                    if(! e.last)
                        log << ",";
                    break;
                case kind::null:
                    log << "null";
                    if(! e.last)
                        log << ",";
                    break;
                }
                log.flush();
            }
            else if(e->value.is_object())
            {
                log << "}";
                if(! e.last)
                    log << ",";
            }
            else if(e->value.is_array())
            {
                log << "]";
                if(! e.last)
                    log << ",";
            }
            log << "\n";
            log.flush();
            ++it;
        }
    }

    void
    run() override
    {
        testIterator();
        pass();
    }
};

BEAST_DEFINE_TESTSUITE(beast,json,iterator);

} // json
} // boost
