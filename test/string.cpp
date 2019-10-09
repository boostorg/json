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
#include <numeric>
#include <string>

#include "test_storage.hpp"

namespace boost {
namespace json {

class string_test : public beast::unit_test::suite
{
public:
    struct test_vectors
    {
        // fit in sbo
        string_view v1; // "abc...

        // dynamic alloc
        string_view v2; // "ABC...

        std::string const s1;
        std::string const s2;

        test_vectors()
            : s1([&]
            {
                std::string s;
                s.resize(string{}.capacity());
                std::iota(s.begin(), s.end(), 'a');
                return s;
            }())
            , s2([&]
            {
                std::string s;
                s.resize(string{}.capacity());
                s.resize(s.size() + 1);
                std::iota(s.begin(), s.end(), 'A');
                return s;
            }())
        {
            v1 = s1;
            v2 = s2;
        }
    };

    void
    testConstruction()
    {
        test_vectors const t;

        // string()
        {
            scoped_fail_storage fs;
            string s;
        }

        // string(storage_ptr)
        {  
            scoped_fail_storage fs;
            string s(fs.get());
            BEAST_EXPECT(s.empty());
            BEAST_EXPECT(*s.get_storage() == *fs.get());
        }

        // string(size_type, char, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                BEAST_EXPECT(s == std::string(t.v1.size(), '*'));
            });

            fail_loop([&]
            {
                string s(t.v2.size(), '*');
                BEAST_EXPECT(s == std::string(t.v2.size(), '*'));
            });
        }
        
