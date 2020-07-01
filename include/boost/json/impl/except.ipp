//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/json
//

#ifndef BOOST_JSON_IMPL_EXCEPT_IPP
#define BOOST_JSON_IMPL_EXCEPT_IPP

#include <boost/json/except.hpp>

namespace boost {
namespace json {

type_error::
type_error(char const* what)
    : std::invalid_argument(what)
{
}

number_required_error::
number_required_error(
    char const* what)
    : type_error(what)
{
}

//----------------------------------------------------------

array_index_error::
array_index_error()
    : std::out_of_range(
        "array index error")
{
}

void
array_index_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        array_index_error());
}

//---

array_required_error::
array_required_error()
    : type_error(
        "array required")
{
}

void
array_required_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        array_required_error());
}

//---

array_too_large::
array_too_large()
    : std::length_error(
        "array too large")
{
}

void
array_too_large::
raise()
{
    BOOST_THROW_EXCEPTION(
        array_too_large());
}

//---

bool_required_error::
bool_required_error()
    : type_error(
        "bool required")
{
}

void
bool_required_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        bool_required_error());
}

//---

char_pos_error::
char_pos_error()
    : std::out_of_range(
        "char index error")
{
}

void
char_pos_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        char_pos_error());
}

//---

double_required_error::
double_required_error()
    : number_required_error(
        "double required")
{
}

void
double_required_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        double_required_error());
}

//---

int64_required_error::
int64_required_error()
    : number_required_error(
        "int64 required")
{
}

void
int64_required_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        int64_required_error());
}

//---

key_not_found::
key_not_found()
    : std::invalid_argument(
        "key not found")
{
}

void
key_not_found::
raise()
{
    BOOST_THROW_EXCEPTION(
        key_not_found());
}

//---

key_too_large::
key_too_large()
    : std::length_error(
        "key too large")
{
}

void
key_too_large::
raise()
{
    BOOST_THROW_EXCEPTION(
        key_too_large());
}

//---

object_required_error::
object_required_error()
    : type_error(
        "object required")
{
}

void
object_required_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        object_required_error());
}

//---

object_too_large::
object_too_large()
    : std::length_error(
        "object too large")
{
}

void
object_too_large::
raise()
{
    BOOST_THROW_EXCEPTION(
        object_too_large());
}

//---

stack_overflow::
stack_overflow()
    : std::runtime_error(
        "stack overflow")
{
}

void
stack_overflow::
raise()
{
    BOOST_THROW_EXCEPTION(
        stack_overflow());
}

//---

string_required_error::
string_required_error()
    : type_error(
        "string required")
{
}

void
string_required_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        string_required_error());
}

//---

string_too_large::
string_too_large()
    : std::length_error(
        "string too large")
{
}

void
string_too_large::
raise()
{
    BOOST_THROW_EXCEPTION(
        string_too_large());
}

//---

uint64_required_error::
uint64_required_error()
    : number_required_error(
        "uint64 required")
{
}

void
uint64_required_error::
raise()
{
    BOOST_THROW_EXCEPTION(
        uint64_required_error());
}

} // json
} // boost

#endif
