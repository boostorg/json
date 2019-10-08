//
// Copyright (c) 2018-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/BeastLounge
//

// Test that header file is self-contained.
#include <boost/json/string.hpp>

#include <boost/config.hpp>
#include <boost/beast/_experimental/unit_test/suite.hpp>

#include "test_storage.hpp"

namespace boost {
namespace json {

class string_test : public beast::unit_test::suite
{
public:
    void
    testCtors()
    {
        // string()
        {
            scoped_fail_storage fs;
            string s;
        }

        // string(storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {  
            string s(sp);
            BEAST_EXPECT(s.empty());
            BEAST_EXPECT(
               *s.get_storage() == *sp);
        });

        // string(size_type, char, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s(3, '*', sp);
            BEAST_EXPECT(s.size() == 3);
            BEAST_EXPECT(s[0] == '*');
        });
        
        // string(string const&, size_type, size_type, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s1 = "abcde";
            string s2(s1, 1, 3, sp);
            BEAST_EXPECT(s2 == "bcd");
        });

        // string(char const*, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s("abc", sp);
            BEAST_EXPECT(s == "abc");
        });

        // string(char const*, size_type, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s("abcde", 3, sp);
            BEAST_EXPECT(s == "abc");
        });

        // string(InputIt, InputIt, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            auto init = { 'a', 'b', 'c' };
            string s(init.begin(), init.end(), sp);
            BEAST_EXPECT(s == "abc");
        });
        
        // string(string)
        fail_loop([]
        {
            string s1 = "abc";
            string s2(s1);
            BEAST_EXPECT(s2 == "abc");
        });

        // string(string, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s1 = "abc";
            string s2(s1, sp);
            BEAST_EXPECT(s2 == "abc");
        });

        // string(pilfered<string>)
        {
            string s1 = "abc";
            string s2(pilfer(s1));
            BEAST_EXPECT(s2 == "abc");
            BEAST_EXPECT(s1.empty());
            BEAST_EXPECT(
                s1.get_storage() == nullptr);
        }

        // string(string&&)
        {
            string s1 = "abc";
            scoped_fail_storage fs;
            string s2(std::move(s1));
            BEAST_EXPECT(s2 == "abc");
            BEAST_EXPECT(s1.empty());
            BEAST_EXPECT(
                *s1.get_storage() ==
                *s2.get_storage());
        }

        // string(string&&, storage_ptr)
        {
            // same storage
            fail_loop([](storage_ptr const& sp)
            {
                string s1("abc", sp);
                string s2(std::move(s1), sp);
                BEAST_EXPECT(s2 == "abc");
                BEAST_EXPECT(s1.empty());
                BEAST_EXPECT(
                    *s1.get_storage() ==
                    *s2.get_storage());
            });

            // different storage
            fail_loop([](storage_ptr const& sp)
            {
                string s1 = "abc";
                string s2(std::move(s1), sp);
                BEAST_EXPECT(s2 == "abc");
                BEAST_EXPECT(s1 == "abc");
                BEAST_EXPECT(
                    *s1.get_storage() !=
                    *s2.get_storage());
            });
        }

        // string(initializer_list, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s({'a', 'b', 'c'}, sp);
            BEAST_EXPECT(s == "abc");
        });

        // string(string_view, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s(string_view("abc"), sp);
            BEAST_EXPECT(s == "abc");
        });

        // string(string_view, size_type, size_type, storage_ptr)
        fail_loop([](storage_ptr const& sp)
        {
            string s(string_view("abcde"), 1, 3, sp);
            BEAST_EXPECT(s == "bcd");
        });
    }

    void
    testOpAssign()
    {
        // operator=(string)
        fail_loop([](storage_ptr const& sp)
        {
            string s1("abc");
            string s2(sp);
            s2 = s1;
            BEAST_EXPECT(s2 == "abc");
        });

        // operator=(string&&)
        {
            // same storage
            fail_loop([](storage_ptr const& sp)
            {
                string s1("abc", sp);
                string s2(sp);
                s2 = std::move(s1);
                BEAST_EXPECT(s2 == "abc");
                BEAST_EXPECT(s1.empty());
            });

            // different storage
            fail_loop([](storage_ptr const& sp)
            {
                string s1("abc");
                string s2(sp);
                s2 = std::move(s1);
                BEAST_EXPECT(s2 == "abc");
                BEAST_EXPECT(s1 == "abc");
            });
        }

        // operator=(char const*)
        fail_loop([](storage_ptr const& sp)
        {
            string s(sp);
            s = "abc";
            BEAST_EXPECT(s == "abc");
        });

        // operator=(char)
        fail_loop([](storage_ptr const& sp)
        {
            string s(sp);
            s = '*';
            BEAST_EXPECT(s == "*");
        });

        // operator=(std::initializer_list<char>)
        fail_loop([](storage_ptr const& sp)
        {
            string s(sp);
            s = {'a', 'b', 'c'};
            BEAST_EXPECT(s == "abc");
        });

        // operator=(string_view);
        fail_loop([](storage_ptr const& sp)
        {
            string s(sp);
            s = string_view("abc", 3);
            BEAST_EXPECT(s == "abc");
        });
    }

    void
    testAssign()
    {
        // assign(size_type, char)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.assign(3, '*');
            BEAST_EXPECT(s == "***");
        });

        // assign(string)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            string s2("abc");
            s.assign(s2);
            BEAST_EXPECT(s == "abc");
        });

        // assign(string, size_type, size_type)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            string s2("abcde");
            s.assign(s2, 1, 3);
            BEAST_EXPECT(s == "bcd");
        });

        // assign(string&&)
        {
            // same storage
            fail_loop([](storage_ptr const& sp)
            {
                string s("123", sp);
                string s2("abc", sp);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == "abc");
            });

            // different storage
            fail_loop([](storage_ptr const& sp)
            {
                string s("123", sp);
                string s2("abc");
                s.assign(std::move(s2));
                BEAST_EXPECT(s == "abc");
            });
        }

        // assign(char const*, size_type)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
        });

        // assign(char const* s)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.assign("abc");
            BEAST_EXPECT(s == "abc");
        });

        // assign(InputIt, InputIt)
        fail_loop([](storage_ptr const& sp)
        {
            std::initializer_list<
                char> init = { 'a', 'b', 'c' };
            string s("123", sp);
            s.assign(init.begin(), init.end());
            BEAST_EXPECT(s == "abc");
        });

        // assign(std::initializer_list<char>)
        fail_loop([](storage_ptr const& sp)
        {
            std::initializer_list<
                char> init = { 'a', 'b', 'c' };
            string s("123", sp);
            s.assign(init);
            BEAST_EXPECT(s == "abc");
        });

        // assign(string_view)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.assign(string_view("abc", 3));
            BEAST_EXPECT(s == "abc");
        });

        // assign(string_view, size_type, size_type);
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.assign(string_view("abcde"), 1, 3);
            BEAST_EXPECT(s == "bcd");
        });
    }

    void
    testAccess()
    {
        string s("abc");
        auto const& cs = s;

        // at(size_type)
        {
            BEAST_EXPECT(s.at(1) == 'b');
            s.at(1) = '*';
            BEAST_EXPECT(s.at(1) == '*');
            s.at(1) = 'b';
            BEAST_THROWS(s.at(3),
                std::out_of_range);
        }

        // at(size_type) const
        {
            BEAST_EXPECT(cs.at(1) == 'b');
            BEAST_THROWS(cs.at(3),
                std::out_of_range);
        }

        // operator[](size_type)
        {
            BEAST_EXPECT(s[1] == 'b');
            s[1] = '*';
            BEAST_EXPECT(s[1] == '*');
            s[1] = 'b';
        }

        // operator[](size_type) const
        {
            BEAST_EXPECT(cs[1] == 'b');
        }

        // front()
        {
            s.front() = '*';
            BEAST_EXPECT(s.front() == '*');
            s.front() = 'a';
        }

        // front() const
        {
            BEAST_EXPECT(cs.front() == 'a');
        }

        // back()
        {
            s.back() = '*';
            BEAST_EXPECT(s.back() == '*');
            s.back() = 'c';
        }

        // back() const
        {
            BEAST_EXPECT(cs.back() == 'c');
        }

        // data()
        {
            BEAST_EXPECT(string_view(
                s.data()) == "abc");
        }
        // data() const
        {
            BEAST_EXPECT(string_view(
                cs.data()) == "abc");
        }

        // c_str()
        {
            BEAST_EXPECT(string_view(
                s.c_str()) == "abc");
        }

        // operator string_view()
        {
            BEAST_EXPECT(
                string_view(s) == "abc");
        }
    }

    void
    testIterators()
    {
        string s = "abc";
        auto const& ac(s);

        {
            auto it = s.begin();
            BEAST_EXPECT(*it == 'a'); ++it;
            BEAST_EXPECT(*it == 'b');   it++;
            BEAST_EXPECT(*it == 'c'); ++it;
            BEAST_EXPECT(it == s.end());
        }
        {
            auto it = s.cbegin();
            BEAST_EXPECT(*it == 'a'); ++it;
            BEAST_EXPECT(*it == 'b');   it++;
            BEAST_EXPECT(*it == 'c'); ++it;
            BEAST_EXPECT(it == s.cend());
        }
        {
            auto it = ac.begin();
            BEAST_EXPECT(*it == 'a'); ++it;
            BEAST_EXPECT(*it == 'b');   it++;
            BEAST_EXPECT(*it == 'c'); ++it;
            BEAST_EXPECT(it == ac.end());
        }
        {
            auto it = s.end();
            --it; BEAST_EXPECT(*it == 'c');
            it--; BEAST_EXPECT(*it == 'b');
            --it; BEAST_EXPECT(*it == 'a');
            BEAST_EXPECT(it == s.begin());
        }
        {
            auto it = s.cend();
            --it; BEAST_EXPECT(*it == 'c');
            it--; BEAST_EXPECT(*it == 'b');
            --it; BEAST_EXPECT(*it == 'a');
            BEAST_EXPECT(it == s.cbegin());
        }
        {
            auto it = ac.end();
            --it; BEAST_EXPECT(*it == 'c');
            it--; BEAST_EXPECT(*it == 'b');
            --it; BEAST_EXPECT(*it == 'a');
            BEAST_EXPECT(it == ac.begin());
        }

        {
            auto it = s.rbegin();
            BEAST_EXPECT(*it == 'c'); ++it;
            BEAST_EXPECT(*it == 'b');   it++;
            BEAST_EXPECT(*it == 'a'); ++it;
            BEAST_EXPECT(it == s.rend());
        }
        {
            auto it = s.crbegin();
            BEAST_EXPECT(*it == 'c'); ++it;
            BEAST_EXPECT(*it == 'b');   it++;
            BEAST_EXPECT(*it == 'a'); ++it;
            BEAST_EXPECT(it == s.crend());
        }
        {
            auto it = ac.rbegin();
            BEAST_EXPECT(*it == 'c'); ++it;
            BEAST_EXPECT(*it == 'b');   it++;
            BEAST_EXPECT(*it == 'a'); ++it;
            BEAST_EXPECT(it == ac.rend());
        }
        {
            auto it = s.rend();
            --it; BEAST_EXPECT(*it == 'a');
            it--; BEAST_EXPECT(*it == 'b');
            --it; BEAST_EXPECT(*it == 'c');
            BEAST_EXPECT(it == s.rbegin());
        }
        {
            auto it = s.crend();
            --it; BEAST_EXPECT(*it == 'a');
            it--; BEAST_EXPECT(*it == 'b');
            --it; BEAST_EXPECT(*it == 'c');
            BEAST_EXPECT(it == s.crbegin());
        }
        {
            auto it = ac.rend();
            --it; BEAST_EXPECT(*it == 'a');
            it--; BEAST_EXPECT(*it == 'b');
            --it; BEAST_EXPECT(*it == 'c');
            BEAST_EXPECT(it == ac.rbegin());
        }

        {
            string s2;
            string const& cs2(s2);
            BEAST_EXPECT(std::distance(
                s2.begin(), s2.end()) == 0);
            BEAST_EXPECT(std::distance(
                cs2.begin(), cs2.end()) == 0);
            BEAST_EXPECT(std::distance(
                s2.rbegin(), s2.rend()) == 0);
            BEAST_EXPECT(std::distance(
                cs2.rbegin(), cs2.rend()) == 0);
        }
    }

    void
    testAppend()
    {
        // append(size_type, char)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(3, '*');
            BEAST_EXPECT(s == "123***");
        });

        // append(string)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(string("abc"));
            BEAST_EXPECT(s == "123abc");
        });

        // append(string, size_type, size_type)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(string("abcde"), 1, 3);
            BEAST_EXPECT(s == "123bcd");
        });

        // append(char const*, size_type)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append("abc", 3);
            BEAST_EXPECT(s == "123abc");
        });

        // append(char const*)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append("abc");
            BEAST_EXPECT(s == "123abc");
        });

        // append(InputIt, InputIt)
#if 0
        fail_loop([](storage_ptr const& sp)
        {
            std::initializer_list<
                char> init = { 'a', 'b', 'c' };
            string s("123", sp);
            s.append(init.begin(), init.end());
            BEAST_EXPECT(s == "123abc");
        });

        // append(init_list)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append({'a', 'b', 'c'});
            BEAST_EXPECT(s == "123abc");
        });
#endif

        // append(string_view)
        fail_loop([](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(string_view("abc", 3));
            BEAST_EXPECT(s == "123abc");
        });

    }

    void
    testCapacity()
    {
        // empty()
        {
            {
                string s;
                BEAST_EXPECT(s.empty());
            }
            {
                string s = "abc";
                BEAST_EXPECT(! s.empty());
            }
        }

        // size()
        // length()
        // max_size()
        {
            string s = "abc";
            BEAST_EXPECT(s.size() == 3);
            BEAST_EXPECT(s.length() == 3);
            BEAST_EXPECT(s.max_size() < string::npos);
        }

        // reserve(size_type)
        {
        }

        // capacity()
        {
        }
    }

    void
    run() override
    {
        testCtors();
        testOpAssign();
        testAssign();
        testAccess();
        testIterators();
        testCapacity();

        testAppend();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,string);

} // json
} // boost