        // string(string const&, size_type, size_type, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(string(t.v1), 1, 3, sp);
                BEAST_EXPECT(s == "bcd");
            });

            fail_loop([&]
            {
                string s(string(t.v1), 1, 3);
                BEAST_EXPECT(s == "bcd");
            });

            fail_loop([&]
            {
                string s(string(t.v1), 1);
                BEAST_EXPECT(s == t.v1.substr(1));
            });
        }

        // string(char const*, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.s1.c_str(), sp);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.s2.c_str(), sp);
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&]
            {
                string s(t.s1.c_str());
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s(t.s2.c_str());
                BEAST_EXPECT(s == t.v2);
            });
        }

        // string(char const*, size_type, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.s1.c_str(), 3, sp);
                BEAST_EXPECT(s == "abc");
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.s2.c_str(), 3, sp);
                BEAST_EXPECT(s == "ABC");
            });

            fail_loop([&]
            {
                string s(t.s1.c_str(), 3);
                BEAST_EXPECT(s == "abc");
            });

            fail_loop([&]
            {
                string s(t.s2.c_str(), 3);
                BEAST_EXPECT(s == "ABC");
            });
        }

        // string(InputIt, InputIt, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.begin(), t.v1.end(), sp);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.begin(), t.v2.end(), sp);
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(
                    make_input_iterator(t.v1.begin()),
                    make_input_iterator(t.v1.end()), sp);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(
                    make_input_iterator(t.v2.begin()),
                    make_input_iterator(t.v2.end()), sp);
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&]
            {
                string s(t.v1.begin(), t.v1.end());
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s(t.v2.begin(), t.v2.end());
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&]
            {
                string s(
                    make_input_iterator(t.v1.begin()),
                    make_input_iterator(t.v1.end()));
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s(
                    make_input_iterator(t.v2.begin()),
                    make_input_iterator(t.v2.end()));
                BEAST_EXPECT(s == t.v2);
            });
        }

        // string(string)
        {
            fail_loop([&]
            {
                string s(string(t.v1));
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s(string(t.v2));
                BEAST_EXPECT(s == t.v2);
            });
        }

        // string(string, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(string(t.v1), sp);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(string(t.v2), sp);
                BEAST_EXPECT(s == t.v2);
            });
        }

        // string(pilfered<string>)
        {
            {
                string s1(t.v1);
                string s2(pilfer(s1));
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(s1.empty());
                BEAST_EXPECT(
                    s1.get_storage() == nullptr);
            }

            {
                string s1(t.v2);
                string s2(pilfer(s1));
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(s1.empty());
                BEAST_EXPECT(
                    s1.get_storage() == nullptr);
            }
        }

        // string(string&&)
        {
            {
                string s1(t.v1);
                scoped_fail_storage fs;
                string s2(std::move(s1));
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(s1.empty());
                BEAST_EXPECT(
                    *s1.get_storage() ==
                    *s2.get_storage());
            }

            {
                string s1(t.v2);
                scoped_fail_storage fs;
                string s2(std::move(s1));
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(s1.empty());
                BEAST_EXPECT(
                    *s1.get_storage() ==
                    *s2.get_storage());
            }
        }

        // string(string&&, storage_ptr)
        {
            // same storage

            fail_loop([&](storage_ptr const& sp)
            {
                string s1(t.v1, sp);
                string s2(std::move(s1), sp);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(s1 == t.v1);
                BEAST_EXPECT(
                    *s1.get_storage() ==
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s1(t.v2, sp);
                string s2(std::move(s1));
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(s1.empty());
                BEAST_EXPECT(
                    *s1.get_storage() ==
                    *s2.get_storage());
            });

            // different storage

            fail_loop([&](storage_ptr const& sp)
            {
                string s1(t.v1);
                string s2(std::move(s1), sp);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(s1 == t.v1);
                BEAST_EXPECT(
                    *s1.get_storage() !=
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s1(t.v2);
                string s2(std::move(s1), sp);
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(s1 == t.v2);
                BEAST_EXPECT(
                    *s1.get_storage() !=
                    *s2.get_storage());
            });
        }

        // string(initializer_list, storage_ptr)
        {
            std::initializer_list<char> init1 = {
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's'
            };
            BEAST_EXPECT(std::string(init1) == t.s1);

            std::initializer_list<char> init2 = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'
            };
            BEAST_EXPECT(std::string(init2) == t.s2);

            fail_loop([&](storage_ptr const& sp)
            {
                string s(init1, sp);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(init2, sp);
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&]
            {
                string s(init1);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s(init2);
                BEAST_EXPECT(s == t.v2);
            });
        }

        // string(string_view, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1, sp);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2, sp);
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&]
            {
                string s(t.v1);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s(t.v2);
                BEAST_EXPECT(s == t.v2);
            });
        }

        // string(string_view, size_type, size_type, storage_ptr)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1, 1, 3, sp);
                BEAST_EXPECT(s == "bcd");
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2, 1, 3, sp);
                BEAST_EXPECT(s == "BCD");
            });

            fail_loop([&]
            {
                string s(t.v1, 1, 3);
                BEAST_EXPECT(s == "bcd");
            });

            fail_loop([&]
            {
                string s(t.v2, 1, 3);
                BEAST_EXPECT(s == "BCD");
            });
        }
    }

    void
    testAssignment()
    {
        test_vectors const t;

        // operator=(string)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string const s2(t.v1);
                s = s2;
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string const s2(t.v1);
                s = s2;
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string const s2(t.v2);
                s = s2;
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string const s2(t.v2);
                s = s2;
                BEAST_EXPECT(s == t.v2);
            });
        }

        // operator=(string&&)
        {
            // same storage

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v1, sp);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v1, sp);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v2, sp);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2.empty());
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v2, sp);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2.empty());
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            // different storage

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v1);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v1);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v2);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v2);
                s = std::move(s2);
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });
        }

        // operator=(char const*)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s = t.s1.c_str();
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s = t.s1.c_str();
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s = t.s2.c_str();
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s = t.s2.c_str();
                BEAST_EXPECT(s == t.v2);
            });
        }

        // operator=(char)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s = '*';
                BEAST_EXPECT(s == "*");
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s = '*';
                BEAST_EXPECT(s == "*");
            });
        }

        // operator=(std::initializer_list<char>)
        {
            std::initializer_list<char> init1 = {
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's'
            };
            BEAST_EXPECT(std::string(init1) == t.s1);

            std::initializer_list<char> init2 = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'
            };
            BEAST_EXPECT(std::string(init2) == t.s2);

            fail_loop([&](storage_ptr const& sp)
            {
                string s(sp);
                s = init1;
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(sp);
                s = init2;
                BEAST_EXPECT(s == t.v2);
            });
        }

        // operator=(string_view)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s = t.v1;
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s = t.v1;
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s = t.v2;
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s = t.v2;
                BEAST_EXPECT(s == t.v2);
            });
        }
    }

    void
    testAssign()
    {
        test_vectors const t;

        // assign(size_type, char)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), 'x', sp);
                s.assign(t.v1.size(), '*');
                BEAST_EXPECT(
                    s == std::string(t.v1.size(), '*'));
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), 'x', sp);
                s.assign(t.v1.size(), '*');
                BEAST_EXPECT(
                    s == std::string(t.v1.size(), '*'));
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), 'x', sp);
                s.assign(t.v2.size(), '*');
                BEAST_EXPECT(
                    s == std::string(t.v2.size(), '*'));
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), 'x', sp);
                s.assign(t.v2.size(), '*');
                BEAST_EXPECT(
                    s == std::string(t.v2.size(), '*'));
            });
        }

        // assign(string)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), 'x', sp);
                s.assign(string(t.v1.size(), '*'));
                BEAST_EXPECT(
                    s == std::string(t.v1.size(), '*'));
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), 'x', sp);
                s.assign(string(t.v1.size(), '*'));
                BEAST_EXPECT(
                    s == std::string(t.v1.size(), '*'));
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), 'x', sp);
                s.assign(string(t.v2.size(), '*'));
                BEAST_EXPECT(
                    s == std::string(t.v2.size(), '*'));
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), 'x', sp);
                s.assign(string(t.v2.size(), '*'));
                BEAST_EXPECT(
                    s == std::string(t.v2.size(), '*'));
            });
        }

        // assign(string, size_type, size_type)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), 'x', sp);
                s.assign(string(t.v1.size(), '*'), 1, 3);
                BEAST_EXPECT(
                    s == std::string(3, '*'));
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), 'x', sp);
                s.assign(string(t.v1.size(), '*'), 1, 3);
                BEAST_EXPECT(
                    s == std::string(3, '*'));
            });
        }

        // assign(string&&)
        {
            // same storage

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v1, sp);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v1, sp);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v2, sp);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2.empty());
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v2, sp);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2.empty());
                BEAST_EXPECT(
                    *s.get_storage() ==
                    *s2.get_storage());
            });

            // different storage

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v1);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v1);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v1);
                BEAST_EXPECT(s2 == t.v1);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v1.size(), '*');
                string s(c, sp);
                string s2(t.v2);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });

            fail_loop([&](storage_ptr const& sp)
            {
                std::string c(t.v2.size(), '*');
                string s(c, sp);
                string s2(t.v2);
                s.assign(std::move(s2));
                BEAST_EXPECT(s == t.v2);
                BEAST_EXPECT(s2 == t.v2);
                BEAST_EXPECT(
                    *s.get_storage() !=
                    *s2.get_storage());
            });
        }

        // assign(char const*, size_type)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(t.s1.c_str(), 3);
                BEAST_EXPECT(s == "abc");
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(t.s1.c_str(), 3);
                BEAST_EXPECT(s == "abc");
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(t.s2.c_str(), 3);
                BEAST_EXPECT(s == "ABC");
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(t.s2.c_str(), 3);
                BEAST_EXPECT(s == "ABC");
            });
        };

        // assign(char const* s)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(t.s1.c_str());
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(t.s1.c_str());
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(t.s2.c_str());
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(t.s2.c_str());
                BEAST_EXPECT(s == t.v2);
            });
        }

        // assign(InputIt, InputIt)
        {
            std::initializer_list<char> init1 = {
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's'
            };
            BEAST_EXPECT(std::string(init1) == t.s1);

            std::initializer_list<char> init2 = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'
            };
            BEAST_EXPECT(std::string(init2) == t.s2);

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(init1.begin(), init1.end());
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(init1.begin(), init1.end());
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(
                    make_input_iterator(init1.begin()),
                    make_input_iterator(init1.end()));
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(
                    make_input_iterator(init1.begin()),
                    make_input_iterator(init1.end()));
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(init2.begin(), init2.end());
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(init2.begin(), init2.end());
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(
                    make_input_iterator(init2.begin()),
                    make_input_iterator(init2.end()));
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(
                    make_input_iterator(init2.begin()),
                    make_input_iterator(init2.end()));
                BEAST_EXPECT(s == t.v2);
            });
        }

        // assign(std::initializer_list<char>)
        {
            std::initializer_list<char> init1 = {
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's'
            };
            BEAST_EXPECT(std::string(init1) == t.s1);

            std::initializer_list<char> init2 = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'
            };
            BEAST_EXPECT(std::string(init2) == t.s2);

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(init1);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(init1);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(init2);
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(init2);
                BEAST_EXPECT(s == t.v2);
            });
        }

        // assign(string_view)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(t.v1);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(t.v1);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(t.v2);
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(t.v2);
                BEAST_EXPECT(s == t.v2);
            });
        }
        
        // assign(string_view, size_type, size_type);
        {
            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v1.size(), '*', sp);
                s.assign(t.v1, 1, 3);
                BEAST_EXPECT(s == "bcd");
            });

            fail_loop([&](storage_ptr const& sp)
            {
                string s(t.v2.size(), '*', sp);
                s.assign(t.v2, 1, 3);
                BEAST_EXPECT(s == "BCD");
            });
        }
    }

    void
    testElementAccess()
    {
        test_vectors const t;

        string s1(t.v1);
        string s2(t.v2);
        auto const& cs1(s1);
        auto const& cs2(s2);

        // at(size_type)
        {
            BEAST_EXPECT(s1.at(1) == 'b');
            s1.at(1) = '*';
            BEAST_EXPECT(s1.at(1) == '*');
            s1.at(1) = 'b';
            BEAST_EXPECT(s1.at(1) == 'b');

            BEAST_EXPECT(s2.at(1) == 'B');
            s2.at(1) = '*';
            BEAST_EXPECT(s2.at(1) == '*');
            s2.at(1) = 'B';
            BEAST_EXPECT(s2.at(1) == 'B');

            BEAST_THROWS(s1.at(s2.size()),
                std::out_of_range);
        }

        // at(size_type) const
        {
            BEAST_EXPECT(cs1.at(1) == 'b');
            BEAST_EXPECT(cs2.at(1) == 'B');

            BEAST_THROWS(cs1.at(cs2.size()),
                std::out_of_range);
        }

        // operator[&](size_type)
        {
            BEAST_EXPECT(s1[1] == 'b');
            s1[1] = '*';
            BEAST_EXPECT(s1[1] == '*');
            s1[1] = 'b';
            BEAST_EXPECT(s1[1] == 'b');

            BEAST_EXPECT(s2[1] == 'B');
            s2[1] = '*';
            BEAST_EXPECT(s2[1] == '*');
            s2[1] = 'B';
            BEAST_EXPECT(s2[1] == 'B');
        }

        // operator[&](size_type) const
        {
            BEAST_EXPECT(cs1[1] == 'b');
            BEAST_EXPECT(cs2[1] == 'B');
        }

        // front()
        {
            BEAST_EXPECT(s1.front() == 'a');
            s1.front() = '*';
            BEAST_EXPECT(s1.front() == '*');
            s1.front() = 'a';
            BEAST_EXPECT(s1.front() == 'a');

            BEAST_EXPECT(s2.front() == 'A');
            s2.front() = '*';
            BEAST_EXPECT(s2.front() == '*');
            s2.front() = 'A';
            BEAST_EXPECT(s2.front() == 'A');
        }

        // front() const
        {
            BEAST_EXPECT(cs1.front() == 'a');
            BEAST_EXPECT(cs2.front() == 'A');
        }

        // back()
        {
            auto const ch1 = s1.at(s1.size()-1);
            auto const ch2 = s2.at(s2.size()-1);

            BEAST_EXPECT(s1.back() == ch1);
            s1.back() = '*';
            BEAST_EXPECT(s1.back() == '*');
            s1.back() = ch1;
            BEAST_EXPECT(s1.back() == ch1);

            BEAST_EXPECT(s2.back() == ch2);
            s2.back() = '*';
            BEAST_EXPECT(s2.back() == '*');
            s2.back() = ch2;
            BEAST_EXPECT(s2.back() == ch2);
        }

        // back() const
        {
            auto const ch1 = s1.at(s1.size()-1);
            auto const ch2 = s2.at(s2.size()-1);

            BEAST_EXPECT(cs1.back() == ch1);
            BEAST_EXPECT(cs2.back() == ch2);
        }

        // data()
        {
            BEAST_EXPECT(
                string_view(s1.data()) == t.v1);
            BEAST_EXPECT(
                string_view(s2.data()) == t.v2);
        }
        // data() const
        {
            BEAST_EXPECT(
                string_view(cs1.data()) == t.v1);
            BEAST_EXPECT(
                string_view(cs2.data()) == t.v2);
        }

        // c_str()
        {
            BEAST_EXPECT(
                string_view(cs1.c_str()) == t.v1);
            BEAST_EXPECT(
                string_view(cs2.c_str()) == t.v2);
        }

        // operator string_view()
        {
            BEAST_EXPECT(
                string_view(cs1) == t.v1);
            BEAST_EXPECT(
                string_view(cs2) == t.v2);
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
    testClear()
    {
        test_vectors const t;

        // clear()
        {
            {
                string s(t.v1);
                s.clear();
                BEAST_EXPECT(s.empty());
                BEAST_EXPECT(s.size() == 0);
                BEAST_EXPECT(s.capacity() > 0);
            }

            {
                string s(t.v2);
                s.clear();
                BEAST_EXPECT(s.empty());
                BEAST_EXPECT(s.size() == 0);
                BEAST_EXPECT(s.capacity() > 0);
            }
        }
    }

    void
    testInsert()
    {
        // insert(size_type, size_type, char)
        // insert(size_type, string_view)
        // insert(size_type, char const*, size_type)
        // insert(size_type, string_view, size_type, size_type)
        // insert(const_iterator, size_type, char ch)
        // insert(const_iterator, InputIt, InputIt)
    }

    void
    testErase()
    {
        test_vectors const t;

        // erase(size_type, size_type)
        {
            {
                string s(t.v1);
                s.erase(1, 3);
                BEAST_EXPECT(s ==
                    std::string(t.v1).erase(1, 3));
            }

            {
                string s(t.v2);
                s.erase(1, 3);
                BEAST_EXPECT(s ==
                    std::string(t.v2).erase(1, 3));
            }

            {
                string s(t.v1);
                s.erase(3);
                BEAST_EXPECT(s ==
                    std::string(t.v1).erase(3));
            }

            {
                string s(t.v2);
                s.erase(3);
                BEAST_EXPECT(s ==
                    std::string(t.v2).erase(3));
            }

            {
                string s(t.v1);
                s.erase();
                BEAST_EXPECT(s ==
                    std::string(t.v1).erase());
            }

            {
                string s(t.v2);
                s.erase();
                BEAST_EXPECT(s ==
                    std::string(t.v2).erase());
            }
        }

        // iterator erase(const_iterator)
        {
            {
                string s(t.v1);
                std::string s2(t.v1);
                s.erase(s.begin() + 3);
                s2.erase(s2.begin() + 3);
                BEAST_EXPECT(s == s2);
            }

            {
                string s(t.v2);
                std::string s2(t.v2);
                s.erase(s.begin() + 3);
                s2.erase(s2.begin() + 3);
                BEAST_EXPECT(s == s2);
            }
        }

        // iterator erase(const_iterator, const_iterator)
        {
            string s(t.v1);
            std::string s2(t.v1);
            s.erase(s.begin() + 1, s.begin() + 3);
            s2.erase(s2.begin() + 1, s2.begin() + 3);
            BEAST_EXPECT(s == s2);
        }
    }

    void
    testPushPop()
    {
        test_vectors const t;

        // push_back(char)
        {
            fail_loop([&]
            {
                string s;
                for(auto ch : t.v1)
                    s.push_back(ch);
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s;
                for(auto ch : t.v2)
                    s.push_back(ch);
                BEAST_EXPECT(s == t.v2);
            });
        }

        // pop_back(char)
        {
            {
                string s(t.v1);
                while(! s.empty())
                    s.pop_back();
                BEAST_EXPECT(s.empty());
                BEAST_EXPECT(s.capacity() > 0);
            }

            {
                string s(t.v2);
                while(! s.empty())
                    s.pop_back();
                BEAST_EXPECT(s.empty());
                BEAST_EXPECT(s.capacity() > 0);
            }
        }
    }

    void
    testAppend()
    {
        // append(size_type, char)
        fail_loop([&](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(3, '*');
            BEAST_EXPECT(s == "123***");
        });

        // append(string)
        fail_loop([&](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(string("abc"));
            BEAST_EXPECT(s == "123abc");
        });

        // append(string, size_type, size_type)
        fail_loop([&](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(string("abcde"), 1, 3);
            BEAST_EXPECT(s == "123bcd");
        });

        // append(char const*, size_type)
        fail_loop([&](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append("abc", 3);
            BEAST_EXPECT(s == "123abc");
        });

        // append(char const*)
        fail_loop([&](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append("abc");
            BEAST_EXPECT(s == "123abc");
        });

        // append(InputIt, InputIt)
        {
            fail_loop([&](storage_ptr const& sp)
            {
                std::initializer_list<
                    char> init = { 'a', 'b', 'c' };
                string s("123", sp);
                s.append(init.begin(), init.end());
                BEAST_EXPECT(s == "123abc");
            });

            // multiple growth
            fail_loop([&](storage_ptr const& sp)
            {
                auto init = {
                    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };
                string s("12345678", sp);
                s.append(init.begin(), init.end());
                BEAST_EXPECT(
                    s == "12345678abcdefghijklmnop");
            });

            // input iterator
            fail_loop([&](storage_ptr const& sp)
            {
                auto init = {
                    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };
                string s("12345678", sp);
                s.append(
                    make_input_iterator(init.begin()),
                    make_input_iterator(init.end()));
                BEAST_EXPECT(
                    s == "12345678abcdefghijklmnop");
            });
        }

        // append(init_list)
        fail_loop([&](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append({'a', 'b', 'c'});
            BEAST_EXPECT(s == "123abc");
        });

        // append(string_view)
        fail_loop([&](storage_ptr const& sp)
        {
            string s("123", sp);
            s.append(string_view("abc", 3));
            BEAST_EXPECT(s == "123abc");
        });
    }

    void
    testPlusEquals()
    {
        test_vectors const t;

        // operator+=(string)
        {
            fail_loop([&]
            {
                string s(t.v1);
                s += string(t.v2);
                BEAST_EXPECT(s == t.s1 + t.s2);

            });

            fail_loop([&]
            {
                string s(t.v2);
                s += string(t.v1);
                BEAST_EXPECT(s == t.s2 + t.s1);
            });
        }

        // operator+=(char)
        {
            fail_loop([&]
            {
                string s("123");
                s += '4';
                BEAST_EXPECT(s == "1234");
            });

            fail_loop([&]
            {
                string s(t.v1);
                s += '*';
                BEAST_EXPECT(s == t.s1 + '*');
            });

            fail_loop([&]
            {
                string s(t.v2);
                s += '*';
                BEAST_EXPECT(s == t.s2 + '*');
            });
        }

        // operator+=(char const*)
        {
            {
                string s(t.v1);
                scoped_fail_storage fs;
                s += "";
                BEAST_EXPECT(s == t.v1);

            }

            fail_loop([&]
            {
                string s(t.v1);
                s += t.s2.c_str();
                BEAST_EXPECT(s == t.s1 + t.s2);

            });

            fail_loop([&]
            {
                string s(t.v2);
                s += t.s1.c_str();
                BEAST_EXPECT(s == t.s2 + t.s1);
            });
        }

        // operator+=(initializer_list)
        {
            std::initializer_list<char> init1 = {
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's'
            };
            BEAST_EXPECT(std::string(init1) == t.s1);

            std::initializer_list<char> init2 = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T'
            };
            BEAST_EXPECT(std::string(init2) == t.s2);

            fail_loop([&]
            {
                string s;
                s += init1;
                BEAST_EXPECT(s == t.v1);
            });

            fail_loop([&]
            {
                string s;
                s += init2;
                BEAST_EXPECT(s == t.v2);
            });

            fail_loop([&]
            {
                string s(t.v1);
                s += init2;
                BEAST_EXPECT(s == t.s1 + t.s2);
            });

            fail_loop([&]
            {
                string s(t.v2);
                s += init1;
                BEAST_EXPECT(s == t.s2 + t.s1);
            });

        }

        // operator+=(string_view)
        {
            fail_loop([&]
            {
                string s(t.v1);
                s += t.v2;
                BEAST_EXPECT(s == t.s1 + t.s2);

            });

            fail_loop([&]
            {
                string s(t.v2);
                s += t.v1;
                BEAST_EXPECT(s == t.s2 + t.s1);
            });
        }
    }

    void
    testCompare()
    {
        test_vectors const t;
        string const v1 = t.v1;

        // compare(string)
        BEAST_EXPECT(v1.compare(string("aaaaaaa")) > 0);
        BEAST_EXPECT(v1.compare(string(t.v1)) == 0);
        BEAST_EXPECT(v1.compare(string("bbbbbbb")) < 0);

        // compare(size_type, size_type, string)
        BEAST_EXPECT(v1.compare(2, 3, string("ccc")) > 0);
        BEAST_EXPECT(v1.compare(2, 3, string("cde")) == 0);
        BEAST_EXPECT(v1.compare(2, 3, string("eee")) < 0);

        // compare(size_type, size_type, string, size_type, size_type)
        BEAST_EXPECT(v1.compare(2, 3, string("bbbbb"), 1, 3) > 0);
        BEAST_EXPECT(v1.compare(2, 3, string("bcdef"), 1, 3) == 0);
        BEAST_EXPECT(v1.compare(2, 3, string("fffff"), 1, 3) < 0);

        // compare(char const*)
        BEAST_EXPECT(v1.compare("aaaaaaa") > 0);
        BEAST_EXPECT(v1.compare(t.s1.c_str()) == 0);
        BEAST_EXPECT(v1.compare("bbbbbbb") < 0);

        // compare(size_type, size_type, char const*)
        BEAST_EXPECT(v1.compare(2, 3, "ccc") > 0);
        BEAST_EXPECT(v1.compare(2, 3, "cde") == 0);
        BEAST_EXPECT(v1.compare(2, 3, "eee") < 0);

        // compare(size_type, size_type, char const*, size_type)
        BEAST_EXPECT(v1.compare(2, 4, "cccc", 4) > 0);
        BEAST_EXPECT(v1.compare(2, 4, "cdef", 4) == 0);
        BEAST_EXPECT(v1.compare(2, 4, "ffff", 4) < 0);

        // compare(string_view s)
        BEAST_EXPECT(v1.compare(string_view("aaaaaaa")) > 0);
        BEAST_EXPECT(v1.compare(t.v1) == 0);
        BEAST_EXPECT(v1.compare(string_view("bbbbbbb")) < 0);

        // compare(size_type, size_type, string_view)
        BEAST_EXPECT(v1.compare(2, 3, string_view("ccc")) > 0);
        BEAST_EXPECT(v1.compare(2, 3, string_view("cde")) == 0);
        BEAST_EXPECT(v1.compare(2, 3, string_view("eee")) < 0);

        // compare(size_type, size_type, string_view, size_type, size_type)
        BEAST_EXPECT(v1.compare(2, 3, string_view("bbbbb"), 1, 3) > 0);
        BEAST_EXPECT(v1.compare(2, 3, string_view("bcdef"), 1, 3) == 0);
        BEAST_EXPECT(v1.compare(2, 3, string_view("fffff"), 1, 3) < 0);
    }

    void
    testStartEndsWith()
    {
        test_vectors const t;
        string const v1 = t.v1;
        string const v2 = t.v2;

        // starts_with(string_view)
        {
            BEAST_EXPECT(v1.starts_with(string_view("abc")));
            BEAST_EXPECT(v2.starts_with(string_view("ABC")));
            BEAST_EXPECT(! v1.starts_with(string_view("xyz")));
            BEAST_EXPECT(! v2.starts_with(string_view("XYZ")));
        }

        // starts_with(char)
        {
            BEAST_EXPECT(v1.starts_with('a'));
            BEAST_EXPECT(v2.starts_with('A'));
            BEAST_EXPECT(! v1.starts_with('x'));
            BEAST_EXPECT(! v2.starts_with('X'));
        }

        // starts_with(char const*)
        {
            BEAST_EXPECT(v1.starts_with("abc"));
            BEAST_EXPECT(v2.starts_with("ABC"));
            BEAST_EXPECT(! v1.starts_with("xyz"));
            BEAST_EXPECT(! v2.starts_with("XYZ"));
        }

        // ends_with(string_view)
        {
            BEAST_EXPECT(v1.ends_with(string_view("qrs")));
            BEAST_EXPECT(v2.ends_with(string_view("RST")));
            BEAST_EXPECT(! v1.ends_with(string_view("abc")));
            BEAST_EXPECT(! v2.ends_with(string_view("ABC")));
        }

        // ends_with(char)
        {
            BEAST_EXPECT(v1.ends_with('s'));
            BEAST_EXPECT(v2.ends_with('T'));
            BEAST_EXPECT(! v1.ends_with('a'));
            BEAST_EXPECT(! v2.ends_with('A'));
        }

        // ends_with(char const*)
        {
            BEAST_EXPECT(v1.ends_with("qrs"));
            BEAST_EXPECT(v2.ends_with("RST"));
            BEAST_EXPECT(! v1.ends_with("abc"));
            BEAST_EXPECT(! v2.ends_with("ABC"));
        }
    }

    void
    testReplace()
    {
    }

    void
    testSubStr()
    {
        test_vectors const t;
        string const s1 = t.v1;
        string const s2 = t.v2;

        // substr(size_type, size_type)
        BEAST_EXPECT(s1.substr() == t.v1);
        BEAST_EXPECT(s1.substr(1) == t.v1.substr(1));
        BEAST_EXPECT(s1.substr(1, 3) == t.v1.substr(1, 3));
        BEAST_EXPECT(s2.substr() == t.v2);
        BEAST_EXPECT(s2.substr(1) == t.v2.substr(1));
        BEAST_EXPECT(s2.substr(1, 3) == t.v2.substr(1, 3));
    }

    void
    testCopy()
    {
    }

    void
    testResize()
    {
    }

    void
    testSwap()
    {
    }

    void
    testFind()
    {
        test_vectors const t;
        string const s1 = t.v1;
        string const s2 = t.v2;

        // find(string, size_type)
        BEAST_EXPECT(s1.find(string("bcd")) == 1);
        BEAST_EXPECT(s1.find(string("cde"), 1) == 2);
        BEAST_EXPECT(s1.find(string("efg"), 5) == string::npos);

        // find(char const*, size_type, size_type)
        BEAST_EXPECT(s1.find("bcd*", 0, 3) == 1);
        BEAST_EXPECT(s1.find("cde*", 1, 3) == 2);
        BEAST_EXPECT(s1.find("efg*", 5, 3) == string::npos);

        // find(char const*, size_type)
        BEAST_EXPECT(s1.find("bcd", 0) == 1);
        BEAST_EXPECT(s1.find("cde", 1) == 2);
        BEAST_EXPECT(s1.find("efg", 5) == string::npos);

        // find(char, size_type)
        BEAST_EXPECT(s1.find('b') == 1);
        BEAST_EXPECT(s1.find('c', 1) == 2);
        BEAST_EXPECT(s1.find('e', 5) == string::npos);

        // find(T const, size_type pos)
        BEAST_EXPECT(s1.find(string_view("bcd")) == 1);
        BEAST_EXPECT(s1.find(string_view("cde"), 1) == 2);
        BEAST_EXPECT(s1.find(string_view("efg"), 5) == string::npos);
    }

    void
    testRfind()
    {
    }

    void
    testFindFirstOf()
    {
    }

    void
    testFindNotFirstOf()
    {
    }

    void
    testFindLastOf()
    {
    }

    void
    testFindNotLastOf()
    {
    }

    void
    testNonMembers()
    {
    }

    void
    run() override
    {
        testConstruction();
        testAssignment();
        testAssign();
        testElementAccess();
        testIterators();
        testCapacity(); //

        testClear(); //
        testInsert(); //
        testErase();
        testPushPop();
        testAppend(); //
        testPlusEquals();
        testCompare();
        testStartEndsWith();
        testReplace(); //
        testSubStr();
        testCopy(); //
        testResize(); //
        testSwap(); //

        testFind();
        testRfind(); //
        testFindFirstOf(); //
        testFindNotFirstOf(); //
        testFindLastOf(); //
        testFindNotLastOf(); //

        testNonMembers();
    }
};

BEAST_DEFINE_TESTSUITE(boost,json,string);

} // json
} // boost
