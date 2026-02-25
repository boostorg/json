//
// Copyright (c) 2026 Roy Bellingan (tsmtgdi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/value.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include "test_suite.hpp"
#include <iostream>

namespace boost {
namespace json {

class erase_at_pointer_test
{
    value
    testValueArray() const
    {
        auto raw  = R"(
[
    {
        "image" : {
            "path" : "somewhere",
            "size" : 9100
        },
        "comment" : [
            {
                "text" : "this is cool",
                "timestamp" : 123456,
                "likes" : [
                    {
                        "author" : "Coco",
                        "timestamp" : 123
                    },
                    {
                        "author" : "Izzy",
                        "timestamp" : 456
                    }
                ]
            }
        ]
    }
]

)";
        return boost::json::parse(raw);
    }
    
    value
    testValueObject() const
    {
        auto raw  = R"(
{
    "comment" : {
        "text" : "this is cool",
        "timestamp" : 123456,
        "likes" : [
            {
                "author" : "Coco",
                "timestamp" : 123
            },
            {
                "author" : "Izzy",
                "timestamp" : 456
            }
        ]
    }
}


)";
        return boost::json::parse(raw);
    }
    
    
public:
    void testObject1()
    {
        value json = testValueObject();
        value target = boost::json::parse(R"(
{
    "comment" : {
        "text" : "this is cool",
        "timestamp" : 123456
    }
}
)");
        boost::system::error_code ec;
        bool res = json.erase_at_pointer("/comment/likes", ec);
        BOOST_TEST(res);
        BOOST_TEST(!ec);
        BOOST_TEST(target == json);
    }
    
    void
    testArray1(){
        value json = testValueArray();
        value target = boost::json::parse(R"(
[
    {
        "image" : {
            "path" : "somewhere",
            "size" : 9100
        }
    }
]
)");
         boost::system::error_code ec;
         bool res = json.erase_at_pointer("/0/comment", ec);
         BOOST_TEST(res);
         BOOST_TEST(!ec);
         BOOST_TEST(target == json);
    }
    
    void
    testArray2(){
        value json = testValueArray();
        value target = boost::json::parse(R"(
[
    {
        "image" : {
            "path" : "somewhere",
            "size" : 9100
        },
        "comment" : [
            {
                "text" : "this is cool",
                "timestamp" : 123456,
                "likes" : [
                    {
                        "author" : "Coco",
                        "timestamp" : 123
                    }
                ]
            }
        ]
    }
]
)");
        boost::system::error_code ec;
        bool res = json.erase_at_pointer("/0/comment/0/likes/1", ec);
        BOOST_TEST(res);
        BOOST_TEST(!ec);
        BOOST_TEST(target == json);
        
    }
    
    
    void
    malformedPointer()
    {
        value const original = testValueArray();
        value copy = original;
        
        boost::system::error_code ec;
        bool res = copy.erase_at_pointer("invalid", ec);
        //it should fail
        BOOST_TEST(!res);
        
        BOOST_TEST(ec == error::missing_slash);
        //and the json should not have any change
        BOOST_TEST(copy == original);
    }
    
    void
    testEmptyPointer()
    {
        value const original = testValueArray();
        value copy = original;
        
        boost::system::error_code ec;
        bool res = copy.erase_at_pointer("", ec);
        //it should fail
        BOOST_TEST(!res);
        
        BOOST_TEST(ec);
        //and the json should not have any change
        BOOST_TEST(copy == original);
    }
    
    void inexistent1()
    {
        value const original = testValueArray();
        value copy = original;
        boost::system::error_code ec;
        bool res = copy.erase_at_pointer("/1/image", ec);
        
        //no deletition
        BOOST_TEST(!res);
        
        //and the json should not have any change
        BOOST_TEST(copy == original);
    }
    
    void
    inexistent2()
    {
        value const original = testValueObject();
        value copy = original;
        boost::system::error_code ec;
        bool res = copy.erase_at_pointer("/something/inexistent", ec);
        
        //no deletition
        BOOST_TEST(!res);

        //and the json should not have any change
        BOOST_TEST(copy == original);
    }
    
    void
    doubleDelete()
    {
        value json = testValueArray();
        boost::system::error_code ec;
        bool res = json.erase_at_pointer("/0/comment/0/text", ec);
        BOOST_TEST(res);
        
        auto copy = json;
        
        //already deleted
        res = json.erase_at_pointer("/0/comment/0/text", ec);
        BOOST_TEST(!res);
        
        BOOST_TEST(copy == json);
    }
    
    void
    chained()
    {
        value json = testValueArray();
        
        value target = boost::json::parse(R"(
[
    {
        "image": {},
        "comment": [
            {
                "likes": [
                    {},
                    {}
                ]
            }
        ]
    }
]
)");
        
        boost::system::error_code ec;
        bool res = json.erase_at_pointer("/0/comment/0/text", ec);
        BOOST_TEST(res);
        
        res = json.erase_at_pointer("/0/comment/0/timestamp", ec);
        BOOST_TEST(res);
        
        res = json.erase_at_pointer("/0/comment/0/likes/0/author", ec);
        BOOST_TEST(res);
        
        res = json.erase_at_pointer("/0/comment/0/likes/0/timestamp", ec);
        BOOST_TEST(res);
        
        //the previous element 0 still exists but is empty
        res = json.erase_at_pointer("/0/comment/0/likes/0/author", ec);
        BOOST_TEST(!res);
        
        res = json.erase_at_pointer("/0/comment/0/likes/1/author", ec);
        BOOST_TEST(res);
        
        res = json.erase_at_pointer("/0/comment/0/likes/1/timestamp", ec);
        BOOST_TEST(res);
        
        res = json.erase_at_pointer("/0/image/path", ec);
        BOOST_TEST(res);
        
        res = json.erase_at_pointer("/0/image/size", ec);
        BOOST_TEST(res);
        
        res = json.erase_at_pointer("/0/image/invalid", ec);
        BOOST_TEST(!res);
        
        BOOST_TEST(json == target);
    }
    
    void
    run()
    {
        testEmptyPointer();
        malformedPointer();
        testArray1();
        testArray2();
        testObject1();
        inexistent1();
        inexistent2();
        doubleDelete();
        chained();
    }
};


TEST_SUITE(erase_at_pointer_test, "boost.json.erase_at_pointer");

} // namespace json
} // namespace boost
