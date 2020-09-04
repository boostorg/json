<img width="880" height = "80" alt = "Boost.JSON Title"
    src="https://raw.githubusercontent.com/CPPAlliance/json/master/doc/images/repo-logo-2.png">

Branch          | Travis | Appveyor | Azure Pipelines | codecov.io | Docs | Matrix |
:-------------: | ------ | -------- | --------------- | ---------- | ---- | ------ |
[`master`](https://github.com/CPPAlliance/json/tree/master) | [![Build Status](https://travis-ci.org/CPPAlliance/json.svg?branch=master)](https://travis-ci.org/CPPAlliance/json) | [![Build status](https://ci.appveyor.com/api/projects/status/8csswcnmfm798203?branch=master&svg=true)](https://ci.appveyor.com/project/vinniefalco/cppalliance-json/branch/master) | [![Build Status](https://img.shields.io/azure-devops/build/vinniefalco/2571d415-8cc8-4120-a762-c03a8eda0659/5/master)](https://vinniefalco.visualstudio.com/json/_build/latest?definitionId=5&branchName=master) | [![codecov](https://codecov.io/gh/CPPAlliance/json/branch/master/graph/badge.svg)](https://codecov.io/gh/CPPAlliance/json/branch/master) | [![Documentation](https://img.shields.io/badge/docs-master-brightgreen.svg)](http://master.json.cpp.al/) | [![Matrix](https://img.shields.io/badge/matrix-master-brightgreen.svg)](http://www.boost.org/development/tests/master/developer/json.html)
[`develop`](https://github.com/CPPAlliance/json/tree/develop) | [![Build Status](https://travis-ci.org/CPPAlliance/json.svg?branch=develop)](https://travis-ci.org/CPPAlliance/json) | [![Build status](https://ci.appveyor.com/api/projects/status/8csswcnmfm798203?branch=develop&svg=true)](https://ci.appveyor.com/project/vinniefalco/cppalliance-json/branch/develop) | [![Build Status](https://img.shields.io/azure-devops/build/vinniefalco/2571d415-8cc8-4120-a762-c03a8eda0659/5/develop)](https://vinniefalco.visualstudio.com/json/_build/latest?definitionId=5&branchName=develop) | [![codecov](https://codecov.io/gh/CPPAlliance/json/branch/develop/graph/badge.svg)](https://codecov.io/gh/CPPAlliance/json/branch/develop) | [![Documentation](https://img.shields.io/badge/docs-develop-brightgreen.svg)](http://develop.json.cpp.al/) | [![Matrix](https://img.shields.io/badge/matrix-develop-brightgreen.svg)](http://www.boost.org/development/tests/develop/developer/json.html)

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

A survey of existing JSON libraries shows impressive diversity and features.
However, no library is known to meet all of the design goals mentioned here.
In particular, we know of no library that supports incremental parsing and
serialization, and also supports custom allocators robustly.

## Design Goals

There are an overwhelming number of JSON libraries. A common theme 
throughout is an emphasis on parsing performance or feature richness.
This library uses a different approach: it provides a carefully
designed JSON container, `value`, engineered from the ground up to be
ideally suited as a vocabulary type. In particular it provides an interface
which is stable, lightweight, and appropriate for use as parameter or
return types in public interfaces.

The design of the library also achieves these goals:

* Requires only C++11.
* Support stateful allocators.
* Top performance of general libraries.
* Uniform interface on all C++ versions.
* Key lookup in objects has constant average complexity.
* Strict parser and serializer which work incrementally.
* Security-aware treatment of untrusted inputs.
* Fast compilation performance.

## CMake

    cmake -G "Visual Studio 16 2019" -A Win32 -B bin -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake
    cmake -G "Visual Studio 16 2019" -A x64 -B bin64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/msvc.cmake

## License

Distributed under the Boost Software License, Version 1.0.
(See accompanying file [LICENSE_1_0.txt](LICENSE_1_0.txt) or copy at
https://www.boost.org/LICENSE_1_0.txt)

