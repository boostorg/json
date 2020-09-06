<img width="880" height = "80" alt = "Boost.JSON Title"
    src="https://raw.githubusercontent.com/CPPAlliance/json/master/doc/images/repo-logo-2.png">

Branch          | [`master`](https://github.com/CPPAlliance/json/tree/master) | [`develop`](https://github.com/CPPAlliance/json/tree/develop) |
--------------- | ----------------------------------------------------------- | ------------------------------------------------------------- |
Travis          | [![Build Status](https://travis-ci.org/CPPAlliance/json.svg?branch=master)](https://travis-ci.org/CPPAlliance/json) | [![Build Status](https://travis-ci.org/CPPAlliance/json.svg?branch=develop)](https://travis-ci.org/CPPAlliance/json)
Appveyor        | [![Build status](https://ci.appveyor.com/api/projects/status/8csswcnmfm798203?branch=master&svg=true)](https://ci.appveyor.com/project/vinniefalco/cppalliance-json/branch/master) | [![Build status](https://ci.appveyor.com/api/projects/status/8csswcnmfm798203?branch=develop&svg=true)](https://ci.appveyor.com/project/vinniefalco/cppalliance-json/branch/develop)
Azure Pipelines | [![Build Status](https://img.shields.io/azure-devops/build/vinniefalco/2571d415-8cc8-4120-a762-c03a8eda0659/5/master)](https://vinniefalco.visualstudio.com/json/_build/latest?definitionId=5&branchName=master) | [![Build Status](https://img.shields.io/azure-devops/build/vinniefalco/2571d415-8cc8-4120-a762-c03a8eda0659/5/develop)](https://vinniefalco.visualstudio.com/json/_build/latest?definitionId=5&branchName=develop)
codecov.io      | [![codecov](https://codecov.io/gh/CPPAlliance/json/branch/master/graph/badge.svg)](https://codecov.io/gh/CPPAlliance/json/branch/master) | [![codecov](https://codecov.io/gh/CPPAlliance/json/branch/develop/graph/badge.svg)](https://codecov.io/gh/CPPAlliance/json/branch/develop)
Docs            | [![Documentation](https://img.shields.io/badge/docs-master-brightgreen.svg)](http://master.json.cpp.al/) | [![Documentation](https://img.shields.io/badge/docs-develop-brightgreen.svg)](http://develop.json.cpp.al/)
Matrix          | [![Matrix](https://img.shields.io/badge/matrix-master-brightgreen.svg)](http://www.boost.org/development/tests/master/developer/json.html) | [![Matrix](https://img.shields.io/badge/matrix-develop-brightgreen.svg)](http://www.boost.org/development/tests/develop/developer/json.html)
Fuzzing         | --- |  [![fuzz](https://github.com/CPPAlliance/json/workflows/fuzz/badge.svg?branch=develop)](https://github.com/CPPAlliance/json/actions?query=workflow%3Afuzz+branch%3Adevelop) 

# Boost.JSON

## This is currently **NOT** an official Boost library.

## Introduction

This library provides containers and algorithms which implement JSON
("JavaScript Object Notation"), a lightweight data-interchange format.
This format is easy for humans to read and write, and easy for machines
to parse and generate. It is based on a subset of the Javascript Programming
Language, Standard ECMA-262. JSON is a text format that is language-indepdent
but uses conventions that are familiar to programmers of the C-family of
languages, including C, C++, C#, Java, JavaScript, Perl, Python, and many
others. These properties make JSON an ideal data-interchange language.

## Motivation

Boost.JSON offers these features:

* Fast compilation
* Require only C++11
* Easy and safe API with allocator support
* Fast incremental parser and serializer
* Constant-time key lookup for objects
* Options to allow non-standard JSON
* Compile without Boost, define `BOOST_JSON_STANDALONE`
* Optional header-only, without linking to a library

This library focuses on a common and popular use-case for JSON:
parsing and serializing to and from a container called `value`
which holds JSON types. This container is designed to be well
suited as a vocabulary type appropriate for use in public interfaces
and libraries, allowing them to be composed. The parser and serializer
are both highly performant, meeting or exceeding the benchmark performance
of the best comparable libraries. Allocators are very well supported.
Code which uses Boost.JSON types will be easy to read, flexible, and
performant.

## Requirements

The library relies heavily on these well known C++ types in its interfaces
(henceforth termed _standard types_):

* `string_view`
* `memory_resource`, `polymorphic_allocator`
* `error_category`, `error_code`, `error_condition`, `system_error`

The requirements for Boost.JSON depend on whether the library is used
as part of Boost, or in the standalone flavor (without Boost):

### Using Boost

* Requires only C++11
* The default configuration
* Aliases for standard types use their Boost equivalents
* Link to a built static or dynamic Boost library, or use header-only (below)
* Supports -fno-exceptions, detected automatically

### Without Boost

* Requires C++17
* Aliases for standard types use their `std` equivalents
* Obtained when defining the macro `BOOST_JSON_STANDALONE`
* Link to a built static or dynamic standalone library, or use header-only (below)
* Supports -fno-exceptions, define `BOOST_NO_EXCEPTIONS` manually

### Header-Only

To use Boost.JSON header-only; that is, to eliminate the requirement
to modify build scripts to link to a static or dynamic library, simply
place the following line in any new or existing translation unit in your
project.
```
#include <boost/json/src.hpp>
```

### Note
The library uses separate inline namespacing for the standalone
flavor to allow libraries which use different flavors to compose
without causing link errors. Linking to both flavors of Boost.JSON
is possible, but not recommended.

## CMake

    cmake -G "Visual Studio 16 2019" -A Win32 -B bin -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake
    cmake -G "Visual Studio 16 2019" -A x64 -B bin64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake

## License

Distributed under the Boost Software License, Version 1.0.
(See accompanying file [LICENSE_1_0.txt](LICENSE_1_0.txt) or copy at
https://www.boost.org/LICENSE_1_0.txt)
